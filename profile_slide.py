import os
from trace_info import trace_info_list
import subprocess
from multiprocessing import Process, Queue
import argparse

output_dir = "./results"
flows_path = "./src/flows"

NUM_PROCESS = 12
command_queue = Queue()

if not os.path.exists(output_dir):
    os.mkdir(output_dir)

def run_command(queue):
    while True:
        command = queue.get()
        if command is None:
            break
        subprocess.run(command, shell=True)


def add_command(queue, command):
    queue.put(command)

def run_exp(method, sample_method, sample_metric, seed, tracetype, metric, slide):
    for trace_info_dict in trace_info_list:
        trace_path = trace_info_dict["path"]
        total_access_num = trace_info_dict["total_access_num"]
        total_access_size = trace_info_dict["total_access_size"]
        unique_access_num = trace_info_dict["unique_access_num"]
        unique_access_size = trace_info_dict["unique_access_size"]
        min_item_size = trace_info_dict["min_item_size"]

        file_name = os.path.basename(trace_path)

        output_file = f"profile_res-{file_name}-{method}-{sample_method}-{sample_metric}-{tracetype}-{metric}-{seed}-slide{slide}.csv"

        output_path = os.path.join(output_dir, output_file)

        

        command = f"{flows_path} -t {trace_path} -o {output_path} --sample_method {sample_method} --sample_metric {sample_metric} --method {method} --tracetype {tracetype} -c {metric} -d {seed} --total_access_num {total_access_num} --total_access_size {total_access_size} --unique_access_num {unique_access_num} --unique_access_size {unique_access_size} --min_item_size {min_item_size} --slide_ratio {slide}"

        print("=====")
        print(command)
        if os.path.exists(output_path):
            print("exp done")
            continue
        add_command(command_queue, command)

    

if __name__ == "__main__":
    method = "FLOWS"
    sample_method = "FIX_RATE"
    sample_metric = 0.01
    tracetype = "bin"
    metric = "MAEQ"

    

    processes = [Process(target=run_command, args=(command_queue,)) for _ in range(NUM_PROCESS)]
    for process in processes:
        process.start()


    
    for slide in [0.1, 0.3, 0.5, 0.7, 0.9]:
        seed = 1
        run_exp(method, sample_method, sample_metric, seed, tracetype, metric, slide)


    for _ in range(NUM_PROCESS):
        command_queue.put(None)

    for process in processes:
        process.join()