#include "libCacheSim.h"
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <fstream>
#include "cmdline.h"
#include <iostream>
#include <map>
#include <memory>
#include <ostream>
#include <vector>
#include <utility>
#include <unordered_map>
#include <list>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <numeric>
#include <cmath>

using namespace std;

template <typename KeyType, typename ValueType>
class LRUCache
{
private:
    int capacity;
    list<pair<KeyType, ValueType>> cacheList;
    unordered_map<KeyType, typename list<pair<KeyType, ValueType>>::iterator> cacheMap;

    bool have_poped_key;
    KeyType poped_key;
    ValueType poped_value;

public:
    LRUCache(int capacity) : capacity(capacity)
    {
        poped_key = KeyType();
        have_poped_key = false;
    }

    bool count(const KeyType &key)
    {
        if (cacheMap.count(key))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    ValueType get(const KeyType &key)
    {
        if (cacheMap.find(key) == cacheMap.end())
        {
            return ValueType(); // If the key does not exist, return default value
        }

        // Move the key to the front of the list
        auto it = cacheMap[key];
        cacheList.splice(cacheList.begin(), cacheList, it);

        return it->second;
    }

    void put(const KeyType &key, const ValueType &value)
    {
        // Key already exists in the map
        if (cacheMap.find(key) != cacheMap.end())
        {
            auto it = cacheMap[key];
            it->second = value;
            cacheList.splice(cacheList.begin(), cacheList, it);
        }
        else
        {
            if (cacheMap.size() == capacity)
            {
                // The cache is full, so delete the least recently used keys
                auto last = prev(cacheList.end());

                KeyType lastKey = last->first;
                ValueType lastValue = last->second;

                poped_key = lastKey;
                poped_value = lastValue;
                have_poped_key = true;

                last->first = key;
                last->second = value;

                cacheList.splice(cacheList.begin(), cacheList, last);
                cacheMap.erase(lastKey);
                cacheMap[key] = cacheList.begin();
            }
            else
            {
                // insert new key
                cacheList.push_front(make_pair(key, value));
                cacheMap[key] = cacheList.begin();
            }
        }
    }

    bool get_poped_kv(KeyType &key, ValueType &value)
    {
        if (have_poped_key)
        {
            have_poped_key = false;
            key = poped_key;
            value = poped_value;
            return true;
        }
        return false;
    }

    int64_t size()
    {
        return cacheList.size();
    }
};

template <typename K, typename V>
class LRU_fix_count
{
public:
    LRU_fix_count(V cnt) : count(cnt), nowCount(0),
                           timeStamp(0), nowSize(0), cache(cnt)
    {
        ;
    }

    int length()
    {
        return nowCount;
    }
    V getNowSize()
    {
        return nowSize;
    }

