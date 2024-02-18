import os
from trace_info import trace_info_list
import subprocess
from multiprocessing import Process, Queue
import argparse

output_dir = "./results"
# output_dir = "./run_time_test"
flows_path = "./src/flows"

run_multi_num = 20

NUM_PROCESS = 4
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

def run_exp(method, sample_method, sample_metric, seed, tracetype, metric):
    for trace_info_dict in trace_info_list:
        trace_path = trace_info_dict["path"]
        total_access_num = trace_info_dict["total_access_num"]
        total_access_size = trace_info_dict["total_access_size"]
        unique_access_num = trace_info_dict["unique_access_num"]
        unique_access_size = trace_info_dict["unique_access_size"]
        min_item_size = trace_info_dict["min_item_size"]

        file_name = os.path.basename(trace_path)

        output_file = f"profile_res-{file_name}-{method}-{sample_method}-{sample_metric}-{tracetype}-{metric}-{seed}.csv"
        log_file = f"profile_res-{file_name}-{method}-{sample_method}-{sample_metric}-{tracetype}-{metric}-{seed}.log"

        output_path = os.path.join(output_dir, output_file)
        log_path = os.path.join(output_dir, log_file)

        

        command = f"unbuffer {flows_path} -t {trace_path} -o {output_path} --sample_method {sample_method} --sample_metric {sample_metric} --method {method} --tracetype {tracetype} -c {metric} -d {seed} --total_access_num {total_access_num} --total_access_size {total_access_size} --unique_access_num {unique_access_num} --unique_access_size {unique_access_size} --min_item_size {min_item_size} | tee  {log_path}"

        print("=====")
        print(command)
        if os.path.exists(output_path):
            print("exp done")
            continue
        add_command(command_queue, command)

    

if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument('--method', type=str, help='Method description')
    parser.add_argument('--sample_method', type=str, help='Sample method description')
    parser.add_argument('--sample_metric', type=str, help='Sample metric description')
    parser.add_argument('--seed', type=int, help='Seed description')
    parser.add_argument('--tracetype', type=str, help='Trace type description')
    parser.add_argument('--metric', type=str, help='Metric description')

    args = parser.parse_args()

    method = args.method
    sample_method = args.sample_method
    sample_metric = args.sample_metric
    seed = args.seed
    tracetype = args.tracetype
    metric = args.metric

    print("=====")
    print(f"method: {method}")
    print(f"sample_method: {sample_method}")
    print(f"sample_metric: {sample_metric}")
    print(f"tracetype: {tracetype}")
    print(f"metric: {metric}")

    

    processes = [Process(target=run_command, args=(command_queue,)) for _ in range(NUM_PROCESS)]
    for process in processes:
        process.start()


    if(method == "REAL"):
        sample_method = "FIX_RATE"
        sample_metric = "1"
        seed = 0
        run_exp(method, sample_method, sample_metric, seed, tracetype, metric)
    elif(sample_method == "FIX_NUM"):
        seed = 1
        run_exp(method, sample_method, sample_metric, seed, tracetype, metric)
    else:
        for i in range(run_multi_num):
            seed = i+1
            run_exp(method, sample_method, sample_metric, seed, tracetype, metric)


    for _ in range(NUM_PROCESS):
        command_queue.put(None)

    for process in processes:
        process.join()