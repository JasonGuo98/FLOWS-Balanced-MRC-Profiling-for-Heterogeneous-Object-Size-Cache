#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <cstring>

using namespace std;

struct MyStruct {
    uint32_t timestamp;
    uint64_t obj_id;
    uint32_t obj_size;
    int64_t next_access_vtime;
};

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
    printf("%s\n", buffer);
    uint64_t key, size, op_count, key_size;
    unordered_map<uint64_t, uint32_t> key_large_size;
    unordered_map<uint64_t, uint32_t> unique_size_map;
    struct MyStruct obj;
    obj.next_access_vtime = 0;
    int64_t i = 0;

    int64_t unique_size = 0;
    int64_t unique_count = 0;

    int64_t total_size = 0;
    int64_t total_count = 0;
    
    while(fscanf(in_file, "%d,%[^,],%d,%d,%d", &key, buffer, &size, &op_count, &key_size) == 5){
        i++;
        if(key_large_size.count(key) && size > key_large_size[key]){
            key_large_size[key] = size;
        }
        if(!key_large_size.count(key)){
            key_large_size[key] = size;
        }
        if (i % 1000000 == 0)
        {
            printf("key: %ld, size: %ld\n", key, size);
            printf("read %ld lines\n", i);
        }
    }


    i = 0;
    fseek(in_file, 0, SEEK_SET);
    fscanf(in_file, "%s", buffer);// read first line
    
    memset(&obj,0, sizeof(MyStruct));
    while(fscanf(in_file, "%d,%[^,],%d,%d,%d", &key, buffer, &size, &op_count, &key_size) == 5){
        auto max_size = key_large_size[key];
        max_size = std::max(max_size,1u);
        obj.next_access_vtime = 0;
        obj.timestamp = i;
        obj.obj_id = key;
        obj.obj_size = max_size;
        fwrite(&obj, sizeof(struct MyStruct), 1, out_file);

        total_size += max_size;
        total_count += 1;
        if(!unique_size_map.count(key)){
            unique_size_map[key] = 1;
            unique_count += 1;
            unique_size += max_size;
        }
        i++;
        if (i % 1000000 == 0)
        {
            printf("write %ld lines\n", i);
        }
    }

    std::cout << "Total Size: " << total_size << ", Total Count: " << total_count << std::endl;
    std::cout << "Unique Size: " << unique_size << ", Unique Count: " << unique_count << std::endl;

    fclose(in_file);
    fclose(out_file);
    return 0;
}
