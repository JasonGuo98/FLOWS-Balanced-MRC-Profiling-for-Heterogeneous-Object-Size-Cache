#ifndef INCLUDE_LRU_FIX_CAPACITY_H
#define INCLUDE_LRU_FIX_CAPACITY_H

#include "splaytree.h"
#include <map>
#include <unordered_map>

template <typename K, typename V>
class LRU_fix_capacity
{
public:
    LRU_fix_capacity(V cap) : capacity(cap), nowSize(0),
                              timeStamp(0)
    {
        ;
    }

    V getLastDistance()
    {
        return lastDistance;
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

        // key in the cache
        if (cache.count(key))
        {
            auto t_size = cache[key];
            int64_t lastSeenTime = t_size.first;
            t_size.first = timeStamp;
            // update reuse distance
            lastDistance = myTree.getDistance(lastSeenTime);

            myTree.erase(lastSeenTime);
            myTree.insert(timeStamp, size);

            cache[key] = t_size;

            timeKeyMap.erase(lastSeenTime);
            timeKeyMap[timeStamp] = key;
        }
        else
        {
            // key not in the cache
            nowSize += size;
            cache[key] = {timeStamp, size};
            timeKeyMap[timeStamp] = key;
            myTree.insert(timeStamp, size);

            lastDistance = INT64_MAX;

            if (nowSize > capacity)
            {
                pop();
            }
        }
    }

    void pop()
    {
        if (nowSize <= capacity)
            return;
        while (nowSize > capacity)
        {
            auto iter = myTree.begin();
            int64_t popTimeStampe = iter->first;

            K popKey = timeKeyMap[popTimeStampe];
            auto t_size = cache[popKey];
            V size = t_size.second;
            cache.erase(popKey);
            timeKeyMap.erase(popTimeStampe);
            myTree.erase(popTimeStampe);
            nowSize -= size;
        }
    }

private:
    V capacity;            // cache capacity
    int64_t timeStamp = 0; // current time stamp
    V lastDistance;        // last reuse distance
    V nowSize;             // current cache size 

    std::unordered_map<K, std::pair<int64_t, V>> cache;

    std::unordered_map<int64_t, K> timeKeyMap;

    SplayTree<int64_t, V> myTree;
};
#endif