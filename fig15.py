from figure_plot import *

if __name__ == "__main__":
    slide_bmrc_maeq, slide_omrc_maeq = get_all_slide_maeq_mean()
    methods_list = [f"slide{slide_ratio}" for slide_ratio in [0.1, 0.3, 0.5, 0.7, 0.9]]
    methods_list += ["CARRA"]
    namelist = ["FLOWS\n1:9", "FLOWS\n3:7", "FLOWS\n5:5", "FLOWS\n7:3", "FLOWS\n9:1", "Carra's"]
    save_name = "plots/fig15.pdf"
    figure15_plot(methods_list, slide_bmrc_maeq, slide_omrc_maeq, namelist, save_name)
