#!/bin/bash

mkdir -p ./trace/qqphoto/compressed/
mkdir -p ./trace/twitter/compressed/
mkdir -p ./trace/wiki/compressed/
mkdir -p ./trace/meta/

wget -P ./trace/qqphoto/compressed/  https://ftp.pdl.cmu.edu/pub/datasets/twemcacheWorkload/cacheDatasets/tencentPhoto/tencent_photo1.oracleGeneral.zst

wget -P ./trace/twitter/compressed/  https://ftp.pdl.cmu.edu/pub/datasets/twemcacheWorkload/cacheDatasets/twitter/sample/cluster17.oracleGeneral.sample10.zst

wget -P ./trace/twitter/compressed/  https://ftp.pdl.cmu.edu/pub/datasets/twemcacheWorkload/cacheDatasets/twitter/sample/cluster18.oracleGeneral.sample10.zst

wget -P ./trace/twitter/compressed/  https://ftp.pdl.cmu.edu/pub/datasets/twemcacheWorkload/cacheDatasets/twitter/sample/cluster24.oracleGeneral.sample10.zst

wget -P ./trace/twitter/compressed/  https://ftp.pdl.cmu.edu/pub/datasets/twemcacheWorkload/cacheDatasets/twitter/sample/cluster29.oracleGeneral.sample10.zst

wget -P ./trace/twitter/compressed/  https://ftp.pdl.cmu.edu/pub/datasets/twemcacheWorkload/cacheDatasets/twitter/sample/cluster44.oracleGeneral.sample10.zst

wget -P ./trace/twitter/compressed/  https://ftp.pdl.cmu.edu/pub/datasets/twemcacheWorkload/cacheDatasets/twitter/sample/cluster45.oracleGeneral.sample10.zst

wget -P ./trace/twitter/compressed/  https://ftp.pdl.cmu.edu/pub/datasets/twemcacheWorkload/cacheDatasets/twitter/sample/cluster52.oracleGeneral.sample10.zst

wget -P ./trace/wiki/compressed/  https://ftp.pdl.cmu.edu/pub/datasets/twemcacheWorkload/cacheDatasets/wiki/wiki_2019t.oracleGeneral.zst

wget -P ./trace/wiki/compressed/  https://ftp.pdl.cmu.edu/pub/datasets/twemcacheWorkload/cacheDatasets/wiki/wiki_2019u.oracleGeneral.zst

aws s3 cp --no-sign-request --recursive s3://cachelib-workload-sharing/pub/kvcache/202206/ ./trace/meta/

aws s3 cp --no-sign-request --recursive s3://cachelib-workload-sharing/pub/cdn/ ./trace/meta/