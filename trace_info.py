import os

trace_info_list = [
    {
        "name": "Twitter52",
        "path":"./trace/twitter/uni_kv_size/cluster52.oracleGeneral.sample10",
        "total_access_num":1343968423,
        "total_access_size":202864439164,
        "unique_access_num":25183876,
        "unique_access_size":2463596005,
        "min_item_size":1
    },
    {
        "name": "Twitter17",
        "path":"./trace/twitter/uni_kv_size/cluster17.oracleGeneral.sample10",
        "total_access_num":883047506,
        "total_access_size":527057345401,
        "unique_access_num":677707,
        "unique_access_size":131857005,
        "min_item_size":1
    },
    {
        "name": "Twitter18",
        "path":"./trace/twitter/uni_kv_size/cluster18.oracleGeneral.sample10",
        "total_access_num":1292200645,
        "total_access_size":89337533634,
        "unique_access_num":1146662,
        "unique_access_size":29717916,
        "min_item_size":1
    },
    {
        "name": "Twitter24",
        "path":"./trace/twitter/uni_kv_size/cluster24.oracleGeneral.sample10",
        "total_access_num":328062779,
        "total_access_size":196376093693,
        "unique_access_num":1218190,
        "unique_access_size":251697474,
        "min_item_size":1
    },
    {
        "name": "Twitter44",
        "path":"./trace/twitter/uni_kv_size/cluster44.oracleGeneral.sample10",
        "total_access_num":548789525,
        "total_access_size":23069325949,
        "unique_access_num":5803081,
        "unique_access_size":212172434,
        "min_item_size":1
    },
    {
        "name": "Twitter45",
        "path":"./trace/twitter/uni_kv_size/cluster45.oracleGeneral.sample10",
        "total_access_num":22288116,
        "total_access_size":1143103486,
        "unique_access_num":6548645,
        "unique_access_size":284493919,
        "min_item_size":1
    },
    {
        "name": "Twitter29",
        "path":"./trace/twitter/uni_kv_size/cluster29.oracleGeneral.sample10",
        "total_access_num":698889960,
        "total_access_size":209647479626,
        "unique_access_num":25247062,
        "unique_access_size":6965324315,
        "min_item_size":1
    },
    {
        "name": "MetaKV-1",
        "path":"./trace/meta/bin_trace/kvcache_traces_1.bin",
        "total_access_num":207409712,
        "total_access_size":157892627411,
        "unique_access_num":18167668,
        "unique_access_size":16551913160,
        "min_item_size":1
    },
    {
        "name": "MetaKV-2",
        "path":"./trace/meta/bin_trace/kvcache_traces_2.bin",
        "total_access_num":204107043,
        "total_access_size":115702958385,
        "unique_access_num":17654174,
        "unique_access_size":12118431745,
        "min_item_size":1
    },
    {
        "name": "MetaKV-3",
        "path":"./trace/meta/bin_trace/kvcache_traces_3.bin",
        "total_access_num":198358817,
        "total_access_size":112255844111,
        "unique_access_num":17266634,
        "unique_access_size":12082289940,
        "min_item_size":1
    },
    {
        "name": "MetaKV-4",
        "path":"./trace/meta/bin_trace/kvcache_traces_4.bin",
        "total_access_num":200728414,
        "total_access_size":149801126471,
        "unique_access_num":17334592,
        "unique_access_size":16633431551,
        "min_item_size":1
    },
    {
        "name": "MetaKV-5",
        "path":"./trace/meta/bin_trace/kvcache_traces_5.bin",
        "total_access_num":203988033,
        "total_access_size":118186857311,
        "unique_access_num":17815285,
        "unique_access_size":12567786793,
        "min_item_size":1
    },
    {
        "name": "MetaCDN-reag",
        "path":"./trace/meta/bin_trace/reag0c01_20230315_20230322_0.2000.bin",
        "total_access_num":50114842,
        "total_access_size":408683243045023,
        "unique_access_num":14706047,
        "unique_access_size":10031617899825,
        "min_item_size":1
    },
    {
        "name":"MetaCDN-renh",
        "path":"./trace/meta/bin_trace/rnha0c01_20230315_20230322_0.8000.bin",
        "total_access_num":102880355,
        "total_access_size":890970047664277,
        "unique_access_num":36942643,
        "unique_access_size":20240758782588,
        "min_item_size":1
    },
    {
        "name":"MetaCDN-rprn",
        "path":"./trace/meta/bin_trace/rprn0c01_20230315_20230322_0.2000.bin",
        "total_access_num":96069551,
        "total_access_size":526763876781229,
        "unique_access_num":34573903,
        "unique_access_size":16484904156822,
        "min_item_size":1
    },
    {
        "name": "wiki2019t",
        "path":"./trace/wiki/uni_kv_size/wiki_2019t.oracleGeneral",
        "total_access_num":207646002,
        "total_access_size":6930104679609,
        "unique_access_num":18394481,
        "unique_access_size":429911251322,
        "min_item_size":1
    },
    {
        "name": "qqphoto",
        "path":"./trace/qqphoto/uni_kv_size/tencent_photo1.oracleGeneral",
        "total_access_num":100000000,
        "total_access_size":2539213752779,
        "unique_access_num":33318898,
        "unique_access_size":750000982724,
        "min_item_size":1
    },
    {
        "name": "wiki2019u",
        "path":"./trace/wiki/uni_kv_size/wiki_2019u.oracleGeneral",
        "total_access_num":100000000,
        "total_access_size":4170592743339,
        "unique_access_num":11054107,
        "unique_access_size":1117673717427,
        "min_item_size":1
    }
]

if __name__ == "__main__":
    for trace_info in trace_info_list:
        file = os.path.basename(trace_info["path"])
        avg_item_size = (trace_info["total_access_size"] / trace_info["total_access_num"])/1024
        traffic = trace_info["total_access_size"] / 1024 / 1024 / 1024/1024
        unique_access_num = trace_info["unique_access_num"]
        total_access_num = trace_info["total_access_num"]
        print(f"{file}: total_access_num={total_access_num}, unique_access_num={unique_access_num}, avg_item_size={avg_item_size}KB, traffic={traffic}TB")