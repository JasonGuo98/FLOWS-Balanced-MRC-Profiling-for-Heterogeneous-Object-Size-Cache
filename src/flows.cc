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

uint64_t hash_uint64(uint64_t x, uint64_t seed)
{
    x ^= seed;
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccd;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53;
    x ^= x >> 33;
    x ^= seed;
    return x;
}


long getMemoryUsage() {
    std::string status_file = "/proc/self/status";
    std::ifstream file(status_file);
    std::string line;

    while (std::getline(file, line)) {
        if (line.find("VmRSS") != std::string::npos) {
            long memory_usage_kb;
            sscanf(line.c_str(), "VmRSS: %ld kB", &memory_usage_kb);
            return memory_usage_kb;
        }
    }

    return -1;
}

template<typename KeyType, typename ValueType>
class LRUCacheFixCapacitySim {
private:
    int64_t capacity;
    int64_t now_size;
    list<pair<KeyType, ValueType>> cacheList;
    unordered_map<KeyType, typename list<pair<KeyType, ValueType>>::iterator> cacheMap;

public:
    LRUCacheFixCapacitySim(int64_t capacity) : capacity(capacity),now_size(0) {
        ;
    }
    bool count(const KeyType& key){
        if(cacheMap.count(key)) {
            return true;
        } else {
            return false;
        }
    }

    ValueType get(const KeyType& key) {
        if (cacheMap.find(key) == cacheMap.end()) {
            return ValueType(); 
        }

        auto it = cacheMap[key];
        cacheList.splice(cacheList.begin(), cacheList, it);

        return it->second;
    }

    void put(const KeyType& key, const ValueType& value) {
        if (cacheMap.find(key) != cacheMap.end()) {
            auto it = cacheMap[key];
            it->second = value;
            cacheList.splice(cacheList.begin(), cacheList, it);
        } else {
            if (now_size + value > capacity) {
                ;
                while(now_size > 0 && now_size + value > capacity){
                    auto last = prev(cacheList.end());
                    
                    KeyType lastKey = last->first;
                    ValueType lastValue = last->second;
                    cacheList.erase(last);
                    now_size -= lastValue;
                    cacheMap.erase(lastKey);
                }
                if(now_size + value <= capacity){
                    cacheList.push_front(std::make_pair(key, value));
                    cacheMap[key] = cacheList.begin();
                    now_size += value;
                }
            }
            else{
                cacheList.push_front(std::make_pair(key, value));
                cacheMap[key] = cacheList.begin();
                now_size += value;
            }
        }
    }
};

struct OracleReq{
    uint32_t timestamp;
    uint64_t obj_id;
    uint32_t obj_size;
    int64_t next_access_vtime;  // -1 if no next access
} ;// 48byte

std::vector<double> smooth_histogram(const std::vector<double>& histogram, int window_size, int startpoint) {
    std::vector<double> smoothed_histogram(histogram.size(), 0.0);
    for(size_t i = 0; i < startpoint; ++i){
        smoothed_histogram[i] = histogram[i];
    }
    for (size_t i = startpoint; i < histogram.size(); ++i) {
        double sum = 0.0;
        int count = 0;
        for (int j = -window_size / 2; j <= window_size / 2; ++j) {
            if (i + j >= 0 && i + j < histogram.size()) {
                sum += histogram[i + j];
                count++;
            }
        }
        smoothed_histogram[i] = sum / count;
    }
    return smoothed_histogram;
}

std::vector<double> normalize_histogram(const std::vector<double>& histogram, double constant_sum, int startpoint) {
    double now_sum = std::accumulate(histogram.begin(), histogram.end(), 0.0);
    double to_change_sum = std::accumulate(histogram.begin() + startpoint, histogram.end(), 0.0);
    double fix_sum = now_sum - to_change_sum;
    double mul_ratio = (constant_sum - fix_sum)/to_change_sum;
    std::vector<double> normalized_histogram(histogram.size());
    for (size_t i = 0; i < histogram.size(); ++i) {
        if(i < startpoint){
            normalized_histogram[i] = histogram[i];
        }
        else{
            normalized_histogram[i] = mul_ratio * histogram[i];
        }
    }
    return normalized_histogram;
}


template <typename K, typename V>
class MinValueMap
{
public:
    MinValueMap(size_t n) : n(n) {}

    bool find(const K &key){
        return map.count(key);
    }

    K insert(const K &key, const V &value)
    {
        auto it = map.find(key);
        if (it != map.end())
        {
            // Key already exists, update its value
            auto setIt = set.find({it->second, it->first});
            set.erase(setIt);
            set.insert({value, key});
            it->second = value;
        }
        else
        {
            // New key
            map[key] = value;
            if (set.size() < n)
            {
                set.insert({value, key});
            }
            else if (value < set.rbegin()->first)
            {
                auto last = *set.rbegin();
                set.erase(last);
                set.insert({value, key});
                map.erase(last.second);
                return last.second;
            }
        }
        return -1;
    }

    bool full() const
    {
        return set.size() == n;
    }

    bool empty() const
    {
        return set.empty();
    }

    V get_max_value() const
    {
        return set.rbegin()->first;
    }

    size_t n;
    std::set<std::pair<V, K>> set;
    std::unordered_map<K, V> map;
private:
};




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
            if (bin_trace_fp == NULL) {
                printf("Failed to open file: %s\n", filename.c_str());
                return;
            }
            printf("ok open file\n");
	    }
        else if (tracetype == "wiki")
        {
            ;
        }
        if(total_access_num!=0){
            ;
        }
        else{
            get_working_set_size();
        }
    }

    ~Trace(){
        if(bin_trace_fp){
            fclose(bin_trace_fp);
        }
    }

    void reset()
    {
        if(use_bin_file){
            fseek(bin_trace_fp, 0, SEEK_SET);
        }
        
        now_idx = 0;
        
    }

    pair<int64_t, int64_t> get_next()
    {
        if(use_bin_file){
            struct OracleReq req;
            now_idx++;
            if (now_idx % 10000000 == 0)
            {
                printf("read %ld lines\n", now_idx);
            }
            if(fread(&req, sizeof(OracleReq), 1, bin_trace_fp) == 1){
                if(req.obj_size<1){
                    req.obj_size = 1;
                }
                return make_pair(req.obj_id, req.obj_size);
            }
            else{
                return make_pair(-1, -1);
            }
        }
        else{
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
        int64_t bin_size = (unique_access_size / test_points)*(bin_idx + 1);
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
        // MAEQ_factor = (unique_access_size / min_item_size) ^ (1.0/(test_points - 1))
        MAEQ_factor = powf(1.0 * unique_access_size / min_item_size, 1.0 / (test_points - 1));
    }
};

