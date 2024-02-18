import numpy as np
import pandas as pd
import struct
from trace_info import trace_info_list
import os
small_cache_size_B = 0

def read_trace(trace_path):
    """
    Reads a binary trace file and returns a histogram and item size dictionary.

    Args:
        trace_path (str): The path to the binary trace file.

    Returns:
        tuple: A tuple containing the histogram dictionary and item size dictionary.
            The histogram dictionary maps object IDs to their frequency of occurrence.
            The item size dictionary maps object IDs to their corresponding size.
    """
    struct_format = "IQLq"

    histogram = {}
    item_size = {}
    with open(trace_path, "rb") as file:
        file_content = file.read()

        struct_size = struct.calcsize(struct_format)
        print(struct_size)

        num_structs = len(file_content) // struct_size
        for i in range(num_structs):
            struct_data = struct.unpack_from(struct_format, file_content, i * struct_size)

            _, obj_id, obj_size, _ = struct_data

            if(i % 1000000 == 0):
                print("read %d", i)

            if obj_id in histogram:
                histogram[obj_id] += 1
            else:
                histogram[obj_id] = 1

            if obj_id not in item_size:
                item_size[obj_id] = obj_size
    return histogram, item_size

def load_csv_mrc_file(file_name):
    """
    Load data from a CSV file and extract cache size, BMRC ratio, and OMRC ratio and catch the first non-decreasing point as the small cache size.

    Parameters:
    file_name (str): The path to the result CSV file.

    Returns:
    tuple: A tuple containing cache size, BMRC ratio, and OMRC ratio as numpy arrays.
    """
    global small_cache_size_B
    mrc = pd.read_csv(file_name)

    cache_size = mrc['cache_size'].values
    bmrc = mrc[' BMRC_ratio'].values
    omrc = mrc[' OMRC_ratio'].values


    # initialize the small cache size to the first non-decreasing point
    for i in range(len(bmrc)-1):
        if small_cache_size_B == 0 and (bmrc[i+1] - bmrc[i]) > 0.2:
            small_cache_size_B = cache_size[i]

    return cache_size, bmrc, omrc

def get_MAEQ(accurate_mrc, estimate_mrc):
    new_accurate_mrc = []
    new_accurate_mrc.append(1)
    for mr in accurate_mrc:
        new_accurate_mrc.append(mr)
    accurate_mrc = np.array(new_accurate_mrc)

    new_estimate_mrc = []
    new_estimate_mrc.append(1)
    for mr in estimate_mrc:
        new_estimate_mrc.append(mr)
    estimate_mrc = np.array(new_estimate_mrc)

    l = 0
    r = 0
    q = 0.01
    last_point = 1.0

    sum_q_level = 0
    sum_q_err = 0
    for i in range(len(accurate_mrc)):
        if accurate_mrc[i] <= last_point-q:
            r = i
        elif (i == len(accurate_mrc) - 1):
            r = i+1
        else:
            continue
        q_level = int((last_point - accurate_mrc[r-1])/q + 1)
        q_err = np.mean(np.abs(accurate_mrc[l:r] - estimate_mrc[l:r])) * q_level

        sum_q_level += q_level
        sum_q_err += q_err

        last_point -= q_level*q
        l = r

    return sum_q_err/sum_q_level

def get_trace_maeq(real_bomrc, profile_bomrc):
    byte_maeq = get_MAEQ(real_bomrc[:, 0], profile_bomrc[:, 0])
    obj_maeq = get_MAEQ(real_bomrc[:, 1], profile_bomrc[:, 1])
    return byte_maeq, obj_maeq

    
def get_method_seed_maeq(method, trace_file, seed, extra_info=""):

    profile_file = f"./results/profile_res-{trace_file}-{method}-bin-MAEQ-{seed}{extra_info}.csv"
    real_file = f"./results/profile_res-{trace_file}-REAL-FIX_RATE-1-bin-MAEQ-0.csv"

    if(not os.path.exists(profile_file)):
        return 0, 0
    profile_mrc = pd.read_csv(profile_file)
    real_mrc = pd.read_csv(real_file)

    profile_bomrc = profile_mrc[[' BMRC_ratio', ' OMRC_ratio']].values
    real_bomrc = real_mrc[[' BMRC_ratio', ' OMRC_ratio']].values

    byte_maeq, obj_maeq = get_trace_maeq(real_bomrc, profile_bomrc)

    return byte_maeq, obj_maeq

def get_method_all_maeq(method, trace_file, extra_info = ""):
    byte_maeq = []
    obj_maeq = []
    for seed in range(1,21):
        _byte_maeq, _obj_maeq = get_method_seed_maeq(method, trace_file, seed, extra_info)
        if(_byte_maeq == 0 and _obj_maeq == 0):
            # print(f"{trace_file}-{method}-{seed}: no file")
            continue
        byte_maeq.append(_byte_maeq)
        obj_maeq.append(_obj_maeq)
    return byte_maeq, obj_maeq


