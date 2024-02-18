#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unordered_map>
#include <string.h>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>


using namespace std;

struct MyStruct {
    uint32_t timestamp;
    uint64_t obj_id;
    uint32_t obj_size;
    int64_t next_access_vtime;
};

uint64_t combineUint32(uint32_t a, uint32_t b) {
    uint64_t result = ((uint64_t)b << 32) | a;
    return result;
}

uint64_t strkey2intkey(const char* str, int k) {
    uint32_t a, b;

    // read k prefix number as uint32
    char temp[12];  // uint32_max needs 11 char
    strncpy(temp, str, k);
    temp[k] = '\0';
    sscanf(temp, "%u", &a);

    b = strlen(str + k);

    uint64_t combined = combineUint32(a, b);

    return combined;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: ./program <in_filename> <out_filename>\n");
        return 1;
    }

    const char* in_filename = argv[1];
    const char* out_filename = argv[2];
    FILE* in_file = fopen(in_filename, "r");
    if (in_file == NULL) {
        printf("Failed to open in_file.\n");
        return 1;
    }
    FILE* out_file = fopen(out_filename, "wb");
    if (out_file == NULL) {
        printf("Failed to open out_file.\n");
        return 1;
    }

    char buffer[1024];
    fscanf(in_file, "%s", buffer);// read first line
    int64_t OpType, objectSize, responseSize, responseHeaderSize, rangeStart, rangeEnd, TTL, cache_hit, item_value, RequestHandler, vip_type;
    uint64_t time_stamp, key;
    uint32_t size;
    double SamplingRate;
    char cacheKey[40960], cdn_content_type_id[1024];

    unordered_map<uint64_t, uint32_t> key_large_size;
    unordered_map<uint64_t, uint32_t> unique_size_map;
    struct MyStruct obj;
    obj.next_access_vtime = 0;
    int64_t i = 0;

    int64_t unique_size = 0;
    int64_t unique_count = 0;

    int64_t total_size = 0;
    int64_t total_count = 0;
    
    while(fscanf(in_file, "%lu,%[^,],%ld,%ld,%ld,%s", &time_stamp, cacheKey, &OpType, &objectSize, &responseSize, buffer) == 6){
        key = strkey2intkey(cacheKey, 11);
        size = max(objectSize, responseSize);

        if(size == -1){
            printf("find -1: %lu,%s,%ld,%ld,%ld,%s\n",time_stamp, cacheKey, OpType, objectSize, responseSize,buffer);
            continue;
        }
        

        if(key_large_size.count(key) && size > key_large_size[key]){
            key_large_size[key] = size;
        }
        if(!key_large_size.count(key)){
            key_large_size[key] = size;
        }
        i++;
        if (i % 1000000 == 0)
        {
            printf("read %ld lines\n", i);
        }
    }


    i = 0;
    fseek(in_file, 0, SEEK_SET);
    fscanf(in_file, "%s", buffer);// read first line

    memset(&obj,0, sizeof(MyStruct));
    while (fscanf(in_file, "%lu,%[^,],%ld,%ld,%s", &time_stamp, cacheKey, &OpType, &objectSize, buffer) == 5) {
        
        
        key = strkey2intkey(cacheKey, 11);
        obj.timestamp = uint32_t(time_stamp/1000);
        obj.obj_id = key;
        obj.obj_size = std::max(1u, key_large_size[key]);
        obj.next_access_vtime = 0;
        fwrite(&obj, sizeof(struct MyStruct), 1, out_file);

        total_size += key_large_size[key];
        total_count += 1;
        if(!unique_size_map.count(key)){
            unique_size_map[key] = 1;
            unique_count += 1;
            unique_size += key_large_size[key];
        }

        i++;
        if (i % 1000000 == 0)
        {
            printf("write %ld lines\n", i);
            // break;
        }
    }

    std::cout << "Total Size: " << total_size << ", Total Count: " << total_count << std::endl;
    std::cout << "Unique Size: " << unique_size << ", Unique Count: " << unique_count << std::endl;


    fclose(in_file);
    fclose(out_file);
    return 0;
}