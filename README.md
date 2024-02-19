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



## Run All Evaluations
   

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
      

3. Draw Figures
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