void carra_mrc_fix_rate(Trace &trace, string output, string metric, double sample_metric, int test_points = TEST_POINT_NUM)
{
    long start_memory_usage_kb = getMemoryUsage();
    trace.reset();
    SplayTree<int32_t, uint64_t> rd_tree;
    unordered_map<int64_t, int32_t> item_last_access_time;
    trace.get_MAEQ_factor(test_points);
    printf("MAEQ_factor: %lf\n", trace.MAEQ_factor);

    bool is_MAE = metric == "MAE";

    vector<int64_t> BMRC_hist(test_points, 0);
    vector<int64_t> OMRC_hist(test_points, 0);
    vector<double> BMRC_ratio(test_points, 0);
    vector<double> OMRC_ratio(test_points, 0);

    vector<int64_t> small_cache_BMRC_hist(test_points, 0);
    vector<int64_t> small_cache_OMRC_hist(test_points, 0);
    vector<double> small_cache_BMRC_ratio(test_points, 0);
    vector<double> small_cache_OMRC_ratio(test_points, 0);

    double sample_rate = sample_metric;

    int64_t key, size;
    int64_t bin_idx = 0;
    int64_t now_time = 0;
    uint64_t sample_max = UINT64_MAX * sample_rate;
    int64_t sampled_cnt = 0, sampled_size = 0;

    int64_t small_cache_size = trace.unique_access_size / trace.unique_access_num * log10(1 / sample_rate) * (1 / sample_rate);
    LRU_fix_capacity<int64_t, int64_t> small_cache(small_cache_size);

    int64_t cnt = 0;
    while (true)
    {
        cnt++;
        pair<int64_t, int64_t> key_size = trace.get_next();
        if (key_size.first == -1 && key_size.second == -1)
        {
            break;
        }
        key = key_size.first;
        size = key_size.second;
        if (small_cache.find(key))
        {
            small_cache.set(key, size);
            int64_t rd = small_cache.getLastDistance();

            if (is_MAE)
            {
                bin_idx = trace.get_MAE_bin_idx(rd, test_points);
            }
            else
            {
                bin_idx = trace.get_MAEQ_bin_idx(rd, test_points);
            }
            small_cache_BMRC_hist[bin_idx] += size;
            small_cache_OMRC_hist[bin_idx] += 1;
        }
        else
        {
            small_cache.set(key, size);
        }

        uint64_t hash_value = hash_uint64(key, g_hash_mask);
        if (hash_value <= sample_max)
        {
            // sampled!
            now_time++;
            sampled_cnt++;
            sampled_size += size;
            if (item_last_access_time.count(key))
            {
                // in reuse distance tree
                int32_t last_acc_time = item_last_access_time[key];
                int64_t rd = rd_tree.getDistance(last_acc_time) / sample_rate;

                item_last_access_time[key] = now_time;
                rd_tree.erase(last_acc_time);
                rd_tree.insert(now_time, size);
                if (is_MAE)
                {
                    bin_idx = trace.get_MAE_bin_idx(rd, test_points);
                }
                else
                {
                    bin_idx = trace.get_MAEQ_bin_idx(rd, test_points);
                }
                BMRC_hist[bin_idx] += size;
                OMRC_hist[bin_idx] += 1;
            }
            else
            {
                // not in reuse distance tree
                item_last_access_time[key] = now_time;
                rd_tree.insert(now_time, size);
            }
        }
    }

    // adjust histogram
    int64_t should_sampled_size = trace.total_access_size * sample_rate;
    int64_t should_sampled_cnt = trace.total_access_num * sample_rate;
    BMRC_hist[0] += should_sampled_size - sampled_size;
    OMRC_hist[0] += should_sampled_cnt - sampled_cnt;

    // calculate CDF
    for (int i = 1; i < test_points; i++)
    {
        BMRC_hist[i] += BMRC_hist[i - 1];
        OMRC_hist[i] += OMRC_hist[i - 1];
        small_cache_BMRC_hist[i] += small_cache_BMRC_hist[i - 1];
        small_cache_OMRC_hist[i] += small_cache_OMRC_hist[i - 1];
    }

    for (int i = 0; i < test_points; i++)
    {
        BMRC_ratio[i] = min(1.0, 1.0 - (double)BMRC_hist[i] / should_sampled_size);
        OMRC_ratio[i] = min(1.0, 1.0 - (double)OMRC_hist[i] / should_sampled_cnt);

        small_cache_BMRC_ratio[i] = min(1.0, 1.0 - (double)small_cache_BMRC_hist[i] / trace.total_access_size);
        small_cache_OMRC_ratio[i] = min(1.0, 1.0 - (double)small_cache_OMRC_hist[i] / trace.total_access_num);
    }

    int small_cache_bin_idx = 0;
    if (is_MAE)
    {
        small_cache_bin_idx = trace.get_MAE_bin_idx(small_cache_size + 1, test_points) - 1;
    }
    else
    {
        small_cache_bin_idx = trace.get_MAEQ_bin_idx(small_cache_size + 1, test_points) - 1;
    }

    if (small_cache_bin_idx >= 0)
    {
        for (int i = 0; i < test_points; i++)
        {
            if (i <= small_cache_bin_idx)
            {
                BMRC_ratio[i] = small_cache_BMRC_ratio[i];
                OMRC_ratio[i] = small_cache_OMRC_ratio[i];
            }
            else
            {
                int64_t _now_cache_size = is_MAE ? trace.get_MAE_bin_size(i, test_points) : trace.get_MAEQ_bin_size(i, test_points);
                BMRC_ratio[i] = BMRC_ratio[i] + (small_cache_BMRC_ratio[small_cache_bin_idx] - BMRC_ratio[small_cache_bin_idx]) * exp(-(_now_cache_size - small_cache_size) / (4 * small_cache_size));
                OMRC_ratio[i] = OMRC_ratio[i] + (small_cache_OMRC_ratio[small_cache_bin_idx] - OMRC_ratio[small_cache_bin_idx]) * exp(-(_now_cache_size - small_cache_size) / (4 * small_cache_size));
            }
        }
    }

    out_file.open(output, ofstream::out | ofstream::trunc);
    out_file << "cache_size, BMRC_ratio, OMRC_ratio" << endl;
    printf("cache_size, BMRC_ratio, OMRC_ratio\n");
    for (int i = 0; i < test_points; i++)
    {
        out_file << (is_MAE ? trace.get_MAE_bin_size(i, test_points) : trace.get_MAEQ_bin_size(i, test_points)) << ", " << BMRC_ratio[i] << ", " << OMRC_ratio[i] << endl;
        printf("%ld, %lf, %lf\n", is_MAE ? trace.get_MAE_bin_size(i, test_points) : trace.get_MAEQ_bin_size(i, test_points), BMRC_ratio[i], OMRC_ratio[i]);
    }

    long end_memory_usage_kb = getMemoryUsage();
    std::cout << "Start Memory usage: " << start_memory_usage_kb << " kB" << std::endl;
    std::cout << "End Memory usage: " << end_memory_usage_kb << " kB" << std::endl;
    std::cout << "Memory usage: " << end_memory_usage_kb - start_memory_usage_kb << " kB" << std::endl;
}


