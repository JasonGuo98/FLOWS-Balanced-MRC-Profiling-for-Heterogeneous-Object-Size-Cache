#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unordered_map>
#include <cstring>

using namespace std;

struct MyStruct
{
    uint32_t timestamp;
    uint64_t obj_id;
    uint32_t obj_size;
    int64_t next_access_vtime;
};

struct TwitterStruct
{
    uint32_t timestamp;
    uint64_t obj_id;
    uint32_t obj_size;
    int64_t next_access_vtime;
} __attribute__((packed));

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: ./program <in_filename> <out_filename> <cut_tail>\n");
        return 1;
    }

    const char *in_filename = argv[1];
    const char *out_filename = argv[2];
    bool cut_tail = false;
    cut_tail = atoi(argv[3]);

    FILE *in_file = fopen(in_filename, "r");
    if (in_file == NULL)
    {
        printf("Failed to open in_file.\n");
        return 1;
    }
    FILE *out_file = fopen(out_filename, "wb");
    if (out_file == NULL)
    {
        printf("Failed to open out_file.\n");
        return 1;
    }

    unordered_map<uint64_t, uint32_t> key_large_size;

    struct TwitterStruct obj;
    struct MyStruct write_obj;
    int64_t i = 0;
    while (fread(&obj, sizeof(struct TwitterStruct), 1, in_file) == 1)
    {
        if (key_large_size.count(obj.obj_id))
        {
            if (obj.obj_size > key_large_size[obj.obj_id])
            {
                key_large_size[obj.obj_id] = obj.obj_size;
            }
        }
        else
        {
            key_large_size[obj.obj_id] = obj.obj_size;
        }
        i++;
        if (i % 1000000 == 0)
        {
            printf("read %ld lines\n", i);
        }
        if (cut_tail && i == 100000000)
        {
            break;
        }
    }

    i = 0;
    memset(&write_obj, 0, sizeof(MyStruct));
    fseek(in_file, 0, SEEK_SET);
    while (fread(&obj, sizeof(struct TwitterStruct), 1, in_file) == 1)
    {
        write_obj.obj_id = obj.obj_id;
        write_obj.obj_size = max(1u, key_large_size[obj.obj_id]);
        write_obj.next_access_vtime = obj.next_access_vtime;
        write_obj.timestamp = obj.timestamp;

        fwrite(&write_obj, sizeof(struct MyStruct), 1, out_file);

        i++;
        if (i % 1000000 == 0)
        {
            printf("process %ld lines\n", i);
        }
        if (cut_tail && i == 100000000)
        {
            break;
        }
    }

    fclose(in_file);
    fclose(out_file);
    return 0;
}