/*
1.Demo需求：
命令行解析：
a.实现：FLOWS、SHARDS、Carra的方法
b.实现：MAE和MAEQ的指标，点数为100
c.实现：根据采样率和采样数的两种方法
d.实现：可以控制采样率和采样数
e.实现：输出真实MRC(MAEQ和MAE两个版本，点数为1000点)
f.实现：根据输入的trace进行测试
g.实现：输出结果到文件，统计trace的workingset
h.实现：输出程序的执行时间

编译：g++ flows -o flows -O3
帮助：./flows

data sample:
0 0 1594 0
0 1 3421 0
0 2 16079 1
0 3 17012 0
0 4 11239 1
0 5 5506 0
0 6 36809 1
0 7 3876 0
0 8 52774 1
0 9 13413 1
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <random>
#include <fcntl.h>
#include <unistd.h>
#include <queue>
#include <set>
#include <cassert>
#include <numeric>
#include "cmdline.h"
#include "splaytree.h"
#include "LRU_fix_capacity.h"
#include "LRU_fix_count.h"
using namespace std;

#define TEST_POINT_NUM 1000
uint64_t g_hash_mask = 0;
ofstream out_file;

struct OracleReq
{
    uint32_t timestamp;
    uint64_t obj_id;
    uint32_t obj_size;
    int64_t next_access_vtime; // -1 if no next access
};                             // 48byte

struct key_feature_t
{
    int64_t last_acc_time;
    int32_t frequency;
    uint32_t size;
};

struct key_feature_short_t
{
    int32_t frequency;
    uint32_t size;
};

bool compare(const key_feature_short_t &a, key_feature_short_t &b)
{
    return a.frequency > b.frequency;
}

class Trace
{
public:
    Trace(string filename,
          string tracetype,
          int64_t _total_access_num = 0,
          int64_t _total_access_size = 0,
          int64_t _unique_access_num = 0,
          int64_t _unique_access_size = 0,
          int64_t _min_item_size = INT64_MAX) : filename(filename), now_idx(0),
                                                total_access_num(_total_access_num),
                                                total_access_size(_total_access_size),
                                                unique_access_num(_unique_access_num),
                                                unique_access_size(_unique_access_size),
                                                min_item_size(_min_item_size),
                                                MAEQ_factor(0.0),
                                                use_bin_file(false),
                                                bin_trace_fp(NULL)
    {
        if (tracetype == "PLAIN")
        {
            FILE *fp = fopen(filename.c_str(), "r");
            if (fp == NULL)
            {
                cerr << "open file error" << endl;
                exit(1);
            }
            printf("open file %s\n", filename.c_str());

            char buf[1024];
            uint64_t time_stamp, key, size, oth;
            ;
            while (fscanf(fp, "%ld %ld %ld %ld", &time_stamp, &key, &size, &oth) != EOF)
            {
                key_size_vec.push_back(make_pair(key, size));
                if (key_size_vec.size() % 10000000 == 0)
                {
                    printf("read %ld lines\n", key_size_vec.size());
                }
            }
            fclose(fp);
        }
        else if (tracetype == "CSV")
        {
            skey_ikey = new unordered_map<string, int64_t>();
            ifstream infile(filename);
            if (!infile.is_open())
            {
                cerr << "can't open file" << endl;
                exit(1);
            }
            printf("open file %s\n", filename.c_str());
            string line;
            uint64_t now_key_cnt = 0;
            while (std::getline(infile, line))
            {
                stringstream linestream(line);
                vector<string> row;
                string item;
                while (getline(linestream, item, ','))
                {
                    row.push_back(item);
                }
                if (row.size() != 4)
                {
                    cerr << "row size error" << endl;
                    exit(1);
                }
                int64_t time_stamp, key, size, oth;
                time_stamp = stoull(row[0]);
                if (skey_ikey->count(row[1]) == 0)
                {
                    skey_ikey->insert(make_pair(row[1], now_key_cnt++));
                    key = skey_ikey->at(row[1]);
                }
                else
                {
                    key = skey_ikey->at(row[1]);
                }
                size = stoull(row[2]);
                oth = stoull(row[3]);
                key_size_vec.push_back(make_pair(key, size));
                if (key_size_vec.size() % 10000000 == 0)
                {
                    printf("read %ld lines\n", key_size_vec.size());
                }
            }
            infile.close();
        }
        else if (tracetype == "bin")
        {
            use_bin_file = true;
            bin_trace_fp = fopen(filename.c_str(), "rb");
            if (bin_trace_fp == NULL)
            {
                printf("Failed to open file: %s\n", filename.c_str());
                return;
            }
            printf("ok open file\n");
        }
        else if (tracetype == "wiki")
        {
            ;
        }
        if (total_access_num != 0)
        {
            cout << "[FILE]: " << filename << endl;
            cout << "total_access_num: " << total_access_num << endl;
            cout << "total_access_size: " << total_access_size << endl;
            cout << "unique_access_num: " << unique_access_num << endl;
            cout << "unique_access_size: " << unique_access_size << endl;
            cout << "min_item_size: " << min_item_size << endl;
        }
        else
        {
            get_working_set_size();
        }
    }

    ~Trace()
    {
        if (bin_trace_fp)
        {
            fclose(bin_trace_fp);
        }
    }

    void reset()
    {
        if (use_bin_file)
        {
            fseek(bin_trace_fp, 0, SEEK_SET);
        }

        now_idx = 0;
    }

    pair<int64_t, int64_t> get_next()
    {
        if (use_bin_file)
        {
            struct OracleReq req;
            now_idx++;
            if (now_idx % 10000000 == 0)
            {
                printf("read %ld lines\n", now_idx);
            }
            if (fread(&req, sizeof(OracleReq), 1, bin_trace_fp) == 1)
            {
                if (req.obj_size < 1)
                {
                    req.obj_size = 1;
                }
                return make_pair(req.obj_id, req.obj_size);
            }
            else
            {
                return make_pair(-1, -1);
            }
        }
        else
        {
            if (now_idx >= key_size_vec.size())
            {
                return make_pair(-1, -1);
            }
            return key_size_vec[now_idx++];
        }
    }

    string filename;
    int64_t now_idx;
    int64_t total_access_num;
    int64_t total_access_size;
    int64_t unique_access_num;
    int64_t unique_access_size;
    int64_t min_item_size;
    double MAEQ_factor;
    bool use_bin_file;
    FILE *bin_trace_fp;
    vector<pair<int64_t, int64_t>> key_size_vec;
    unordered_map<string, int64_t> *skey_ikey;

    void get_working_set_size()
    {
        unordered_map<int64_t, int64_t> key_seen_time;
        total_access_num = 0;
        total_access_size = 0;
        unique_access_num = 0;
        unique_access_size = 0;
        int64_t key, size;
        reset();

        while (true)
        {
            pair<int64_t, int64_t> key_size = get_next();
            if (key_size.first == -1 && key_size.second == -1)
            {
                break;
            }
            key = key_size.first;
            size = key_size.second;
            total_access_size += size;
            total_access_num++;
            min_item_size = min(min_item_size, size);
            min_item_size = max((int64_t)1, min_item_size);
            if (key_seen_time.count(key) == 0)
            {
                key_seen_time[key] = 1;
                unique_access_num++;
                unique_access_size += size;
            }
            else
            {
                key_seen_time[key]++;
            }
        }

        cout << "[FILE]: " << filename << endl;
        cout << "total_access_num: " << total_access_num << endl;
        cout << "total_access_size: " << total_access_size << endl;
        cout << "unique_access_num: " << unique_access_num << endl;
        cout << "unique_access_size: " << unique_access_size << endl;
        cout << "min_item_size: " << min_item_size << endl;

        reset();
    }
    int get_MAE_bin_idx(int64_t rd, int test_points)
    {
        int bin_idx = rd * test_points / unique_access_size;
        bin_idx = min(bin_idx, test_points - 1);
        return bin_idx;
    }
    int64_t get_MAE_bin_size(int bin_idx, int test_points)
    {
        int64_t bin_size = (unique_access_size / test_points) * (bin_idx + 1);
        return bin_size;
    }

    // min_item_size * MAEQ_factor ^ i >= rd
    int get_MAEQ_bin_idx(int64_t rd, int test_points)
    {
        int bin_idx = log(rd / min_item_size) / log(MAEQ_factor);
        bin_idx = min(bin_idx, test_points - 1);
        bin_idx = max(bin_idx, 0);
        return bin_idx;
    }

    int64_t get_MAEQ_bin_size(int bin_idx, int test_points)
    {
        int64_t bin_size = min_item_size * pow(MAEQ_factor, bin_idx);
        return bin_size;
    }

    void get_MAEQ_factor(int test_points)
    {
        MAEQ_factor = powf(1.0 * unique_access_size / min_item_size, 1.0 / (test_points - 1));
    }

    void get_feature(string output, string metric, int test_points = TEST_POINT_NUM)
    {
        bool is_MAE = metric == "MAE";
        get_MAEQ_factor(test_points);

        unordered_map<int64_t, key_feature_t> key_feature_map;
        key_feature_map.reserve(2 * unique_access_num);

        // set the load factor
        float desiredLoadFactor = 1.5;
        key_feature_map.max_load_factor(desiredLoadFactor);

        vector<double> BMRC_hist(test_points, 0);
        vector<double> OMRC_hist(test_points, 0);

        int64_t key, size;
        reset();
        int64_t time_now = 0;
        int bin_idx = 0;

        while (true)
        {
            pair<int64_t, int64_t> key_size = get_next();
            if (key_size.first == -1 && key_size.second == -1)
            {
                break;
            }
            key = key_size.first;
            size = key_size.second;

            if (key_feature_map.count(key) == 0)
            {
                key_feature_t feature{
                    .last_acc_time = time_now,
                    .frequency = 1,
                    .size = size};
                key_feature_map[key] = feature;
            }
            else
            {
                auto &feature = key_feature_map[key];
                int64_t last_acc_time = feature.last_acc_time;
                int64_t reuse_time = time_now - last_acc_time;

                feature.last_acc_time = time_now;
                feature.frequency += 1;

                if (reuse_time < unique_access_size)
                {
                    if (is_MAE)
                    {
                        bin_idx = get_MAE_bin_idx(reuse_time, test_points);
                    }
                    else
                    {
                        bin_idx = get_MAEQ_bin_idx(reuse_time, test_points);
                    }
                    BMRC_hist[bin_idx] += size;
                    OMRC_hist[bin_idx] += 1;
                }
            }
            time_now += size;
        }

        printf("%d\n", __LINE__);
        // Obtain normalized reuse time histogram
        for (int i = 0; i < test_points; i++)
        {
            BMRC_hist[i] = (double)BMRC_hist[i] / total_access_size;
            OMRC_hist[i] = (double)OMRC_hist[i] / total_access_num;
        }

        vector<key_feature_short_t> feature_vec;
        for (auto it = key_feature_map.begin(); it != key_feature_map.end(); ++it)
        {
            const auto &feature = it->second;
            key_feature_short_t short_feature{
                .frequency = feature.frequency,
                .size = feature.size};
            feature_vec.push_back(short_feature);
        }

        key_feature_map.clear();
        // sort feature_vec by frequency
        std::sort(feature_vec.begin(), feature_vec.end(), compare);

        vector<double> freq_ratio_threshold = {0.0005, 0.001, 0.002, 0.005, 0.01, 0.02, 0.05, 0.1, 0.2, 0.3};
        vector<double> freq_Byte_hist(10, 0);
        vector<double> freq_Obj_hist(10, 0);
        vector<int32_t> freq_ratio_index;
        int now_freq_idx = 0;

        // get the threshold
        for (int i; i < feature_vec.size(); i++)
        {
            while (now_freq_idx < 10 && i > feature_vec.size() * freq_ratio_threshold[now_freq_idx])
            {
                now_freq_idx++;
            }
            if (now_freq_idx >= 10)
            {
                // finish
                break;
            }
            auto feature = feature_vec[i];

            freq_Byte_hist[now_freq_idx] += feature.frequency * feature.size;
            freq_Obj_hist[now_freq_idx] += feature.frequency;
        }

        // normalize
        for (int i = 0; i < 10; i++)
        {
            freq_Byte_hist[i] /= total_access_size;
            freq_Obj_hist[i] /= total_access_num;
        }

        // Get the number of objects with the kth most accessed number
        int last_freq = 0;
        int now_k = 0;
        int next_k_index = 0;
        int cnt = 0;
        vector<int32_t> k_vec = {1, 2, 3, 10, 100};
        vector<int32_t> k_vec_cnt(5, 0);
        for (int i; i < feature_vec.size(); i++)
        {
            auto feature = feature_vec[i];
            int32_t this_freq = feature.frequency;

            if (last_freq == 0 || this_freq != last_freq)
            {
                if (now_k == k_vec[next_k_index])
                {
                    k_vec_cnt[next_k_index] = cnt;
                    next_k_index++;
                    if (next_k_index >= 5)
                    {
                        break;
                    }
                }
                last_freq = this_freq;
                now_k++;
                cnt = 1;
            }
            else
            {
                cnt++;
            }
        }

        double reaccess_byte_ratio = 1.0 * (total_access_size - unique_access_size) / total_access_size;
        double reaccess_object_ratio = 1.0 * (total_access_num - unique_access_num) / total_access_num;

        // save to feature file
        FILE *fp;
        fp = fopen(output.c_str(), "w");

        fprintf(fp, "%lf\n", reaccess_byte_ratio);
        fprintf(fp, "%lf\n", reaccess_object_ratio);

        for (int i = 0; i < test_points; i++)
        {
            fprintf(fp, "%lf\n", BMRC_hist[i]);
        }

        for (int i = 0; i < test_points; i++)
        {
            fprintf(fp, "%lf\n", OMRC_hist[i]);
        }

        for (int i = 0; i < 10; i++)
        {
            fprintf(fp, "%lf\n", freq_Byte_hist[i]);
        }

        for (int i = 0; i < 10; i++)
        {
            fprintf(fp, "%lf\n", freq_Obj_hist[i]);
        }

        for (int i = 0; i < 5; i++)
        {
            fprintf(fp, "%d\n", k_vec_cnt[i]);
        }

        fclose(fp);

        reset();
    }
};

int main(const int argc, char *argv[])
{
    cmdline::parser a;
    a.add<string>("trace", 't', "trace file name", true, "");
    a.add<string>("output", 'o', "output file name", true, "");
    a.add<string>("metric", 'c', "MAE/MAEQ", false, "MAE");
    a.add<string>("logpath", 'l', "logging file path leave blank if not necessary", false, "");
    a.add<string>("tracetype", 'y', "trace file type", false, "PLAIN");

    a.add<int64_t>("total_access_num", '\0', "total_access_num", false, 0);
    a.add<int64_t>("total_access_size", '\0', "total_access_size", false, 0);
    a.add<int64_t>("unique_access_num", '\0', "unique_access_num", false, 0);
    a.add<int64_t>("unique_access_size", '\0', "unique_access_size", false, 0);
    a.add<int64_t>("min_item_size", '\0', "min_item_size", false, 1);

    a.parse_check(argc, argv);
    string tracefile = a.get<string>("trace");
    string output = a.get<string>("output");
    string metric = a.get<string>("metric");
    string logpath = a.get<string>("logpath");
    string trace_type = a.get<string>("tracetype");
    int64_t total_access_num = a.get<int64_t>("total_access_num");
    int64_t total_access_size = a.get<int64_t>("total_access_size");
    int64_t unique_access_num = a.get<int64_t>("unique_access_num");
    int64_t unique_access_size = a.get<int64_t>("unique_access_size");
    int64_t min_item_size = a.get<int64_t>("min_item_size");

    if (logpath != "")
    {
        freopen(logpath.c_str(), "w", stdout);
    }

    printf("feature dump to %s\n", output.c_str());

    printf("tracefile: %s\n", tracefile.c_str());
    Trace trace(tracefile, trace_type, total_access_num, total_access_size, unique_access_num, unique_access_size, min_item_size);

    auto time_start = chrono::steady_clock::now();
    cout << "[metric]: " << metric << endl;

    trace.get_feature(output, metric);

    auto time_end = chrono::steady_clock::now();
    auto time_used = chrono::duration_cast<chrono::milliseconds>(time_end - time_start);
    cout << "Time used: " << time_used.count() << " ms" << endl;
    fclose(stdout);
    return 0;
}