void carra_mrc_fix_cnt(Trace &trace, string output, string metric, double sample_metric, int test_points = TEST_POINT_NUM)
{
    trace.reset();
    SplayTree<int32_t, uint64_t> rd_tree;
    unordered_map<int64_t, int32_t> item_last_access_time;
    trace.get_MAEQ_factor(test_points);
    printf("MAEQ_factor: %lf\n", trace.MAEQ_factor);

    bool is_MAE = metric == "MAE";

    vector<int64_t> BMRC_hist(test_points, 0);
    vector<int64_t> OMRC_hist(test_points, 0);
    vector<double> BMRC_ratio(test_points, 0);
    vector<double> OMRC_ratio(test_points, 0);

    vector<int64_t> small_cache_BMRC_hist(test_points, 0);
    vector<int64_t> small_cache_OMRC_hist(test_points, 0);
    vector<double> small_cache_BMRC_ratio(test_points, 0);
    vector<double> small_cache_OMRC_ratio(test_points, 0);

    

    int64_t sample_count = sample_metric;
    double _sample_rate = 1.0*sample_count/trace.unique_access_num;
    int64_t small_cache_count = std::min(sample_count/2.0, log10(1 / _sample_rate) * (1 / _sample_rate));
    sample_count -= small_cache_count;
    int64_t small_cache_size = trace.unique_access_size / trace.unique_access_num * small_cache_count;
    LRU_fix_capacity<int64_t, int64_t> small_cache(small_cache_size);

    MinValueMap<int64_t, uint64_t> sample_map(sample_count);
    double sample_rate = 1.0;

    int64_t key, size;
    int64_t bin_idx = 0;
    int64_t now_time = 0;
    uint64_t sample_max = UINT64_MAX * sample_rate;
    double sampled_cnt = 0, sampled_size = 0;
    while (true)
    {
        pair<int64_t, int64_t> key_size = trace.get_next();
        if (key_size.first == -1 && key_size.second == -1)
        {
            break;
        }
        key = key_size.first;
        size = key_size.second;
        uint64_t hash_value = hash_uint64(key, g_hash_mask);


        if (small_cache.find(key))
        {
            small_cache.set(key, size);
            int64_t rd = small_cache.getLastDistance();

            if (is_MAE)
            {
                bin_idx = trace.get_MAE_bin_idx(rd, test_points);
            }
            else
            {
                bin_idx = trace.get_MAEQ_bin_idx(rd, test_points);
            }
            small_cache_BMRC_hist[bin_idx] += size;
            small_cache_OMRC_hist[bin_idx] += 1;
        }
        else
        {
            small_cache.set(key, size);
        }

        if ((!sample_map.full()) || hash_value <= sample_map.get_max_value())
        {
            // sampled!
            if (!sample_map.full())
            {
                sample_rate = 1.0;
            }
            else
            {
                sample_rate = sample_map.get_max_value() * 1.0 / UINT64_MAX;
            }
            int64_t poped_key = sample_map.insert(key, hash_value);
            if (poped_key != -1)
            {
                int64_t poped_key_last_acc = item_last_access_time[poped_key];
                rd_tree.erase(poped_key_last_acc);
            }

            now_time++;
            sampled_cnt += 1.0 / sample_rate;
            sampled_size += 1.0 * size / sample_rate;
            if (item_last_access_time.count(key))
            {
                // in reuse distance tree
                int32_t last_acc_time = item_last_access_time[key];
                int64_t rd = rd_tree.getDistance(last_acc_time);

                item_last_access_time[key] = now_time;
                rd_tree.erase(last_acc_time);
                rd_tree.insert(now_time, size / sample_rate);
                if (is_MAE)
                {
                    bin_idx = trace.get_MAE_bin_idx(rd, test_points);
                }
                else
                {
                    bin_idx = trace.get_MAEQ_bin_idx(rd, test_points);
                }
                BMRC_hist[bin_idx] += 1.0 * size / sample_rate;
                OMRC_hist[bin_idx] += 1.0 / sample_rate;
            }
            else
            {
                // not in reuse distance tree
                item_last_access_time[key] = now_time;
                rd_tree.insert(now_time, size / sample_rate);
            }
        }
    }

    // adjust histogram
    int64_t should_sampled_size = trace.total_access_size;
    int64_t should_sampled_cnt = trace.total_access_num;

    BMRC_hist[0] += should_sampled_size - sampled_size;
    OMRC_hist[0] += should_sampled_cnt - sampled_cnt;

    for (int i = 0; i < test_points; i++)
    {
        BMRC_ratio[i] = min(1.0, 1.0 - (double)BMRC_hist[i] / should_sampled_size);
        OMRC_ratio[i] = min(1.0, 1.0 - (double)OMRC_hist[i] / should_sampled_cnt);
    }

    // calculate CDF
    for (int i = 1; i < test_points; i++)
    {
        BMRC_hist[i] += BMRC_hist[i - 1];
        OMRC_hist[i] += OMRC_hist[i - 1];
        small_cache_BMRC_hist[i] += small_cache_BMRC_hist[i - 1];
        small_cache_OMRC_hist[i] += small_cache_OMRC_hist[i - 1];
    }

    for (int i = 0; i < test_points; i++)
    {
        BMRC_ratio[i] = min(1.0, 1.0 - (double)BMRC_hist[i] / should_sampled_size);
        OMRC_ratio[i] = min(1.0, 1.0 - (double)OMRC_hist[i] / should_sampled_cnt);

        small_cache_BMRC_ratio[i] = min(1.0, 1.0 - (double)small_cache_BMRC_hist[i] / trace.total_access_size);
        small_cache_OMRC_ratio[i] = min(1.0, 1.0 - (double)small_cache_OMRC_hist[i] / trace.total_access_num);
    }

    int small_cache_bin_idx = 0;
    if (is_MAE)
    {
        small_cache_bin_idx = trace.get_MAE_bin_idx(small_cache_size + 1, test_points) - 1;
    }
    else
    {
        small_cache_bin_idx = trace.get_MAEQ_bin_idx(small_cache_size + 1, test_points) - 1;
    }

    if (small_cache_bin_idx >= 0)
    {
        for (int i = 0; i < test_points; i++)
        {
            if (i <= small_cache_bin_idx)
            {
                BMRC_ratio[i] = small_cache_BMRC_ratio[i];
                OMRC_ratio[i] = small_cache_OMRC_ratio[i];
            }
            else
            {
                int64_t _now_cache_size = is_MAE ? trace.get_MAE_bin_size(i, test_points) : trace.get_MAEQ_bin_size(i, test_points);
                BMRC_ratio[i] = BMRC_ratio[i] + (small_cache_BMRC_ratio[small_cache_bin_idx] - BMRC_ratio[small_cache_bin_idx]) * exp(-(_now_cache_size - small_cache_size) / (4 * small_cache_size));
                OMRC_ratio[i] = OMRC_ratio[i] + (small_cache_OMRC_ratio[small_cache_bin_idx] - OMRC_ratio[small_cache_bin_idx]) * exp(-(_now_cache_size - small_cache_size) / (4 * small_cache_size));
            }
        }
    }

    out_file.open(output, ofstream::out | ofstream::trunc);
    out_file << "cache_size, BMRC_ratio, OMRC_ratio" << endl;
    printf("cache_size, BMRC_ratio, OMRC_ratio\n");
    for (int i = 0; i < test_points; i++)
    {
        out_file << (is_MAE ? trace.get_MAE_bin_size(i, test_points) : trace.get_MAEQ_bin_size(i, test_points)) << ", " << BMRC_ratio[i] << ", " << OMRC_ratio[i] << endl;
        printf("%ld, %lf, %lf\n", is_MAE ? trace.get_MAE_bin_size(i, test_points) : trace.get_MAEQ_bin_size(i, test_points), BMRC_ratio[i], OMRC_ratio[i]);
    }
}


