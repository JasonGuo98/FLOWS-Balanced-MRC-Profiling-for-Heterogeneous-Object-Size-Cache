from figure_plot import *

if __name__ == "__main__":

    methods = ["SHARDS-FIX_NUM-32000", "CARRA-FIX_NUM-32000", "FLOWS-FIX_NUM-32000", ]
    methods += ["SHARDS-FIX_NUM-16000", "CARRA-FIX_NUM-16000", "FLOWS-FIX_NUM-16000"]
    methods += ["SHARDS-FIX_NUM-8000", "CARRA-FIX_NUM-8000", "FLOWS-FIX_NUM-8000"]
    method_cluster = [["SHARDS-FIX_NUM-32000", "CARRA-FIX_NUM-32000", "FLOWS-FIX_NUM-32000", ],
                      ["SHARDS-FIX_NUM-16000", "CARRA-FIX_NUM-16000", "FLOWS-FIX_NUM-16000"],
                      ["SHARDS-FIX_NUM-8000", "CARRA-FIX_NUM-8000", "FLOWS-FIX_NUM-8000"]]

    methods_list = []
    methods_list += [f"SHARDS-FIX_NUM-{sample_num}" for sample_num in [32000, 16000, 8000]]
    methods_list += [f"CARRA-FIX_NUM-{sample_num}" for sample_num in [32000, 16000, 8000]]
    methods_list += [f"FLOWS-FIX_NUM-{sample_num}" for sample_num in [32000, 16000, 8000]]

    bmrc_maeq, omrc_maeq = get_all_method_meaeq_dist(methods_list)
    cluster_name = ["32K", "16K", "8K"]
    labels = ['SHARDS', "Carra's", "FLOWS"]

    save_name = "plots/fig14.pdf"
    x_label_name = "SampleCount"
    figure13_14_plot(methods, method_cluster, bmrc_maeq, omrc_maeq, cluster_name, labels, save_name, x_label_name)