def get_all_maeq_mean():
    """
    Calculate the mean of byte_maeq and obj_maeq for each trace file and method.

    Returns:
        bmrc_maeq (dict): A dictionary containing the mean byte_maeq for each trace file and method.
        omrc_maeq (dict): A dictionary containing the mean obj_maeq for each trace file and method.
    """
    bmrc_maeq = {}
    omrc_maeq = {}
    methods_list = []
    methods_list += [f"SHARDS-FIX_RATE-{sample_rate}" for sample_rate in [0.01, 0.001, 0.0001]]
    methods_list += [f"CARRA-FIX_RATE-{sample_rate}" for sample_rate in [0.01, 0.001, 0.0001]]
    methods_list += [f"FLOWS-FIX_RATE-{sample_rate}" for sample_rate in [0.01, 0.001, 0.0001]]
    methods_list += ["DL-NONE-1"]
    for method in methods_list:
        for info in trace_info_list:
            trace_path = info["path"]
            trace_file = os.path.basename(trace_path)

            byte_maeq, obj_maeq = get_method_all_maeq(method, trace_file)

            print(f"{trace_file}-{method}==> byte_maeq: {np.mean(byte_maeq)}, obj_maeq: {np.mean(obj_maeq)}")

            bmrc_maeq[f"{trace_file}-{method}"] = np.mean(byte_maeq)
            omrc_maeq[f"{trace_file}-{method}"] = np.mean(obj_maeq)

    return bmrc_maeq, omrc_maeq


def get_all_slide_maeq_mean():
    """
    Calculate the mean of byte_maeq and obj_maeq for different methods and extra_info values.

    Returns:
    - bmrc_maeq: A dictionary containing the mean byte_maeq values for different methods and extra_info values.
    - omrc_maeq: A dictionary containing the mean obj_maeq values for different methods and extra_info values.
    """

    bmrc_maeq = {}
    omrc_maeq = {}
    methods_list = ["CARRA-FIX_RATE-0.01"]
    methods_list += [f"FLOWS-FIX_RATE-0.01"]
    bmrc_maeq["CARRA"] = []
    omrc_maeq["CARRA"] = []
    extra_info_list = ["slide0.1", "slide0.3", "slide0.5", "slide0.7", "slide0.9"]
    for extra_info in extra_info_list:
        bmrc_maeq[extra_info] = []
        omrc_maeq[extra_info] = []

    for method in methods_list:
        for info in trace_info_list:
            if method == "CARRA-FIX_RATE-0.01":
                trace_path = info["path"]
                trace_file = os.path.basename(trace_path)

                byte_maeq, obj_maeq = get_method_all_maeq(method, trace_file)

                print(f"{trace_file}-{method}==> byte_maeq: {np.mean(byte_maeq)}, obj_maeq: {np.mean(obj_maeq)}")

                bmrc_maeq['CARRA'].append(np.mean(byte_maeq))
                omrc_maeq['CARRA'].append(np.mean(obj_maeq))
            else:
                for extra_info in extra_info_list:
                    trace_path = info["path"]
                    trace_file = os.path.basename(trace_path)

                    byte_maeq, obj_maeq = get_method_all_maeq(method, trace_file, "-"+extra_info)

                    print(f"{trace_file}-{method}==> byte_maeq: {np.mean(byte_maeq)}, obj_maeq: {np.mean(obj_maeq)}")

                    bmrc_maeq[extra_info].append(np.mean(byte_maeq))
                    omrc_maeq[extra_info].append(np.mean(obj_maeq))

    return bmrc_maeq, omrc_maeq

def get_all_method_meaeq_dist(methods_list):
    """
    Calculate the mean absolute error quotient (MAEQ) distribution for each method in the given list.

    Args:
        methods_list (list): A list of methods.

    Returns:
        tuple: A tuple containing two dictionaries: bmrc_maeq and omrc_maeq.
            - bmrc_maeq (dict): A dictionary where the keys are methods and the values are lists of byte MAEQ values.
            - omrc_maeq (dict): A dictionary where the keys are methods and the values are lists of object MAEQ values.
    """
    bmrc_maeq = {}
    omrc_maeq = {}
    for method in methods_list:
        bmrc_maeq[method] = []
        omrc_maeq[method] = []
        for info in trace_info_list:
            trace_path = info["path"]
            trace_file = os.path.basename(trace_path)

            byte_maeq, obj_maeq = get_method_all_maeq(method, trace_file, "")

            print(f"{trace_file}-{method}==> byte_maeq: {np.mean(byte_maeq)}, obj_maeq: {np.mean(obj_maeq)}")

            bmrc_maeq[method].extend(list(byte_maeq))
            omrc_maeq[method].extend(list(obj_maeq))

    return bmrc_maeq, omrc_maeq