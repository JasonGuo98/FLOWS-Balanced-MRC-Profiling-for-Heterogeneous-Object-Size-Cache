from figure_plot import *

if __name__ == "__main__":
    bmrc_maeq, omrc_maeq = get_all_maeq_mean()
    methods_list = ["DL-NONE-1"]
    methods_list += [f"SHARDS-FIX_RATE-{sample_rate}" for sample_rate in [0.01]]
    methods_list += [f"CARRA-FIX_RATE-{sample_rate}" for sample_rate in [0.01]]
    methods_list += [f"FLOWS-FIX_RATE-{sample_rate}" for sample_rate in [0.01]]
    save_name = "plots/fig12.pdf"
    figure12_plot(methods_list, bmrc_maeq, omrc_maeq, save_name)