void flows_mrc_fix_rate(Trace &trace, string output, string metric, double sample_metric, int test_points = TEST_POINT_NUM, double slide_ratio = 0.5)
{
    printf("%s\n", __func__);

    long start_memory_usage_kb = getMemoryUsage();

    trace.reset();
    SplayTree<int32_t, uint64_t> rd_tree;
    unordered_map<int64_t, int32_t> item_last_access_time;
    trace.get_MAEQ_factor(test_points);
    printf("MAEQ_factor: %lf\n", trace.MAEQ_factor);

    bool is_MAE = metric == "MAE";


    vector<double> BMRC_hist(test_points, 0);
    vector<double> OMRC_hist(test_points, 0);
    vector<double> BMRC_ratio(test_points, 0);
    vector<double> OMRC_ratio(test_points, 0);

    double sample_rate = sample_metric;
    double weighted_sample_rate_base = sample_rate * (slide_ratio);
    double spatial_sample_rate = sample_rate * (1 - slide_ratio);

    int64_t key, size;
    int64_t bin_idx = 0;
    int64_t rd = 0;
    int64_t now_time = 0;
    uint64_t sample_max = UINT64_MAX * spatial_sample_rate;
    int64_t s_avg = trace.unique_access_size / trace.unique_access_num;
    double sampled_cnt = 0, sampled_size = 0;

    int64_t small_cache_count = log10(1 / sample_rate) * (1 / sample_rate);
    double moving_avg_cache_size = small_cache_count * s_avg;;
    // int64_t small_cache_count = 1;
    LRU_fix_count<int64_t, int64_t> small_cache(small_cache_count);

    while (true)
    {
        now_time++;
        pair<int64_t, int64_t> key_size = trace.get_next();
        if (key_size.first == -1 && key_size.second == -1)
        {
            break;
        }
        key = key_size.first;
        size = key_size.second;
        if (small_cache.find(key))
        {
            moving_avg_cache_size = moving_avg_cache_size * 0.999 + 0.001 * small_cache.getNowSize();
            // hit in cache filter
            small_cache.update(key, size);
            rd = small_cache.getLastDistance();
            if (is_MAE)
            {
                bin_idx = trace.get_MAE_bin_idx(rd, test_points);
            }
            else
            {
                bin_idx = trace.get_MAEQ_bin_idx(rd, test_points);
            }
            
            BMRC_hist[bin_idx] += size;
            OMRC_hist[bin_idx] += 1;
            sampled_cnt += 1;
            sampled_size += size;
        }
        else
        {
            // miss in cache filter
            uint64_t hash_value = hash_uint64(key, g_hash_mask);
            double weighted_sample_rate = min(1.0, weighted_sample_rate_base * size / s_avg);
            uint64_t weighted_sample_max = weighted_sample_rate * UINT64_MAX;
            uint64_t weighted_size = max(size, (int64_t)(s_avg/weighted_sample_rate_base));

            if (hash_value <= sample_max || hash_value <= weighted_sample_max)
            {
                if (item_last_access_time.count(key))
                {
                    int32_t last_acc_time = item_last_access_time[key];
                    rd = rd_tree.getDistance(last_acc_time);
                    rd += small_cache.getNowSize();
                    if (is_MAE)
                    {
                        bin_idx = trace.get_MAE_bin_idx(rd, test_points);
                    }
                    else
                    {
                        bin_idx = trace.get_MAEQ_bin_idx(rd, test_points);
                    }

                    if (hash_value <= sample_max)
                    {
                        OMRC_hist[bin_idx] += 1.0 / spatial_sample_rate;
                    }

                    if (hash_value <= weighted_sample_max)
                    {
                        BMRC_hist[bin_idx] += weighted_size;
                        rd_tree.erase(last_acc_time);
                    }
                }

                if (hash_value <= sample_max)
                {
                    sampled_cnt += 1.0 / spatial_sample_rate;
                }

                if (hash_value <= weighted_sample_max)
                {
                    sampled_size += weighted_size;
                }
            }

            small_cache.set(key, size);
            if (small_cache.length() > small_cache_count)
            {
                // TODO: prob insert
                auto pop_k_size = small_cache.pop();
                int64_t pop_key = pop_k_size.first;
                int64_t pop_size = pop_k_size.second;
                moving_avg_cache_size = moving_avg_cache_size * 0.999 + 0.001 * small_cache.getNowSize();

                // int64_t pop_key = key;
                // int64_t pop_size = size;

                uint64_t pop_hash_value = hash_uint64(pop_key, g_hash_mask);
                double pop_weighted_sample_rate = min(1.0, weighted_sample_rate_base * pop_size / s_avg);
                uint64_t pop_weighted_sample_max = pop_weighted_sample_rate * UINT64_MAX;

                if (pop_hash_value <= sample_max || pop_hash_value <= pop_weighted_sample_max)
                {
                    item_last_access_time[pop_key] = now_time;
                    if (pop_hash_value <= pop_weighted_sample_max)
                    {
                        rd_tree.insert(now_time, pop_size / pop_weighted_sample_rate);
                    }
                }
            }
        }
    }

    // modify hist
    int64_t avg_small_cache_size = moving_avg_cache_size;
    int64_t accu_start_bin_size = moving_avg_cache_size + s_avg/sample_rate * log2(1/sample_rate) * 100;
    int start_bin_idx = 0;
    int accu_start_bin_idx = 0;
    if (is_MAE)
    {
        start_bin_idx = trace.get_MAE_bin_idx(avg_small_cache_size + 1, test_points);
        accu_start_bin_idx = trace.get_MAE_bin_idx(accu_start_bin_size + 1, test_points);
    }
    else
    {

        start_bin_idx = trace.get_MAEQ_bin_idx(avg_small_cache_size + 1, test_points);
        accu_start_bin_idx = trace.get_MAEQ_bin_idx(accu_start_bin_size + 1, test_points);
    }
    printf("start_bin_idx: %d\n", start_bin_idx);
    double remain_size = trace.total_access_size - sampled_size;
    double remain_cnt = trace.total_access_num - sampled_cnt;
    printf("%lf, %ld\n", sampled_size, trace.total_access_size);
    printf("%lf, %ld\n", sampled_cnt, trace.total_access_num);

    double ok_size = 0;
    double ok_cnt = 0;
    double in_accu_size = 0;
    double in_accu_cnt = 0;
    for(int i = 0; i < test_points; i++){
        ok_size += BMRC_hist[i];
        ok_cnt += OMRC_hist[i];
    }
    for(int i = start_bin_idx; i < accu_start_bin_idx; i++){
        in_accu_size += BMRC_hist[i];
        in_accu_cnt += OMRC_hist[i];
    }


    if(start_bin_idx == 0 && is_MAE){
        BMRC_hist[0] += remain_size;
        OMRC_hist[0] += remain_cnt;
    }
    else{
        double bhr_end = (in_accu_size + remain_size);
        double ohr_end = (in_accu_cnt + remain_cnt);
        double now_bhr_end = (in_accu_size);
        double now_ohr_end = (in_accu_cnt);

        double byte_mul_ratio = bhr_end/now_bhr_end;
        double obj_mul_ratio = ohr_end/now_ohr_end;
        printf("%lf, %lf\n",byte_mul_ratio,  obj_mul_ratio);

        for (int i = start_bin_idx; i < accu_start_bin_idx; i++)
        {
            ;
            BMRC_hist[i] *= byte_mul_ratio;
            OMRC_hist[i] *= obj_mul_ratio;
        }
    }


    // calculate CDF
    for (int i = 1; i < test_points; i++)
    {
        BMRC_hist[i] += BMRC_hist[i - 1];
        OMRC_hist[i] += OMRC_hist[i - 1];
    }


    for (int i = 0; i < test_points; i++)
    {
        BMRC_ratio[i] = min(1.0, 1.0 - (double)BMRC_hist[i] / trace.total_access_size);
        OMRC_ratio[i] = min(1.0, 1.0 - (double)OMRC_hist[i] / trace.total_access_num);
    }

    out_file.open(output, ofstream::out | ofstream::trunc);
    out_file << "cache_size, BMRC_ratio, OMRC_ratio" << endl;
    printf("cache_size, BMRC_ratio, OMRC_ratio\n");
    for (int i = 0; i < test_points; i++)
    {
        out_file << (is_MAE ? trace.get_MAE_bin_size(i, test_points) : trace.get_MAEQ_bin_size(i, test_points)) << ", " << BMRC_ratio[i] << ", " << OMRC_ratio[i] << endl;
    }

    long end_memory_usage_kb = getMemoryUsage();
    std::cout << "Start Memory usage: " << start_memory_usage_kb << " kB" << std::endl;
    std::cout << "End Memory usage: " << end_memory_usage_kb << " kB" << std::endl;
    std::cout << "Memory usage: " << end_memory_usage_kb - start_memory_usage_kb << " kB" << std::endl;
}

