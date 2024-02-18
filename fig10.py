from figure_plot import *

if __name__ == "__main__":

    trace_name = "reag0c01_20230315_20230322_0.2000.bin"

    real_file_name = f"./results/profile_res-{trace_name}-REAL-FIX_RATE-1-bin-MAEQ-0.csv"
    cache_size, real_BMRC, real_OMRC = load_csv_mrc_file(real_file_name)

    ContentPop_mrc_file = f"./results/profile_res-{trace_name}-SHARDS-FIX_RATE-0.01-bin-MAEQ-2.csv"
    _, BMRC1, OMRC1= load_csv_mrc_file(ContentPop_mrc_file)


    ContentPop_mrc_file = f"./results/profile_res-{trace_name}-CARRA-FIX_RATE-0.01-bin-MAEQ-2.csv"
    _, BMRC2, OMRC2= load_csv_mrc_file(ContentPop_mrc_file)

    ContentPop_mrc_file = f"./results/profile_res-{trace_name}-FLOWS-FIX_RATE-0.01-bin-MAEQ-2.csv"
    _, BMRC3, OMRC3= load_csv_mrc_file(ContentPop_mrc_file)

    ContentPop_mrc_file = f"./results/profile_res-{trace_name}-DL-NONE-1-bin-MAEQ-2.csv"
    _, BMRC4, OMRC4= load_csv_mrc_file(ContentPop_mrc_file)

    bmrcs = [real_BMRC, BMRC4, BMRC1, BMRC2, BMRC3, ]
    omrcs = [real_OMRC, OMRC4, OMRC1, OMRC2, OMRC3, ]
    profile_names = ["Exact", "LPCA", "SHARDS", "Carra's", "FLOWS"]
    save_name = "plots/fig10.pdf"
    figure1_10_plot(cache_size, bmrcs, omrcs, profile_names, save_name)