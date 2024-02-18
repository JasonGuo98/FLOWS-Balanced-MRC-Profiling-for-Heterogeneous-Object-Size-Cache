#!/bin/bash

cd src

make

g++ get_feature.cc -o get_feature -O2
g++ convert_metacdn2oracle.cc -o ./convert_metacdn2oracle -O2
g++ convert_metakv2oracle.cc -o ./convert_metakv2oracle -O2
g++ make_oracle_same_size.cc -o ./make_oracle_same_size -O2

cd ..