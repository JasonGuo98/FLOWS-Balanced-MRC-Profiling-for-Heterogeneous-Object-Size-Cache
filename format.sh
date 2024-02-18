#!/bin/bash

cd src



mkdir -p ../trace/meta/bin_trace/

./convert_metakv2oracle ../trace/meta/kvcache_traces_1.csv ../trace/meta/bin_trace/kvcache_traces_1.bin
./convert_metakv2oracle ../trace/meta/kvcache_traces_2.csv ../trace/meta/bin_trace/kvcache_traces_2.bin
./convert_metakv2oracle ../trace/meta/kvcache_traces_3.csv ../trace/meta/bin_trace/kvcache_traces_3.bin
./convert_metakv2oracle ../trace/meta/kvcache_traces_4.csv ../trace/meta/bin_trace/kvcache_traces_4.bin
./convert_metakv2oracle ../trace/meta/kvcache_traces_5.csv ../trace/meta/bin_trace/kvcache_traces_5.bin

./convert_metacdn2oracle ../trace/meta/reag0c01_20230315_20230322_0.2000.csv ../trace/meta/bin_trace/reag0c01_20230315_20230322_0.2000.bin

./convert_metacdn2oracle ../trace/meta/rnha0c01_20230315_20230322_0.8000.csv ../trace/meta/bin_trace/rnha0c01_20230315_20230322_0.8000.bin

./convert_metacdn2oracle ../trace/meta/rprn0c01_20230315_20230322_0.2000.csv ../trace/meta/bin_trace/rprn0c01_20230315_20230322_0.2000.bin




mkdir -p ../trace/qqphoto/uni_kv_size/
mkdir -p ../trace/twitter/uni_kv_size/
mkdir -p ../trace/wiki/uni_kv_size/

./make_oracle_same_size ../trace/qqphoto/decompress/tencent_photo1.oracleGeneral ../trace/qqphoto/uni_kv_size/tencent_photo1.oracleGeneral 1

./make_oracle_same_size ../trace/twitter/decompress/cluster17.oracleGeneral.sample10 ../trace/twitter/uni_kv_size/cluster17.oracleGeneral.sample10 0

./make_oracle_same_size ../trace/twitter/decompress/cluster18.oracleGeneral.sample10 ../trace/twitter/uni_kv_size/cluster18.oracleGeneral.sample10 0

./make_oracle_same_size ../trace/twitter/decompress/cluster24.oracleGeneral.sample10 ../trace/twitter/uni_kv_size/cluster24.oracleGeneral.sample10 0

./make_oracle_same_size ../trace/twitter/decompress/cluster29.oracleGeneral.sample10 ../trace/twitter/uni_kv_size/cluster29.oracleGeneral.sample10 0

./make_oracle_same_size ../trace/twitter/decompress/cluster44.oracleGeneral.sample10 ../trace/twitter/uni_kv_size/cluster44.oracleGeneral.sample10 0

./make_oracle_same_size ../trace/twitter/decompress/cluster45.oracleGeneral.sample10 ../trace/twitter/uni_kv_size/cluster45.oracleGeneral.sample10 0

./make_oracle_same_size ../trace/twitter/decompress/cluster52.oracleGeneral.sample10 ../trace/twitter/uni_kv_size/cluster52.oracleGeneral.sample10 0

./make_oracle_same_size ../trace/wiki/decompress/wiki_2019t.oracleGeneral ../trace/wiki/uni_kv_size/wiki_2019t.oracleGeneral 0

./make_oracle_same_size ../trace/wiki/decompress/wiki_2019u.oracleGeneral ../trace/wiki/uni_kv_size/wiki_2019u.oracleGeneral 1

cd ..