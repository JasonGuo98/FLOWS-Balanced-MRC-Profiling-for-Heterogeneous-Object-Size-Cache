#ifndef INCLUDE_LRU_FIX_COUNT_H
#define INCLUDE_LRU_FIX_COUNT_H

#include "splaytree.h"
#include "avl_array.h"
#include "robin_hood.h"
#include <map>
#include <utility>
#include <unordered_map>
#include <list>

using namespace std;

#define likely(x) __builtin_expect(!!(x), 1)

template <typename KeyType, typename ValueType>
class LRUCache
{
private:
    int capacity;
    list<pair<KeyType, ValueType>> cacheList;
    robin_hood::unordered_map<KeyType, typename list<pair<KeyType, ValueType>>::iterator> cacheMap;

    bool have_poped_key;
    KeyType poped_key;
    ValueType poped_value;

    typename list<pair<KeyType, ValueType>>::iterator buffer_;

public:
    LRUCache(int capacity) : capacity(capacity)
    {
        poped_key = 0;
        have_poped_key = false;
        cacheMap.reserve(4 * capacity);
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
        auto it = cacheMap[key];
        cacheList.splice(cacheList.begin(), cacheList, it);

        return it->second;
    }

    void update(const KeyType &key, const ValueType &value)
    {
        auto it = cacheMap[key];
        it->second = value;
    }

    void put(const KeyType &key, const ValueType &value)
    {
        {
            if (likely(cacheMap.size() == capacity))
            {
                // if cache is full, delete the least recently used key
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
                // insert new key-value pair
                cacheList.push_front(std::make_pair(key, value));
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

    V getLastDistance()
    {
        return lastDistance;
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
        nowCount += 1;
        nowSize += size;
        cache.put(key, {timeStamp, size});
        myTree.append(timeStamp, size);

        lastDistance = INT64_MAX;
    }

    void update(K key, V size)
    {
        timeStamp++;
        {
            pair<int64_t, V> t_value = cache.get(key);
            int64_t last_timestamp = t_value.first;
            cache.update(key, {timeStamp, size});

            lastDistance = myTree.getDistanceAndRemovel(last_timestamp, size);
            myTree.append(timeStamp, size);
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

            myTree.remove(last_timestamp);

            nowSize -= t_value.second;

            return {poped_key, t_value.second};
        }
        return {0, 0};
    }

private:
    V count;               // max number of cached objects
    int64_t timeStamp = 0; // current time stamp
    V lastDistance;        // last reuse distance
    V nowCount;            // current number of cached objects
    V nowSize;             // current cache size

    LRUCache<K, pair<int64_t, V>> cache;

    AVL<int64_t, V> myTree;
};

#endif