void flows_mrc_fix_cnt(Trace &trace, string output, string metric, double sample_metric, int test_points = TEST_POINT_NUM, double weighted_slide_ratio = 0.5)
{
    trace.reset();
    SplayTree<int64_t, int64_t> small_req_rd_tree;
    SplayTree<int64_t, int64_t> large_req_rd_tree;
    unordered_map<int64_t, int32_t> item_last_access_time;
    trace.get_MAEQ_factor(test_points);
    printf("MAEQ_factor: %lf\n", trace.MAEQ_factor);

    bool is_MAE = metric == "MAE";

    vector<double> BMRC_hist(test_points, 0);
    vector<double> OMRC_hist(test_points, 0);
    vector<double> BMRC_ratio(test_points, 0);
    vector<double> OMRC_ratio(test_points, 0);

    int sample_count = sample_metric;
    double _sample_rate = 1.0*sample_count/trace.unique_access_num;
    int64_t small_cache_count = std::min(sample_count/2.0, log10(1 / _sample_rate) * (1 / _sample_rate));
    sample_count -= small_cache_count;


    int64_t weighted_sample_cnt = sample_count * (weighted_slide_ratio);
    double spatial_sample_cnt = sample_count * (1 - weighted_slide_ratio);
    MinValueMap<int64_t, double> weighted_sample_map(weighted_sample_cnt);
    MinValueMap<int64_t, uint64_t> spatial_sample_map(spatial_sample_cnt);
    double weighted_sample_rate = 1.0;
    double spatial_sample_rate = 1.0;

    int64_t key, size;
    int64_t bin_idx = 0;
    int64_t rd = 0;
    int64_t now_time = 0;
    double weighted_sample_max = UINT64_MAX;
    uint64_t spatial_sample_max = UINT64_MAX;
    double s_avg = trace.unique_access_size / trace.unique_access_num;
    double sampled_cnt = 0, sampled_size = 0;

    
    double moving_avg_cache_size = small_cache_count * s_avg;
    LRU_fix_count<int64_t, int64_t> small_cache(small_cache_count);

    while (true)
    {
        now_time++;
        pair<int64_t, int64_t> key_size = trace.get_next();
        if (key_size.first == -1 && key_size.second == -1)
        {
            break;
        }
        key = key_size.first;
        size = key_size.second;

        bool hit_in_cache_filter =false;
        if (small_cache.find(key))
        {
            moving_avg_cache_size = moving_avg_cache_size * 0.999 + 0.001 * small_cache.getNowSize();
            small_cache.update(key, size);
            rd = small_cache.getLastDistance();
            if (is_MAE)
            {
                bin_idx = trace.get_MAE_bin_idx(rd, test_points);
            }
            else
            {
                bin_idx = trace.get_MAEQ_bin_idx(rd, test_points);
            }
            
            BMRC_hist[bin_idx] += size;
            OMRC_hist[bin_idx] += 1;
            sampled_cnt += 1;
            sampled_size += size;
            hit_in_cache_filter = true;
        }
        else{
            // insert to cache filter
            small_cache.set(key, size);
            if (small_cache.length() > small_cache_count)
            {
                // TODO: prob insert
                auto pop_k_size = small_cache.pop();
                int64_t pop_key = pop_k_size.first;
                int64_t pop_size = pop_k_size.second;
                moving_avg_cache_size = moving_avg_cache_size * 0.999 + 0.001 * small_cache.getNowSize();
            }
        }
        
        {
            uint64_t hash_value = hash_uint64(key, g_hash_mask);
            double weighted_hash = ((1.0 * hash_value / UINT64_MAX * s_avg / size));
            bool is_spatial_sampled = (!spatial_sample_map.full())||spatial_sample_map.find(key)  || hash_value <= spatial_sample_map.get_max_value();
            bool is_weighted_sampled = (!weighted_sample_map.full())||weighted_sample_map.find(key) || weighted_hash <= weighted_sample_map.get_max_value();
            double its_p = min(1.0, weighted_sample_rate * (size + 0.01) / s_avg);

            if (is_spatial_sampled || is_weighted_sampled)
            {
                ;
                if (is_spatial_sampled)
                    spatial_sample_map.insert(key, hash_value);

                if (is_weighted_sampled)
                {
                    int64_t poped_key = weighted_sample_map.insert(key, weighted_hash);
                    if (poped_key != -1)
                    {
                        int64_t poped_key_last_acc = item_last_access_time[poped_key];
                        large_req_rd_tree.erase(poped_key_last_acc);
                        small_req_rd_tree.erase(poped_key_last_acc);
                    }
                }

                if (!spatial_sample_map.full())
                {
                    spatial_sample_rate = 1.0;
                    weighted_sample_rate = 1.0;
                }
                else
                {
                    spatial_sample_rate = spatial_sample_map.get_max_value() * 1.0 / UINT64_MAX;
                    weighted_sample_rate = weighted_sample_map.get_max_value();
                }

                if (item_last_access_time.count(key))
                {
                    // have seen
                    // printf("%ld\n", key);
                    int32_t last_acc_time = item_last_access_time[key];
                    item_last_access_time[key] = now_time;
                    int64_t small_req_rd = small_req_rd_tree.getDistance(last_acc_time) * s_avg / weighted_sample_rate;
                    int64_t large_req_rd = large_req_rd_tree.getDistance(last_acc_time);
                    int64_t cache_filter_rd = small_cache.getNowSize();
                    rd = small_req_rd * 1.0 + large_req_rd;
                    

                    if (is_weighted_sampled)
                    {
                        if (its_p == 1.0)
                        {
                            ;
                        }
                        else
                        {
                            small_req_rd -= s_avg;
                            small_req_rd += size * weighted_sample_rate;
                        }
                    }
                    else
                    {
                        rd += size;
                    }

                    if (is_MAE)
                    {
                        bin_idx = trace.get_MAE_bin_idx(rd, test_points);
                    }
                    else
                    {
                        bin_idx = trace.get_MAEQ_bin_idx(rd, test_points);
                    }

                    if (!hit_in_cache_filter && is_spatial_sampled)
                    {
                        // spatial sample
                        if (bin_idx < test_points)
                        {
                            OMRC_hist[bin_idx] += 1.0 / spatial_sample_rate;
                        }
                        sampled_cnt += 1.0 / spatial_sample_rate;
                    }

                    if (is_weighted_sampled)
                    {
                        if(!hit_in_cache_filter){
                            if (bin_idx < test_points)
                            {
                                BMRC_hist[bin_idx] += 1.0 * size / its_p;
                            }
                            sampled_size += 1.0 * size / its_p;
                        }
                        
                        if (its_p == 1.0)
                        {
                            // it is a big object
                            large_req_rd_tree.erase(last_acc_time);
                            small_req_rd_tree.erase(last_acc_time);
                            large_req_rd_tree.insert(item_last_access_time[key], size);
                        }
                        else
                        {
                            // it becomes a small one
                            large_req_rd_tree.erase(last_acc_time);
                            small_req_rd_tree.erase(last_acc_time);
                            small_req_rd_tree.insert(item_last_access_time[key], 1);
                        }
                    }
                }
                else
                {
                    // not seen
                    item_last_access_time[key] = now_time;
                    if (!hit_in_cache_filter && is_spatial_sampled)
                    {
                        sampled_cnt += 1.0 / spatial_sample_rate;
                    }
                    if (is_weighted_sampled)
                    {
                        if(!hit_in_cache_filter){
                            sampled_size += 1.0 * size / its_p;
                        }
                        if (its_p == 1.0)
                        {
                            // it is a big object
                            large_req_rd_tree.insert(item_last_access_time[key], size);
                        }
                        else
                        {
                            // it becomes a small one
                            small_req_rd_tree.insert(item_last_access_time[key], 1);
                        }
                    }
                }
                
            }
            else{
                if(item_last_access_time.count(key)){
                    item_last_access_time.erase(key);
                }
            }
        }

    }
    // adjust histogram
    int64_t should_sampled_size = trace.total_access_size;
    int64_t should_sampled_cnt = trace.total_access_num;
    printf("should_sampled_size: %ld\n", should_sampled_size);
    printf("sampled_size: %lf\n", sampled_size);
    printf("should_sampled_cnt: %ld\n", should_sampled_cnt);
    printf("sampled_cnt: %lf\n", sampled_cnt);
    printf("spatial_sample_rate: %lf\n", spatial_sample_rate);
    printf("weighted_sample_rate: %lf\n", weighted_sample_rate);


    // modify hist
    int64_t avg_small_cache_size = moving_avg_cache_size;
    int64_t accu_start_bin_size = moving_avg_cache_size + s_avg/(spatial_sample_rate + weighted_sample_rate) * log2(1/(spatial_sample_rate + weighted_sample_rate)) * 100;
    int start_bin_idx = 0;
    int accu_start_bin_idx = 0;
    if (is_MAE)
    {
        start_bin_idx = trace.get_MAE_bin_idx(avg_small_cache_size + 1, test_points);
        accu_start_bin_idx = trace.get_MAE_bin_idx(accu_start_bin_size + 1, test_points);
    }
    else
    {

        start_bin_idx = trace.get_MAEQ_bin_idx(avg_small_cache_size + 1, test_points);
        accu_start_bin_idx = trace.get_MAEQ_bin_idx(accu_start_bin_size + 1, test_points);
    }
    printf("start_bin_idx: %d\n", start_bin_idx);
    printf("accu_start_bin_idx: %d\n", accu_start_bin_idx);
    double remain_size = trace.total_access_size - sampled_size;
    double remain_cnt = trace.total_access_num - sampled_cnt;
    printf("%lf, %ld\n", sampled_size, trace.total_access_size);
    printf("%lf, %ld\n", sampled_cnt, trace.total_access_num);

    double ok_size = 0;
    double ok_cnt = 0;
    double in_accu_size = 0;
    double in_accu_cnt = 0;
    for(int i = 0; i < test_points; i++){
        ok_size += BMRC_hist[i];
        ok_cnt += OMRC_hist[i];
    }
    for(int i = start_bin_idx; i < accu_start_bin_idx; i++){
        in_accu_size += BMRC_hist[i];
        in_accu_cnt += OMRC_hist[i];
    }


    if(start_bin_idx == 0 && is_MAE){
        BMRC_hist[0] += remain_size;
        OMRC_hist[0] += remain_cnt;
    }
    else{
        double bhr_end = (in_accu_size + remain_size);
        double ohr_end = (in_accu_cnt + remain_cnt);
        double now_bhr_end = (in_accu_size);
        double now_ohr_end = (in_accu_cnt);

        double byte_mul_ratio = bhr_end/now_bhr_end;
        double obj_mul_ratio = ohr_end/now_ohr_end;
        printf("%lf, %lf\n",byte_mul_ratio,  obj_mul_ratio);

        for (int i = start_bin_idx; i < accu_start_bin_idx; i++)
        {
            ;
            BMRC_hist[i] *= byte_mul_ratio;
            OMRC_hist[i] *= obj_mul_ratio;
        }
    }

    // calculate CDF
    for (int i = 1; i < test_points; i++)
    {

        BMRC_hist[i] += BMRC_hist[i - 1];
        OMRC_hist[i] += OMRC_hist[i - 1];
    }

    for (int i = 0; i < test_points; i++)
    {
        BMRC_ratio[i] = min(1.0, 1.0 - (double)BMRC_hist[i] / trace.total_access_size);
        OMRC_ratio[i] = min(1.0, 1.0 - (double)OMRC_hist[i] / trace.total_access_num);
    }

    out_file.open(output, ofstream::out | ofstream::trunc);
    out_file << "cache_size, BMRC_ratio, OMRC_ratio" << endl;
    printf("cache_size, BMRC_ratio, OMRC_ratio\n");
    for (int i = 0; i < test_points; i++)
    {
        out_file << (is_MAE ? trace.get_MAE_bin_size(i, test_points) : trace.get_MAEQ_bin_size(i, test_points)) << ", " << BMRC_ratio[i] << ", " << OMRC_ratio[i] << endl;
        printf("%ld, %lf, %lf\n", is_MAE ? trace.get_MAE_bin_size(i, test_points) : trace.get_MAEQ_bin_size(i, test_points), BMRC_ratio[i], OMRC_ratio[i]);
    }
}

