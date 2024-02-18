import numpy as np
import pickle
import os
from figure_plot import *

def get_info(histogram, item_size):
    """
    Calculate size distribution and request distribution based on a histogram and item sizes.

    Args:
        histogram (dict): A dictionary representing the histogram of requests.
            The keys are the items and the values are the corresponding request counts.
        item_size (dict): A dictionary representing the sizes of the items.
            The keys are the items and the values are the corresponding sizes.

    Returns:
        tuple: A tuple containing two lists:
            - size_dist: A list of size distributions for different thresholds.
            - req_dist: A list of request distributions for different thresholds.
    """
    hist_list = []
    for k in histogram.keys():
        hist_list.append(histogram[k])
    hist_list.sort(reverse=True)

    bar_size = int(len(hist_list))
    i = 0
    size_th = []
    for i in [0.0001,0.001, 0.01, 0.1, 1.0]:
        size_th.append(hist_list[0 : int(bar_size*(i))][-1])

    size_dist = [[] for i in range(5)]
    req_dist = np.zeros(5)
    all_req = 0
    for k in histogram.keys():
        all_req += histogram[k]
        for i in range(len(size_th)):
            if histogram[k] >= size_th[i]:
                size_dist[i].append(item_size[k])
                req_dist[i] += histogram[k]

    for i in range(len(size_th)):
        size_dist[i].sort()

        new_size_dist = []
        for j in range(100):
            new_size_dist.append(size_dist[i][int(j*len(size_dist[i])/100)])
        new_size_dist.append(size_dist[i][-1])

        size_dist[i] = new_size_dist
        req_dist[i]/=all_req

    return size_dist, req_dist

if __name__ == "__main__":
    # meta_CDN
    if not os.path.exists("results/cdn_data.pkl"):
        histogram, item_size = read_trace("./trace/meta/bin_trace/reag0c01_20230315_20230322_0.2000.bin")
        with open('results/cdn_data.pkl', 'wb') as file:
            pickle.dump((histogram, item_size), file)
    with open('results/cdn_data.pkl', 'rb') as file:
        histogram, item_size = pickle.load(file)

    cdn_size_dist, cdn_req_dist = get_info(histogram, item_size)

    ## wiki_2019
    if not os.path.exists("results/wiki_data.pkl"):
        histogram, item_size = read_trace("./trace/wiki/uni_kv_size/wiki_2019u.oracleGeneral")
        with open('results/wiki_data.pkl', 'wb') as file:
            pickle.dump((histogram, item_size), file)
    with open('results/wiki_data.pkl', 'rb') as file:
        histogram, item_size = pickle.load(file)

    wiki2019u_size_dist, wiki2019u_req_dist = get_info(histogram, item_size)

    names = ["MetaCDN_reag", "wiki2019u"]
    save_name = "plots/fig4.pdf"
    figure4_plot((cdn_size_dist, wiki2019u_size_dist), (cdn_req_dist, wiki2019u_req_dist), names, save_name)