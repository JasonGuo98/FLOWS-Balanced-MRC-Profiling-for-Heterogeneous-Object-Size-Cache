import os
from trace_info import trace_info_list
import subprocess
import argparse
import matplotlib.pyplot as plt
import csv
import pandas as pd
import matplotlib.pyplot

output_dir = "./features"
flows_path = "./src/get_feature"


if not os.path.exists(output_dir):
    os.mkdir(output_dir)

    



def get_feature(tracetype, metric):
    for trace_info_dict in trace_info_list[:]:
        trace_path = trace_info_dict["path"]
        total_access_num = trace_info_dict["total_access_num"]
        total_access_size = trace_info_dict["total_access_size"]
        unique_access_num = trace_info_dict["unique_access_num"]
        unique_access_size = trace_info_dict["unique_access_size"]
        min_item_size = trace_info_dict["min_item_size"]

        file_name = os.path.basename(trace_path)

        output_file = f"profile_res-{file_name}-{tracetype}-{metric}.txt"
        output_path = os.path.join(output_dir, output_file)

        if os.path.exists(output_path):
            # read file and plot
            continue
        command = f"{flows_path} -t {trace_path} -o {output_path} --tracetype {tracetype} -c {metric} --total_access_num {total_access_num} --total_access_size {total_access_size} --unique_access_num {unique_access_num} --unique_access_size {unique_access_size} --min_item_size {min_item_size}"
        print(command)
        subprocess.run(command, shell=True)
        


    

if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument('--tracetype', type=str, default="bin", help='Trace type description')
    parser.add_argument('--metric', type=str, default="MAEQ", help='Metric description')

    args = parser.parse_args()

    tracetype = args.tracetype
    metric = args.metric

    print("=====")
    print(f"tracetype: {tracetype}")
    print(f"metric: {metric}")

    get_feature(tracetype, metric)