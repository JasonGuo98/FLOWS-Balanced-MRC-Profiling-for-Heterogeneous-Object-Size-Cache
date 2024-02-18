from figure_plot import *

if __name__ == "__main__":
    cache_sizes = []
    exact_bomrc = []
    profile_bomrc = []

    for i in range(len(trace_info_list)):
        trace_info = trace_info_list[i]
        trace_name = os.path.basename(trace_info["path"])
        exact_mrc_file = f"./results/profile_res-{trace_name}-REAL-FIX_RATE-1-bin-MAEQ-0.csv"
        flows_mrc_file = f"./results/profile_res-{trace_name}-FLOWS-FIX_RATE-0.01-bin-MAEQ-2.csv"

        cache_size, exact_bmrc, exact_omrc = load_csv_mrc_file(exact_mrc_file)
        _, flows_bmrc, flows_omrc = load_csv_mrc_file(flows_mrc_file)

        cache_sizes.append(cache_size)
        exact_bomrc.append(np.vstack((exact_bmrc, exact_omrc)))
        profile_bomrc.append(np.vstack((flows_bmrc, flows_omrc)))
    
    save_name = "plots/fig11.pdf"
    figure11_plot(cache_sizes, exact_bomrc, profile_bomrc, save_name)
