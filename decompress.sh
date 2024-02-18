#!/bin/bash

mkdir -p ./trace/qqphoto/decompress/
mkdir -p ./trace/twitter/decompress/
mkdir -p ./trace/wiki/decompress/

unzstd -d ./trace/qqphoto/compressed/tencent_photo1.oracleGeneral.zst -o ./trace/qqphoto/decompress/tencent_photo1.oracleGeneral
unzstd -d ./trace/twitter/compressed/cluster17.oracleGeneral.sample10.zst -o ./trace/twitter/decompress/cluster17.oracleGeneral.sample10
unzstd -d ./trace/twitter/compressed/cluster18.oracleGeneral.sample10.zst -o ./trace/twitter/decompress/cluster18.oracleGeneral.sample10
unzstd -d ./trace/twitter/compressed/cluster24.oracleGeneral.sample10.zst -o ./trace/twitter/decompress/cluster24.oracleGeneral.sample10
unzstd -d ./trace/twitter/compressed/cluster29.oracleGeneral.sample10.zst -o ./trace/twitter/decompress/cluster29.oracleGeneral.sample10
unzstd -d ./trace/twitter/compressed/cluster44.oracleGeneral.sample10.zst -o ./trace/twitter/decompress/cluster44.oracleGeneral.sample10
unzstd -d ./trace/twitter/compressed/cluster45.oracleGeneral.sample10.zst -o ./trace/twitter/decompress/cluster45.oracleGeneral.sample10
unzstd -d ./trace/twitter/compressed/cluster52.oracleGeneral.sample10.zst -o ./trace/twitter/decompress/cluster52.oracleGeneral.sample10

unzstd -d ./trace/wiki/compressed/wiki_2019t.oracleGeneral.zst -o ./trace/wiki/decompress/wiki_2019t.oracleGeneral
unzstd -d ./trace/wiki/compressed/wiki_2019u.oracleGeneral.zst -o ./trace/wiki/decompress/wiki_2019u.oracleGeneral