void shards_adj_mrc_fix_rate(Trace &trace, string output, string metric, double sample_metric, int test_points = TEST_POINT_NUM)
{
    long start_memory_usage_kb = getMemoryUsage();
    trace.reset();
    SplayTree<int32_t, uint64_t> rd_tree;
    unordered_map<int64_t, int32_t> item_last_access_time;
    trace.get_MAEQ_factor(test_points);
    printf("MAEQ_factor: %lf\n", trace.MAEQ_factor);

    bool is_MAE = metric == "MAE";

    vector<int64_t> BMRC_hist(test_points, 0);
    vector<int64_t> OMRC_hist(test_points, 0);
    vector<double> BMRC_ratio(test_points, 0);
    vector<double> OMRC_ratio(test_points, 0);
    double sample_rate = sample_metric;

    int64_t key, size;
    int64_t bin_idx = 0;
    int64_t now_time = 0;
    uint64_t sample_max = UINT64_MAX * sample_rate;
    int64_t sampled_cnt = 0, sampled_size = 0;
    while (true)
    {
        pair<int64_t, int64_t> key_size = trace.get_next();
        if (key_size.first == -1 && key_size.second == -1)
        {
            break;
        }
        key = key_size.first;
        size = key_size.second;
        uint64_t hash_value = hash_uint64(key, g_hash_mask);

        if (hash_value <= sample_max)
        {
            // sampled!
            now_time++;
            sampled_cnt++;
            sampled_size += size;
            if (item_last_access_time.count(key))
            {
                // in reuse distance tree
                int32_t last_acc_time = item_last_access_time[key];
                int64_t rd = rd_tree.getDistance(last_acc_time) / sample_rate;

                item_last_access_time[key] = now_time;
                rd_tree.erase(last_acc_time);
                rd_tree.insert(now_time, size);
                if (is_MAE)
                {
                    bin_idx = trace.get_MAE_bin_idx(rd, test_points);
                }
                else
                {
                    bin_idx = trace.get_MAEQ_bin_idx(rd, test_points);
                }
                BMRC_hist[bin_idx] += size;
                OMRC_hist[bin_idx] += 1;
            }
            else
            {
                // not in reuse distance tree
                item_last_access_time[key] = now_time;
                rd_tree.insert(now_time, size);
            }
        }
    }

    // adjust histogram
    int64_t should_sampled_size = trace.total_access_size * sample_rate;
    int64_t should_sampled_cnt = trace.total_access_num * sample_rate;
    BMRC_hist[0] += should_sampled_size - sampled_size;
    OMRC_hist[0] += should_sampled_cnt - sampled_cnt;

    // calculate CDF
    for (int i = 1; i < test_points; i++)
    {
        BMRC_hist[i] += BMRC_hist[i - 1];
        OMRC_hist[i] += OMRC_hist[i - 1];
    }

    for (int i = 0; i < test_points; i++)
    {
        BMRC_ratio[i] = min(1.0, 1.0 - (double)BMRC_hist[i] / should_sampled_size);
        OMRC_ratio[i] = min(1.0, 1.0 - (double)OMRC_hist[i] / should_sampled_cnt);
    }

    out_file.open(output, ofstream::out | ofstream::trunc);
    out_file << "cache_size, BMRC_ratio, OMRC_ratio" << endl;
    printf("cache_size, BMRC_ratio, OMRC_ratio\n");
    for (int i = 0; i < test_points; i++)
    {
        out_file << (is_MAE ? trace.get_MAE_bin_size(i, test_points) : trace.get_MAEQ_bin_size(i, test_points)) << ", " << BMRC_ratio[i] << ", " << OMRC_ratio[i] << endl;
        printf("%ld, %lf, %lf\n", is_MAE ? trace.get_MAE_bin_size(i, test_points) : trace.get_MAEQ_bin_size(i, test_points), BMRC_ratio[i], OMRC_ratio[i]);
    }

    long end_memory_usage_kb = getMemoryUsage();
    std::cout << "Start Memory usage: " << start_memory_usage_kb << " kB" << std::endl;
    std::cout << "End Memory usage: " << end_memory_usage_kb << " kB" << std::endl;
    std::cout << "Memory usage: " << end_memory_usage_kb - start_memory_usage_kb << " kB" << std::endl;
}