    bool find(K key)
    {
        if (cache.count(key))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    void set(K key, V size)
    {
        timeStamp++;
        update(key, size, timeStamp);
    }

    void update(K key, V size, int64_t timeStamp)
    {
        if (cache.count(key))
        {
            pair<int64_t, V> t_value = cache.get(key);
            int64_t last_timestamp = t_value.first;
            cache.put(key, {timeStamp, size});
        }
        else
        {
            nowCount += 1;
            nowSize += size;
            cache.put(key, {timeStamp, size});
        }
    }

    pair<K, V> pop()
    {
        K poped_key;
        pair<int64_t, V> t_value;
        if (cache.get_poped_kv(poped_key, t_value))
        {
            nowCount -= 1;
            int64_t last_timestamp = t_value.first;
            nowSize -= t_value.second;

            return {poped_key, t_value.second};
        }
        return {0, 0};
    }

    int getMaxCount()
    {
        return count;
    }

private:
    V count;               // Capacity
    int64_t timeStamp = 0; // Current time stamp
    V nowCount;            // Current number of cached objects
    V nowSize;             // Current size of cached objects

    LRUCache<K, pair<int64_t, V>> cache;
};

struct HitRatioInfo
{
    double n_hit;
    double n_req;
    double size_hit;
    double size_req;
};
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

uint64_t g_hash_mask;

class WeightedSampler
{
    double sample_rate{0.01};
    double min_weighted_sample_rate{0.003};
    double avg_obj_size{0};
    int64_t total_req{0};
    int64_t total_req_size{0};
    HitRatioInfo hitinfo;
    LRU_fix_count<uint64_t, int64_t> cachefilter;
    int cachefilter_length;
    bool use_cache_filter;

public:
    WeightedSampler(double sample_rate, double avg_obj_size, double min_wsr_ratio = 0.5, bool use_cache_filter = false) : sample_rate(sample_rate), avg_obj_size(avg_obj_size),
                                                                                                                          cachefilter_length(max(1.0 * log10(1 / sample_rate) * (1 / sample_rate), 1.0)), cachefilter(max(1.0 * log10(1 / sample_rate) * (1 / sample_rate), 1.0)),
                                                                                                                          use_cache_filter(use_cache_filter)
    {
        hitinfo.n_hit = 0;
        hitinfo.n_req = 0;
        hitinfo.size_hit = 0;
        hitinfo.size_req = 0;
        g_hash_mask = hash_uint64(1, 0);
        min_weighted_sample_rate = min_wsr_ratio * sample_rate;
    }

    double get_hit_prob(cache_t *cache)
    {
        return 1.0 * cache->cache_size / (1e-5 + cachefilter.getNowSize());
    }

    bool spatial_sample(cache_t *cache, request_t *req, double &weighted_size, double &weighted_req, bool &hit_in_CF)
    {
        hit_in_CF = false;
        if (sample_rate == 1.0)
        {
            weighted_size = req->obj_size;
            weighted_req = 1.0;
            return true;
        }

        uint64_t _hashValue = hash_uint64(req->obj_id, g_hash_mask);
        uint64_t spatial_sample_max = sample_rate * UINT64_MAX;

        if (_hashValue <= spatial_sample_max)
        {
            weighted_size = req->obj_size / sample_rate;
            weighted_req = 1.0 / sample_rate;
            return true;
        }

        return false;
    }

