from figure_plot import *

if __name__ == "__main__":

    methods = ["DL-NONE-1", "SHARDS-FIX_RATE-0.01", "CARRA-FIX_RATE-0.01", "FLOWS-FIX_RATE-0.01", ]
    methods += ["SHARDS-FIX_RATE-0.001", "CARRA-FIX_RATE-0.001", "FLOWS-FIX_RATE-0.001"]
    methods += ["SHARDS-FIX_RATE-0.0001", "CARRA-FIX_RATE-0.0001", "FLOWS-FIX_RATE-0.0001"]
    method_cluster = [["DL-NONE-1", "SHARDS-FIX_RATE-0.01", "CARRA-FIX_RATE-0.01", "FLOWS-FIX_RATE-0.01", ],
                      ["SHARDS-FIX_RATE-0.001", "CARRA-FIX_RATE-0.001", "FLOWS-FIX_RATE-0.001"],
                      ["SHARDS-FIX_RATE-0.0001", "CARRA-FIX_RATE-0.0001", "FLOWS-FIX_RATE-0.0001"]]

    methods_list = []
    methods_list += [f"SHARDS-FIX_RATE-{sample_rate}" for sample_rate in [0.01, 0.001, 0.0001]]
    methods_list += [f"CARRA-FIX_RATE-{sample_rate}" for sample_rate in [0.01, 0.001, 0.0001]]
    methods_list += [f"FLOWS-FIX_RATE-{sample_rate}" for sample_rate in [0.01, 0.001, 0.0001]]
    methods_list += ["DL-NONE-1"]

    bmrc_maeq, omrc_maeq = get_all_method_meaeq_dist(methods_list)
    cluster_name = ["1%", "0.1%", "0.01%"]
    labels = ['LPCA', 'SHARDS', "Carra's", "FLOWS"]
    
    save_name = "plots/fig13.pdf"
    x_label_name = "SampleRate*"
    figure13_14_plot(methods, method_cluster, bmrc_maeq, omrc_maeq, cluster_name, labels, save_name, x_label_name)