void shards_adj_mrc_fix_cnt(Trace &trace, string output, string metric, double sample_metric, int test_points = TEST_POINT_NUM)
{
    trace.reset();
    SplayTree<int32_t, uint64_t> rd_tree;
    unordered_map<int64_t, int32_t> item_last_access_time;
    trace.get_MAEQ_factor(test_points);
    printf("MAEQ_factor: %lf\n", trace.MAEQ_factor);

    bool is_MAE = metric == "MAE";

    vector<double> BMRC_hist(test_points, 0);
    vector<double> OMRC_hist(test_points, 0);
    vector<double> BMRC_ratio(test_points, 0);
    vector<double> OMRC_ratio(test_points, 0);
    int64_t sample_count = sample_metric;
    MinValueMap<int64_t, uint64_t> sample_map(sample_count);
    double sample_rate = 1.0;

    int64_t key, size;
    int64_t bin_idx = 0;
    int64_t now_time = 0;
    uint64_t sample_max = UINT64_MAX * sample_rate;
    double sampled_cnt = 0, sampled_size = 0;
    while (true)
    {
        pair<int64_t, int64_t> key_size = trace.get_next();
        if (key_size.first == -1 && key_size.second == -1)
        {
            break;
        }
        key = key_size.first;
        size = key_size.second;
        uint64_t hash_value = hash_uint64(key, g_hash_mask);

        if ((!sample_map.full()) || hash_value <= sample_map.get_max_value())
        {
            // sampled!
            if (!sample_map.full())
            {
                sample_rate = 1.0;
            }
            else
            {
                sample_rate = sample_map.get_max_value() * 1.0 / UINT64_MAX;
            }
            int64_t poped_key = sample_map.insert(key, hash_value);
            if (poped_key != -1)
            {
                int64_t poped_key_last_acc = item_last_access_time[poped_key];
                rd_tree.erase(poped_key_last_acc);
            }

            now_time++;
            sampled_cnt += 1.0 / sample_rate;
            sampled_size += 1.0 * size / sample_rate;
            if (item_last_access_time.count(key))
            {
                // in reuse distance tree
                int32_t last_acc_time = item_last_access_time[key];
                int64_t rd = rd_tree.getDistance(last_acc_time);

                item_last_access_time[key] = now_time;
                rd_tree.erase(last_acc_time);
                rd_tree.insert(now_time, size / sample_rate);
                if (is_MAE)
                {
                    bin_idx = trace.get_MAE_bin_idx(rd, test_points);
                }
                else
                {
                    bin_idx = trace.get_MAEQ_bin_idx(rd, test_points);
                }
                BMRC_hist[bin_idx] += 1.0 * size / sample_rate;
                OMRC_hist[bin_idx] += 1.0 / sample_rate;
            }
            else
            {
                // not in reuse distance tree
                item_last_access_time[key] = now_time;
                rd_tree.insert(now_time, size / sample_rate);
            }
        }
    }

    // adjust the histogram
    int64_t should_sampled_size = trace.total_access_size;
    int64_t should_sampled_cnt = trace.total_access_num;

    BMRC_hist[0] += should_sampled_size - sampled_size;
    OMRC_hist[0] += should_sampled_cnt - sampled_cnt;

    // calculate CDF
    for (int i = 1; i < test_points; i++)
    {
        BMRC_hist[i] += BMRC_hist[i - 1];
        OMRC_hist[i] += OMRC_hist[i - 1];
    }

    for (int i = 0; i < test_points; i++)
    {
        BMRC_ratio[i] = min(1.0, 1.0 - (double)BMRC_hist[i] / should_sampled_size);
        OMRC_ratio[i] = min(1.0, 1.0 - (double)OMRC_hist[i] / should_sampled_cnt);
    }

    out_file.open(output, ofstream::out | ofstream::trunc);
    out_file << "cache_size, BMRC_ratio, OMRC_ratio" << endl;
    printf("cache_size, BMRC_ratio, OMRC_ratio\n");
    for (int i = 0; i < test_points; i++)
    {
        out_file << (is_MAE ? trace.get_MAE_bin_size(i, test_points) : trace.get_MAEQ_bin_size(i, test_points)) << ", " << BMRC_ratio[i] << ", " << OMRC_ratio[i] << endl;
        printf("%ld, %lf, %lf\n", is_MAE ? trace.get_MAE_bin_size(i, test_points) : trace.get_MAEQ_bin_size(i, test_points), BMRC_ratio[i], OMRC_ratio[i]);
    }
}

void real_mrc(Trace &trace, string output, string metric, int test_points = TEST_POINT_NUM)
{
    long start_memory_usage_kb = getMemoryUsage();
    trace.reset();
    SplayTree<int32_t, uint64_t> rd_tree;
    unordered_map<int64_t, int32_t> item_last_access_time;
    trace.get_MAEQ_factor(test_points);
    printf("MAEQ_factor: %lf\n", trace.MAEQ_factor);

    bool is_MAE = metric == "MAE";

    vector<int64_t> BMRC_hist(test_points, 0);
    vector<int64_t> OMRC_hist(test_points, 0);
    vector<double> BMRC_ratio(test_points, 0);
    vector<double> OMRC_ratio(test_points, 0);

    int64_t key, size;
    int64_t bin_idx = 0;
    int64_t now_time = 0;
    while (true)
    {
        now_time++;
        pair<int64_t, int64_t> key_size = trace.get_next();
        if (key_size.first == -1 && key_size.second == -1)
        {
            break;
        }
        key = key_size.first;
        size = key_size.second;
        if (item_last_access_time.count(key))
        {
            // in reuse distance tree
            int32_t last_acc_time = item_last_access_time[key];
            int64_t rd = rd_tree.getDistance(last_acc_time);

            item_last_access_time[key] = now_time;
            rd_tree.erase(last_acc_time);
            rd_tree.insert(now_time, size);
            if (is_MAE)
            {
                bin_idx = trace.get_MAE_bin_idx(rd, test_points);
            }
            else
            {
                bin_idx = trace.get_MAEQ_bin_idx(rd, test_points);
            }
            BMRC_hist[bin_idx] += size;
            OMRC_hist[bin_idx] += 1;
        }
        else
        {
            // not in reuse distance tree
            item_last_access_time[key] = now_time;
            rd_tree.insert(now_time, size);
        }
    }

    // calculate CDF
    for (int i = 1; i < test_points; i++)
    {
        BMRC_hist[i] += BMRC_hist[i - 1];
        OMRC_hist[i] += OMRC_hist[i - 1];
    }

    for (int i = 0; i < test_points; i++)
    {
        BMRC_ratio[i] = min(1.0, 1.0 - (double)BMRC_hist[i] / trace.total_access_size);
        OMRC_ratio[i] = min(1.0, 1.0 - (double)OMRC_hist[i] / trace.total_access_num);
    }
    out_file.open(output, ofstream::out | ofstream::trunc);
    out_file << "cache_size, BMRC_ratio, OMRC_ratio" << endl;
    printf("cache_size, BMRC_ratio, OMRC_ratio\n");
    for (int i = 0; i < test_points; i++)
    {
        out_file << (is_MAE ? trace.get_MAE_bin_size(i, test_points) : trace.get_MAEQ_bin_size(i, test_points)) << ", " << BMRC_ratio[i] << ", " << OMRC_ratio[i] << endl;
        printf("%ld, %lf, %lf\n", is_MAE ? trace.get_MAE_bin_size(i, test_points) : trace.get_MAEQ_bin_size(i, test_points), BMRC_ratio[i], OMRC_ratio[i]);
    }

    
    long end_memory_usage_kb = getMemoryUsage();
    std::cout << "Start Memory usage: " << start_memory_usage_kb << " kB" << std::endl;
    std::cout << "End Memory usage: " << end_memory_usage_kb << " kB" << std::endl;
    std::cout << "Memory usage: " << end_memory_usage_kb - start_memory_usage_kb << " kB" << std::endl;
}