    bool weighted_sample(cache_t *cache, request_t *req, double &weighted_size, double &weighted_req, bool &hit_in_CF)
    {
        hit_in_CF = false;
        if (sample_rate == 1.0)
        {
            weighted_size = req->obj_size;
            weighted_req = 1.0;
            return true;
        }
        if (use_cache_filter)
        {
            if (cachefilter.find(req->obj_id))
            {
                cachefilter.set(req->obj_id, req->obj_size);
                weighted_size = req->obj_size;
                weighted_req = 1.0;
                // if(req->obj_size < cache->cache_size){
                //     hit_in_CF = true;
                // }
                hit_in_CF = true;
                return true;
            }
            else
            {
                cachefilter.set(req->obj_id, req->obj_size);
                if (cachefilter.length() > cachefilter.getMaxCount())
                {
                    auto pop_kv = cachefilter.pop();
                    // uint64_t pop_hashValue = hash_uint64(pop_kv.first, g_hash_mask);
                    // double pop_weighted_sample_rate = min(1.0, sample_rate * pop_kv.second / avg_obj_size);
                    // pop_weighted_sample_rate = max(min_weighted_sample_rate, pop_weighted_sample_rate);
                    // uint64_t pop_weighte_sample_max = pop_weighted_sample_rate * UINT64_MAX;
                    // if(pop_weighted_sample_rate == 1.0) pop_weighte_sample_max = UINT64_MAX;
                    // if(pop_hashValue > pop_weighte_sample_max){
                    //     cache->remove(cache, pop_kv.first);
                    // }
                }
                // while(cachefilter.getNowSize() > cache->cache_size){
                //     auto pop_kv = cachefilter.pop();
                // }
            }
        }

        uint64_t _hashValue = hash_uint64(req->obj_id, g_hash_mask);
        double this_weighted_sample_rate = min(1.0, sample_rate * req->obj_size / avg_obj_size);
        this_weighted_sample_rate = max(min_weighted_sample_rate, this_weighted_sample_rate);

        // double this_weighted_sample_rate = sample_rate;

        uint64_t weighte_sample_max = this_weighted_sample_rate * UINT64_MAX;
        if (this_weighted_sample_rate == 1.0)
            weighte_sample_max = UINT64_MAX;

        if (_hashValue <= weighte_sample_max)
        {
            weighted_size = req->obj_size / this_weighted_sample_rate;
            weighted_req = 1.0 / this_weighted_sample_rate;
            return true;
        }

        return false;
    }
};

uint64_t est_avg_obj_size(reader_t *reader, int prefix = 10000)
{
    request_t *req = new_request();
    uint64_t sum_size = 0;
    for (int i = 0; i < prefix; i++)
    {
        read_one_req(reader, req);
        // req->obj_size = 1024;
        sum_size += req->obj_size;
    }
    reset_reader(reader);
    free_request(req);
    return sum_size / prefix;
}

pair<double, double> one_test(reader_t *const reader, double sample_rate, string cache_algo, int64_t cache_size, bool use_weighted_sample = true)
{

    request_t *req = new_request();

    /* setup a cache */
    common_cache_params_t cc_params = {
        .cache_size = cache_size, .hashpower = 24, .consider_obj_metadata = false};
    cache_t *cache = NULL;
    if (cache_algo == "LRU")
    {
        cache = LRU_init(cc_params, NULL);
    }
    else if (cache_algo == "LIRS")
    {
        cache = LIRS_init(cc_params, NULL);
    }
    else if (cache_algo == "TwoQ")
    {
        cache = TwoQ_init(cc_params, NULL);
    }
    else if (cache_algo == "LFU")
    {
        cache = LFU_init(cc_params, NULL);
    }
    else if (cache_algo == "FIFO")
    {
        cache = FIFO_init(cc_params, NULL);
    }
    else if (cache_algo == "ARC")
    {
        cache = ARC_init(cc_params, NULL);
    }
    else if (cache_algo == "Size")
    {
        cache = Size_init(cc_params, NULL);
    }
    else if (cache_algo == "LHD")
    {
        cache = LHD_init(cc_params, NULL);
    }
    else if (cache_algo == "Cacheus")
    {
        cache = Cacheus_init(cc_params, NULL);
    }
    else if (cache_algo == "S3FIFOd")
    {
        cache = S3FIFOd_init(cc_params, NULL);
    }
    else if (cache_algo == "S3FIFO")
    {
        cache = S3FIFOd_init(cc_params, NULL);
    }

    HitRatioInfo global_info = {.n_hit = 0, .n_req = 0, .size_hit = 0, .size_req = 0};
    HitRatioInfo sample_info = {.n_hit = 0, .n_req = 0, .size_hit = 0, .size_req = 0};

    uint64_t avg_obj_size = est_avg_obj_size(reader);
    WeightedSampler sampler(sample_rate, avg_obj_size, 0.5, false);
    double weighted_size;
    double weighted_req;
    int64_t req_cnt = 0;
    bool hit_in_cf;
    while (read_one_req(reader, req) == 0)
    {
        // req->obj_size = 1024;
        req_cnt += 1;
        global_info.n_req += 1;
        global_info.size_req += req->obj_size;
        if (((!use_weighted_sample) && sampler.spatial_sample(cache, req, weighted_size, weighted_req, hit_in_cf)) || (use_weighted_sample && sampler.weighted_sample(cache, req, weighted_size, weighted_req, hit_in_cf)))
        {
            sample_info.n_req += weighted_req;
            sample_info.size_req += weighted_size;

            req->obj_size = weighted_size;

            if (cache->get(cache, req) == TRUE)
            {
                sample_info.n_hit += weighted_req;
                sample_info.size_hit += weighted_size;
            }
        }
    }

    // 1603674944
    printf("========\n");
    printf("sample_info.size_hit %lf, sample_info.n_hit %lf\n", sample_info.size_hit, sample_info.n_hit);
    //   if(cache_algo != "LHD"){
    sample_info.size_hit += global_info.size_req - sample_info.size_req;
    sample_info.n_hit += global_info.n_req - sample_info.n_req;
    //   }
    double mbr = 1 - (double)sample_info.size_hit / global_info.size_req;
    double omr = 1 - (double)sample_info.n_hit / global_info.n_req;
    printf("cache size %ld\n", cache_size);
    printf("sampled byte %lf, req byte %lf\n", sample_info.size_req, global_info.size_req);
    printf("sampled req %lf, req req %lf\n", sample_info.n_req, global_info.n_req);
    printf("sample byte miss ratio: %lf\n", mbr);
    printf("sample obj miss ratio: %lf\n", omr);

    cache->cache_free(cache);
    free_request(req);

    return {mbr, omr};
}

string getFileNameFromPath(const string &filePath)
{
    size_t lastSlash = filePath.find_last_of('/');
    if (lastSlash != string::npos)
    {
        return filePath.substr(lastSlash + 1);
    }
    return filePath;
}

int main(int argc, char **argv)
{
    ofstream out_file;
    cmdline::parser a;
    a.add<string>("trace", 't', "trace file name", true, "");
    a.add<double>("sample_rate", 'r', "sample rate", true, 0.01);
    a.add<bool>("use_weighted_sample", 's', "true for weighted sample, false for spatial sample", false, true);
    a.add<string>("cache_algo", 'c', "cache algo", true, "LRU");

    a.parse_check(argc, argv);
    string tracefile = a.get<string>("trace");
    double sample_rate = a.get<double>("sample_rate");
    string cache_algo = a.get<string>("cache_algo");
    bool use_weighted_sample = a.get<bool>("use_weighted_sample");

    auto reader = setup_reader(tracefile.c_str(), ORACLE_GENERAL_TRACE_FLOWS, nullptr);
    vector<double> bmr_vec;
    vector<double> omr_vec;
    int test_points = 100;
    int now_cache_size = 1;
    int delta = 1;
    size_t delta_size = 1 * GiB;

    for (int i = 1; i <= test_points; i++)
    {
        reset_reader(reader);
        auto bmr_omr = one_test(reader, sample_rate, cache_algo, now_cache_size * delta_size, use_weighted_sample);
        bmr_vec.push_back(bmr_omr.first);
        omr_vec.push_back(bmr_omr.second);
        now_cache_size += delta;
    }

    stringstream ss;
    string method = "WS";
    if (sample_rate == 1.0)
    {
        method = "REAL";
    }
    else if (!use_weighted_sample)
    {
        method = "SS";
    }

    string fileName = getFileNameFromPath(tracefile);

    ss << "profile_res-" << fileName << "-" << method << "-" << cache_algo << "-" << to_string(sample_rate) << "-"
       << "bin"
       << "-"
       << "MAE"
       << "-" << 1 << ".csv";
    string output_file_name = ss.str();
    cout << output_file_name << endl;

    out_file.open(output_file_name, ofstream::out | ofstream::trunc);
    out_file << "cache_size, BMRC_ratio, OMRC_ratio" << endl;
    printf("cache_size, BMRC_ratio, OMRC_ratio\n");
    for (int i = 0; i < test_points; i++)
    {
        out_file << (i * delta + 1) * delta_size << ", " << bmr_vec[i] << ", " << omr_vec[i] << endl;
        printf("%ld, %lf, %lf\n", (i * 3 + 1) * delta_size, bmr_vec[i], omr_vec[i]);
    }
    close_reader(reader);
    out_file.close();

    return 0;
}
