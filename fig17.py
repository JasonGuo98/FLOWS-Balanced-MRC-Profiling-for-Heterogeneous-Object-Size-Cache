from figure_plot import *

if __name__ == "__main__":
    trace_name = "wiki_2019u.oracleGeneral"

    real_file_name = f"./results/profile_res-{trace_name}-WS-ARC-0.010000-bin-MAE-1.csv"
    cache_size, BMRC0, OMRC0 = load_csv_mrc_file(real_file_name)



    ContentPop_mrc_file = f"./results/profile_res-{trace_name}-REAL-TwoQ-1.000000-bin-MAE-1.csv"
    _, BMRC1, OMRC1= load_csv_mrc_file(ContentPop_mrc_file)

    ContentPop_mrc_file = f"./results/profile_res-{trace_name}-REAL-ARC-1.000000-bin-MAE-1.csv"
    _, BMRC2, OMRC2= load_csv_mrc_file(ContentPop_mrc_file)

    ContentPop_mrc_file = f"./results/profile_res-{trace_name}-REAL-Cacheus-1.000000-bin-MAE-1.csv"
    _, BMRC3, OMRC3= load_csv_mrc_file(ContentPop_mrc_file)

    ContentPop_mrc_file = f"./results/profile_res-{trace_name}-REAL-S3FIFOd-1.000000-bin-MAE-1.csv"
    _, BMRC4, OMRC4= load_csv_mrc_file(ContentPop_mrc_file)


    ContentPop_mrc_file = f"./results/profile_res-{trace_name}-WS-TwoQ-0.010000-bin-MAE-1.csv"
    _, sample_BMRC1, sample_OMRC1= load_csv_mrc_file(ContentPop_mrc_file)

    ContentPop_mrc_file = f"./results/profile_res-{trace_name}-WS-ARC-0.010000-bin-MAE-1.csv"
    _, sample_BMRC2, sample_OMRC2= load_csv_mrc_file(ContentPop_mrc_file)

    ContentPop_mrc_file = f"./results/profile_res-{trace_name}-WS-Cacheus-0.010000-bin-MAE-1.csv"
    _, sample_BMRC3, sample_OMRC3= load_csv_mrc_file(ContentPop_mrc_file)

    ContentPop_mrc_file = f"./results/profile_res-{trace_name}-WS-S3FIFOd-0.010000-bin-MAE-1.csv"
    _, sample_BMRC4, sample_OMRC4= load_csv_mrc_file(ContentPop_mrc_file)

    bmrcs = [BMRC1, BMRC2, BMRC3, BMRC4,]
    omrcs = [OMRC1, OMRC2, OMRC3, OMRC4,]

    sample_bmrcs = [sample_BMRC1, sample_BMRC2, sample_BMRC3, sample_BMRC4,]
    sample_omrcs = [sample_OMRC1, sample_OMRC2, sample_OMRC3, sample_OMRC4,]

    profile_names = ["TwoQ", "ARC", "Cacheus", "S3FIFO"]
    save_name = "plots/fig17.pdf"
    figure17_plot(cache_size, bmrcs, omrcs, sample_bmrcs, sample_omrcs, profile_names, save_name)