void real_mrc_sim(Trace &trace, string output, string metric, int test_points = TEST_POINT_NUM)
{
    printf("real_MRC_SIM\n");
    trace.reset();
    SplayTree<int32_t, uint64_t> rd_tree;
    unordered_map<int64_t, int32_t> item_last_access_time;
    trace.get_MAEQ_factor(test_points);
    printf("MAEQ_factor: %lf\n", trace.MAEQ_factor);

    bool is_MAE = metric == "MAE";

    vector<int64_t> BMRC_hist(test_points, 0);
    vector<int64_t> OMRC_hist(test_points, 0);
    vector<double> BMRC_ratio(test_points, 0);
    vector<double> OMRC_ratio(test_points, 0);

    int64_t key, size;
    int64_t bin_idx = 0;
    int64_t now_time = 0;


    vector<int64_t> small_cache_BMRC_hist(test_points, 0);
    vector<int64_t> small_cache_OMRC_hist(test_points, 0);
    vector<double> small_cache_BMRC_ratio(test_points, 0);
    vector<double> small_cache_OMRC_ratio(test_points, 0);


    double sample_rate = 0.000001;

    int64_t small_cache_size = trace.unique_access_size / trace.unique_access_num * log10(1 / sample_rate) * (1 / sample_rate);
    LRU_fix_capacity<int64_t, int64_t> small_cache(small_cache_size);

    int64_t cnt = 0;
    while (true)
    {
        cnt++;
        pair<int64_t, int64_t> key_size = trace.get_next();
        if (key_size.first == -1 && key_size.second == -1)
        {
            break;
        }
        key = key_size.first;
        size = key_size.second;
        if (small_cache.find(key))
        {
            small_cache.set(key, size);
            int64_t rd = small_cache.getLastDistance();

            if (is_MAE)
            {
                bin_idx = trace.get_MAE_bin_idx(rd, test_points);
            }
            else
            {
                bin_idx = trace.get_MAEQ_bin_idx(rd, test_points);
            }
            small_cache_BMRC_hist[bin_idx] += size;
            small_cache_OMRC_hist[bin_idx] += 1;
        }
        else
        {
            small_cache.set(key, size);
        }
    }

    // calculate CDF
    for (int i = 1; i < test_points; i++)
    {
        small_cache_BMRC_hist[i] += small_cache_BMRC_hist[i - 1];
        small_cache_OMRC_hist[i] += small_cache_OMRC_hist[i - 1];
    }

    for (int i = 0; i < test_points; i++)
    {
        BMRC_ratio[i] = min(1.0, 1.0 - (double)small_cache_BMRC_hist[i] / trace.total_access_size);
        OMRC_ratio[i] = min(1.0, 1.0 - (double)small_cache_OMRC_hist[i] / trace.total_access_num);
    }

    int small_cache_bin_idx = 0;
    if (is_MAE)
    {
        small_cache_bin_idx = trace.get_MAE_bin_idx(small_cache_size + 1, test_points) - 1;
    }
    else
    {
        small_cache_bin_idx = trace.get_MAEQ_bin_idx(small_cache_size + 1, test_points) - 1;
    }

    printf("small_cache_bin_idx: %d\n", small_cache_bin_idx);
    for(int i = small_cache_bin_idx; i < test_points; i++){
        trace.reset();
        int64_t cache_size = (is_MAE ? trace.get_MAE_bin_size(i, test_points) : trace.get_MAEQ_bin_size(i, test_points));
        if(i >0){
           int64_t last_cache_size = (is_MAE ? trace.get_MAE_bin_size(i-1, test_points) : trace.get_MAEQ_bin_size(i-1, test_points));
           if(cache_size == last_cache_size){
            BMRC_ratio[i] = BMRC_ratio[i-1];
            OMRC_ratio[i] = OMRC_ratio[i-1];
            i++;
            continue;
           }
        }
        LRUCacheFixCapacitySim<int64_t, uint32_t> cache(cache_size);
        int64_t hit_byte = 0;
        int64_t hit_req = 0;
        while (true)
        {
            pair<int64_t, int64_t> key_size = trace.get_next();
            if (key_size.first == -1 && key_size.second == -1)
            {
                break;
            }
            key = key_size.first;
            size = key_size.second;

            if(cache.count(key)){
                cache.get(key);
                hit_byte += size;
                hit_req += 1;
            }
            else{
                cache.put(key, size);
            }
        }
        BMRC_ratio[i] = 1.0-1.0*hit_byte/trace.total_access_size;
        OMRC_ratio[i] = 1.0-1.0*hit_req/trace.total_access_num;
        printf("%ld, %lf, %lf\n", is_MAE ? trace.get_MAE_bin_size(i, test_points) : trace.get_MAEQ_bin_size(i, test_points), BMRC_ratio[i], OMRC_ratio[i]);
    }

    
    out_file.open(output, ofstream::out | ofstream::trunc);
    out_file << "cache_size, BMRC_ratio, OMRC_ratio" << endl;
    printf("cache_size, BMRC_ratio, OMRC_ratio\n");
    for (int i = 0; i < test_points; i++)
    {
        out_file << (is_MAE ? trace.get_MAE_bin_size(i, test_points) : trace.get_MAEQ_bin_size(i, test_points)) << ", " << BMRC_ratio[i] << ", " << OMRC_ratio[i] << endl;
        printf("%ld, %lf, %lf\n", is_MAE ? trace.get_MAE_bin_size(i, test_points) : trace.get_MAEQ_bin_size(i, test_points), BMRC_ratio[i], OMRC_ratio[i]);
    }
}


int main(const int argc, char *argv[])
{
    cmdline::parser a;
    a.add<string>("trace", 't', "trace file name", true, "");
    a.add<string>("output", 'o', "output file name", true, "");
    a.add<string>("sample_method", 's', "FIX_RATE/FIX_NUM", false, "FIX_RATE");
    a.add<double>("sample_metric", 'r', "sample rate/ sample num", false, 0.01);
    a.add<string>("method", 'm', "REAL/SHARDS/CARRA/FLOWS", false, "REAL");
    a.add<string>("metric", 'c', "MAE/MAEQ", false, "MAE");
    a.add<uint64_t>("seed", 'd', "random seed", false, 42);
    a.add<string>("logpath", 'l', "logging file path leave blank if not necessary", false, "");
    a.add<string>("tracetype", 'y', "trace file type", false, "PLAIN");

    a.add<int64_t>("total_access_num", '\0', "total_access_num", false, 0);
    a.add<int64_t>("total_access_size", '\0', "total_access_size", false, 0);
    a.add<int64_t>("unique_access_num", '\0', "unique_access_num", false, 0);
    a.add<int64_t>("unique_access_size", '\0', "unique_access_size", false, 0);
    a.add<int64_t>("min_item_size", '\0', "min_item_size", false, 0);
    a.add<double>("slide_ratio", '\0', "slide_ratio", false, 0.5);


    a.parse_check(argc, argv);
    string tracefile = a.get<string>("trace");
    string output = a.get<string>("output");
    string sample_method = a.get<string>("sample_method");
    double sample_metric = a.get<double>("sample_metric");
    string method = a.get<string>("method");
    string metric = a.get<string>("metric");
    uint64_t seed = a.get<uint64_t>("seed");
    string logpath = a.get<string>("logpath");
    string trace_type = a.get<string>("tracetype");
    int64_t total_access_num = a.get<int64_t>("total_access_num");
    int64_t total_access_size = a.get<int64_t>("total_access_size");
    int64_t unique_access_num = a.get<int64_t>("unique_access_num");
    int64_t unique_access_size = a.get<int64_t>("unique_access_size");
    int64_t min_item_size = a.get<int64_t>("min_item_size");
    double slide_ratio = a.get<double>("slide_ratio");

    g_hash_mask = hash_uint64(seed, 0);

    if (logpath != "")
    {
        freopen(logpath.c_str(), "w", stdout);
    }

    
    printf("mrc dump to %s\n", output.c_str());

    printf("tracefile: %s\n", tracefile.c_str());
    Trace trace(tracefile, trace_type, total_access_num, total_access_size, unique_access_num, unique_access_size, min_item_size);

    auto time_start = chrono::steady_clock::now();
    cout << "[method]: " << method << endl;
    cout << "[metric]: " << metric << endl;
    cout << "[seed]: " << seed << endl;
    cout << "[sample_method]: " << sample_method << endl;
    cout << "[sample_metric]: " << sample_metric << endl;
    if (method == "REAL")
    {
        trace.reset();
        real_mrc(trace, output, metric);
    }
    else if (method == "SHARDS")
    {
        if (sample_method == "FIX_RATE")
        {
            shards_adj_mrc_fix_rate(trace, output, metric, sample_metric);
        }
        else
        {
            shards_adj_mrc_fix_cnt(trace, output, metric, sample_metric);
        }
    }
    else if (method == "CARRA")
    {
        if (sample_method == "FIX_RATE")
        {
            carra_mrc_fix_rate(trace, output, metric, sample_metric);
        }
        else
        {
            carra_mrc_fix_cnt(trace, output, metric, sample_metric);
        }
    }
    else if (method == "FLOWS")
    {
        if (sample_method == "FIX_RATE")
        {
            flows_mrc_fix_rate(trace, output, metric, sample_metric, TEST_POINT_NUM, slide_ratio);
        }
        else
        {
            flows_mrc_fix_cnt(trace, output, metric, sample_metric, TEST_POINT_NUM, slide_ratio);
        }
    }
    else
    {
        assert(0);
    }

    auto time_end = chrono::steady_clock::now();
    auto time_used = chrono::duration_cast<chrono::milliseconds>(time_end - time_start);
    cout << "Time used: " << time_used.count() << " ms" << endl;

    

    fclose(stdout);
    out_file.close();
    return 0;
}