# FLOWS: Balanced MRC Profiling for Heterogeneous Object-Size Cache

This repository contains source code and scripts that help reproduce our EuroSys 2024 paper titled "FLOWS: Balanced MRC Profiling for Heterogeneous Object-Size Cache." Our work aims to provide efficient cache profiling for both Byte Miss Ratio Curve (BMRC) and Object Miss Ratio Curve (OMRC) construction for heterogeneous object-size caches, enabling efficient cache management.

## Overview

Our work is implemented in [src/flows.cc](src/flows.cc). It can be used to process specific trace file formats and BMRC and OMRC construction using [SHARDS](https://www.usenix.org/conference/fast15/technical-sessions/presentation/waldspurger), [HCPP](https://www.usenix.org/conference/atc20/presentation/carra) (CARRA's method), and our FLOWS methods. The specific parameters are as follows:

```bash
usage: ./flows --trace=string --output=string [options] ... 
options:
  -t, --trace                 trace file name (string)
  -o, --output                output file name (string)
  -s, --sample_method         FIX_RATE/FIX_NUM (string [=FIX_RATE])
  -r, --sample_metric         sample rate/ sample num (double [=0.01])
  -m, --method                REAL/SHARDS/CARRA/FLOWS (string [=REAL])
  -c, --metric                MAE/MAEQ (string [=MAE])
  -d, --seed                  random seed (unsigned long [=42])
  -l, --logpath               logging file path leave blank if not necessary (string [=])
  -y, --tracetype             trace file type (string [=PLAIN])
      --total_access_num      total_access_num (long [=0])
      --total_access_size     total_access_size (long [=0])
      --unique_access_num     unique_access_num (long [=0])
      --unique_access_size    unique_access_size (long [=0])
      --min_item_size         min_item_size (long [=0])
      --slide_ratio           slide_ratio (double [=0.5])
  -?, --help                  print this message
```

Please refer to the comments in the [src/flows.cc](src/flows.cc) file for more detailed information on how to use these parameters and their respective values. For example, use the following command to profile trace `cluster45.oracleGeneral.sample10` with `FLOWS` method under `0.01` sample rate: 

```bash
./src/flows -t ./trace/twitter/uni_kv_size/cluster45.oracleGeneral.sample10 -o ./results/profile_res-cluster45.oracleGeneral.sample10-FLOWS-FIX_RATE-0.01-bin-MAEQ-1.csv --sample_method FIX_RATE --sample_metric 0.01 --method FLOWS --tracetype bin -c MAEQ -d 1 --total_access_num 22288116 --total_access_size 1143103486 --unique_access_num 6548645 --unique_access_size 284493919 --min_item_size 1
```



## File Description

   ```bash
   ./libCacheSim # Modified for our trace type
   ./fonts # Font file of Times New Roman
   ./features # Features needed by LPCA algorithm
   ./plots # Saved figures
   ./results # Evaluation csv files for MRCs
   ./sample_sim # Mini simulations tool
   ./src # Source code of FLOWS and our implementation of SHARDS and HCPP (Carra's method)
   ./trace # Downloaded trace files
   ./trained_model # Saved models LPCA
   ```

## Prepare Environment

- Ubuntu 20.04 LTS
- Python 3.8.10

   ```bash
   python -m venv figs
   source figs/bin/activate
   pip install -r requirements.txt
   ```
- Tools

   ```bash
   sh +x make_all_tools.sh
   ```

## Prepare Trace Files

   This process requires a maximum of 800GB of storage space for downloading compressed files, extracting files, and converting formats.
   
   1. Download files

      ```bash
      sh +x download_trace.sh
      ```
   
   2. Decompress trace files
      ```bash
      sh +x decompress.sh
      ```

   3. Format trace files and make sure valid object size

      ```bash
      sh +x format.sh
      ```



## Run Evaluations
   
   This process will last for several hours as all methods undergo 20 rounds of testing to obtain average performance. Additionally, the HCPP method takes longer to complete compared to the other methods.

   * Exact Method: 
   
      ```bash
      python profile_all.py --method REAL --sample_method FIX_RATE --sample_metric 1 --tracetype bin --metric MAEQ
      ```


   * SHARDS Method: 
   
      ```bash
      python profile_all.py --method SHARDS --sample_method FIX_RATE --sample_metric 0.01 --tracetype bin --metric MAEQ
      python profile_all.py --method SHARDS --sample_method FIX_RATE --sample_metric 0.001 --tracetype bin --metric MAEQ
      python profile_all.py --method SHARDS --sample_method FIX_RATE --sample_metric 0.0001 --tracetype bin --metric MAEQ

      python profile_all.py --method SHARDS --sample_method FIX_NUM --sample_metric 8000 --tracetype bin --metric MAEQ
      python profile_all.py --method SHARDS --sample_method FIX_NUM --sample_metric 16000 --tracetype bin --metric MAEQ
      python profile_all.py --method SHARDS --sample_method FIX_NUM --sample_metric 32000 --tracetype bin --metric MAEQ
      ```

   * Evaluate Carra's Method: 
   
      ```bash
      python profile_all.py --method CARRA --sample_method FIX_RATE --sample_metric 0.01 --tracetype bin --metric MAEQ
      python profile_all.py --method CARRA --sample_method FIX_RATE --sample_metric 0.001 --tracetype bin --metric MAEQ
      python profile_all.py --method CARRA --sample_method FIX_RATE --sample_metric 0.0001 --tracetype bin --metric MAEQ

      python profile_all.py --method CARRA --sample_method FIX_NUM --sample_metric 8000 --tracetype bin --metric MAEQ
      python profile_all.py --method CARRA --sample_method FIX_NUM --sample_metric 16000 --tracetype bin --metric MAEQ
      python profile_all.py --method CARRA --sample_method FIX_NUM --sample_metric 32000 --tracetype bin --metric MAEQ
      ```

   * Evaluate LPCA Method: 
   
      ```bash
      mkdir trained_model
      
      python get_all_feature.py

      python train_model.py
      ```

   * Evaluate FLOWS Method: 
   
      ```bash
      python profile_all.py --method FLOWS --sample_method FIX_RATE --sample_metric 0.01 --tracetype bin --metric MAEQ
      python profile_all.py --method FLOWS --sample_method FIX_RATE --sample_metric 0.001 --tracetype bin --metric MAEQ
      python profile_all.py --method FLOWS --sample_method FIX_RATE --sample_metric 0.0001 --tracetype bin --metric MAEQ

      python profile_all.py --method FLOWS --sample_method FIX_NUM --sample_metric 8000 --tracetype bin --metric MAEQ
      python profile_all.py --method FLOWS --sample_method FIX_NUM --sample_metric 16000 --tracetype bin --metric MAEQ
      python profile_all.py --method FLOWS --sample_method FIX_NUM --sample_metric 32000 --tracetype bin --metric MAEQ

      # slide ratio between weighted sampling and spatial samiling
      python profile_slide.py
      ```

   * Minimal Simulation Evaluation:

      ```bash
      cd sample_sim/build

      # Weighted Sampling Method
      ./wsSim -t ../../trace/wiki/uni_kv_size/wiki_2019u.oracleGeneral -r 0.01 -c S3FIFOd
      ./wsSim -t ../../trace/wiki/uni_kv_size/wiki_2019u.oracleGeneral -r 0.01 -c Cacheus
      ./wsSim -t ../../trace/wiki/uni_kv_size/wiki_2019u.oracleGeneral -r 0.01 -c ARC
      ./wsSim -t ../../trace/wiki/uni_kv_size/wiki_2019u.oracleGeneral -r 0.01 -c TwoQ

      # Exact Method
      ./wsSim -t ../../trace/wiki/uni_kv_size/wiki_2019u.oracleGeneral -r 1 -c S3FIFOd
      ./wsSim -t ../../trace/wiki/uni_kv_size/wiki_2019u.oracleGeneral -r 1 -c Cacheus
      ./wsSim -t ../../trace/wiki/uni_kv_size/wiki_2019u.oracleGeneral -r 1 -c ARC
      ./wsSim -t ../../trace/wiki/uni_kv_size/wiki_2019u.oracleGeneral -r 1 -c TwoQ

      cp *.csv ../../results
      ```
      

## Plot Figures

   ```bash
   python fig1a.py
   python fig1b.py
   python fig4.py
   python fig10.py
   python fig11.py
   python fig12.py
   python fig13.py
   python fig14.py
   python fig15.py
   python fig17.py
   ```

   Figures are saved in `plots` folder.
