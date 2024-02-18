import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from utils import *

if not os.path.exists("plots"):
    os.makedirs("plots")

# plot style configure
try:
    plt.style.use('seaborn-v0_8-paper')
except:
    pass

plt.rcParams['xtick.direction'] = 'in'
plt.rcParams['ytick.direction'] = 'in'
import matplotlib.font_manager as fm
fm.fontManager.addfont('fonts/Times.ttf')
plt.rc('font',family='Times New Roman')

def figure1_10_plot(cache_size, BMR_s, OMR_s, names, save_name):
    """
    Draw a figure with two subplots showing the Byte Miss Ratio (BMR) and Object Miss Ratio (OMR) 
    for different cache sizes.

    Parameters:
    cache_size: The cache sizes.
    BMR_s: List of BMR values for different scenarios.
    OMR_s: List of OMR values for different scenarios.
    names: List of names for the scenarios.
    save_name (str): The filepath to save the figure.

    Returns:
    None
    """
    fig, ax = plt.subplots(tight_layout=True, nrows=1, ncols=2, figsize=(13, 6))

    x_idx = cache_size/(1024*1024)

    _linewidth = 2
    marker = ['D', 'v', 'o', "p"]
    def plot_data(this_ax, datas):
        this_ax.plot(x_idx, datas[0], linewidth=3*_linewidth, label=names[0], color='black')
        if(len(datas) > 4):
            this_ax.plot(x_idx, datas[4], linewidth=1*_linewidth, label=names[4],linestyle='--',
                        marker = marker[2], markeredgecolor='None', markersize = _linewidth*4, markevery=20, zorder = 1000)
        this_ax.plot(x_idx, datas[1], linewidth=1*_linewidth, label=names[1],linestyle='--',
                    marker = marker[0], markeredgecolor='None', markersize = _linewidth*4, markevery=20)
        this_ax.plot(x_idx, datas[2], linewidth=1*_linewidth, label=names[2],linestyle='--',
                    marker = marker[1], markeredgecolor='None', markersize = _linewidth*5, markevery=20)
        this_ax.plot(x_idx, datas[3], linewidth=1*_linewidth, label=names[3],linestyle='--',
                    marker = marker[3], markeredgecolor='None', markersize = _linewidth*4, markevery=19, )

        this_ax.tick_params(labelsize=24)

        this_ax.spines['bottom'].set_linewidth(2);
        this_ax.spines['left'].set_linewidth(2);
        this_ax.spines['right'].set_linewidth(2);
        this_ax.spines['top'].set_linewidth(2);

    plot_data(ax[0], BMR_s)
    plot_data(ax[1], OMR_s)
    ax[0].set_xscale('log')
    ax[1].set_xscale('log')

    # indicate the smaller cache sizes area
    where = (x_idx <= small_cache_size_B/(1024*1024))
    ax[0].fill_between(x_idx, 1, 0, where = where, color = 'lightgray')
    ax[1].fill_between(x_idx, 1, 0, where = where, color = 'lightgray')

    def set_subpolt_info(this_ax, x_label, y_label):
        this_ax.set_xlabel(x_label, fontsize=28, color='black')
        this_ax.set_ylabel(y_label, fontsize=28, color='black')

        this_ax.grid(linestyle = "--")
        this_ax.set_xlim(0.001, max(x_idx))
        this_ax.set_ylim(0, 1.0)

        leg = this_ax.legend(fontsize=24, frameon=True, columnspacing=0.1)
        leg.get_frame().set_linewidth(0.0)
        leg.get_frame().set_facecolor('white')
        leg.get_frame().set_alpha(0.5)

    set_subpolt_info(ax[0], "Cache Size (MB, log scale)", "Byte Miss Ratio")
    set_subpolt_info(ax[1], "Cache Size (MB, log scale)", "Object Miss Ratio")

    fig.tight_layout()
    plt.savefig(save_name, dpi=400)
    plt.show()

def figure4_plot(size_dists, req_dists, names, save_name):
    """
    Plots a figure with two subplots showing cache sizes distributions and request CDFs.

    Parameters:
    - size_dists: A list of two size distributions.
    - req_dists: A list of two request CDFs.
    - names: A list of two names for the subplots.
    - save_name: The filepath to save the figure.

    Returns:
    None
    """
    def draw_one(this_ax, idx, size_dist, req_dist, plot_left_label, plot_right_label, upper_or_lower):
        _my_lw = 2
        
        error_point_style = dict(markerfacecolor='k', marker='+', markersize = 4*_my_lw, markeredgecolor='k')
        bp = this_ax.boxplot(size_dist,patch_artist=True, flierprops= error_point_style)
        boxcolor_list = ['lightsteelblue','lightsteelblue']
        color_list = ['k','k']

        [bp['boxes'][i].set(color=boxcolor_list[i%2], edgecolor = color_list[i%2], alpha=1.0, linewidth = _my_lw) for i in range(len(size_dist))]
        [bp['medians'][i].set(color='k', linewidth = 2*_my_lw)for i in range(len(size_dist))]
        [bp['whiskers'][i*2].set(color=color_list[i%2], linewidth = _my_lw)for i in range(len(size_dist))]
        [bp['whiskers'][i*2+1].set(color=color_list[i%2], linewidth = _my_lw)for i in range(len(size_dist))]
        [bp['caps'][i*2].set(color=color_list[i%2], linewidth = _my_lw)for i in range(len(size_dist))]
        [bp['caps'][i*2+1].set(color=color_list[i%2], linewidth = _my_lw)for i in range(len(size_dist))]
        [bp['fliers'][i].set(markeredgecolor=color_list[i%2], linewidth = _my_lw)for i in range(len(size_dist))]

        this_ax.tick_params(labelsize=20)
        if(plot_left_label):
            this_ax.set_ylabel('Size Distribution', fontsize=28)
        this_ax.set_xlabel('#% hot objects', fontsize=28)
        
        this_ax.set_yscale('log', base = 10)
        this_ax.set_xticklabels(['0.01%', '0.1%', '1%', '10%', '100%',], fontsize=20, rotation=0)

        color = ['lightsteelblue', ]
        ec_color = ['k', ]
        labels = ['Size',]
        patches = [mpatches.Rectangle((0.5,0.5,), 1,1, facecolor=color[i],ec = ec_color[i], ls="-", linewidth=2, label="{:s}".format(labels[i]) ) for i in range(len(color))] 
        leg = this_ax.legend(patches, labels, ncol=1,  fontsize=24, loc = '%s left'%upper_or_lower)
        leg.get_frame().set_linewidth(0.0)
        leg.get_frame().set_facecolor('white')
        leg.get_frame().set_alpha(0.8)

        ax2 = this_ax.twinx()
        ax2.tick_params(labelsize=20)
        ax2.plot(idx, req_dist,alpha=0.8, linewidth = _my_lw*2,color='darkgreen',label=u'Req CDF', 
                linestyle='--', marker = 'D',markerfacecolor='dodgerblue', markeredgecolor='None', markersize = _my_lw*5, markevery=1)  
        leg = ax2.legend(loc='%s right'%upper_or_lower, fontsize=24,)
        leg.get_frame().set_linewidth(0.0)
        leg.get_frame().set_facecolor('white')
        leg.get_frame().set_alpha(0.8)
        ax2.set_ylim([0, 1.05]) 
        if plot_right_label:
            ax2.set_ylabel('Req CDF', fontsize=28)


        this_ax.spines['bottom'].set_linewidth(1);
        this_ax.spines['left'].set_linewidth(1);
        this_ax.spines['right'].set_linewidth(1);
        this_ax.spines['top'].set_linewidth(1);

    fig, ax = plt.subplots(nrows=1, ncols=2, figsize=(14,6.5))
    
    idx = [i+1 for i in range(5)]

    cdn_size_dist, wiki2019u_size_dist = size_dists
    cdn_req_dist, wiki2019u_req_dist = req_dists
    meta_cdn_name, wiki_2019_name = names

    draw_one(ax[0], idx, cdn_size_dist, cdn_req_dist, True, False, upper_or_lower = "lower")
    ax[0].set_title(meta_cdn_name, fontsize=28, y=1.01, color='black')
    draw_one(ax[1], idx, wiki2019u_size_dist, wiki2019u_req_dist, False, True, upper_or_lower = "upper")
    ax[1].set_title(wiki_2019_name, fontsize=28, y=1.01, color='black')
    
    fig.subplots_adjust(left=None, bottom=0.15, right=None, top=0.9, wspace=0.20, hspace=0.00)

    plt.savefig(save_name, dpi=400)
    plt.show()

def figure7_plot(uni_cov_s, size_cov_s, names, save_name):
    """
    Plots Figure 7 with two subplots showing the variation of WSS estimation coefficient
    with different spatial sampling rates.

    Parameters:
    uni_cov_s: List of arrays containing the WSS estimation coefficients for homogeneous cases.
    size_cov_s: List of arrays containing the WSS estimation coefficients for heterogeneous cases.
    names: List of names for the subplots.
    save_name: File name to save the figure.

    Returns:
    None
    """
    def draw_one(this_ax, idx, uni_cov, size_cov):
        this_ax.plot(idx, uni_cov)
        _linewidth = 1
        this_ax.plot(idx, size_cov, linewidth=4*_linewidth, label='Heterogeneous', color='darkred',
                    marker = 'v',markerfacecolor='deeppink', linestyle='--',markeredgecolor='None', markersize = _linewidth*15, markevery=1)
        this_ax.plot(idx, uni_cov, linewidth=4*_linewidth, label='Homogeneous', color='black',
                    marker = 'D',markerfacecolor='g', markeredgecolor='None', markersize = _linewidth*15, markevery=1)
        this_ax.tick_params(labelsize=24)

        this_ax.tick_params(axis='x', pad=10)

        this_ax.spines['bottom'].set_linewidth(2);
        this_ax.spines['left'].set_linewidth(2);
        this_ax.spines['right'].set_linewidth(2);
        this_ax.spines['top'].set_linewidth(2);

        this_ax.set_xticks([y*1+1 for y in range(len(['10%', '5%', '1%', '0.5%', '0.1%',]))],) #指定x轴的轴刻度个数
        this_ax.set_xticklabels(['10%', '5%', '1%', '0.5%', '0.1%',], fontsize=20, rotation=0)
        this_ax.set_xlabel('Spatial sampling rate', fontsize=28)
        this_ax.set_ylabel('WSS estimation coefficient of variation', fontsize=20)
        this_ax.grid(linestyle = "--")

        leg = this_ax.legend(loc='upper left', fontsize=18, frameon=True,)
        leg.get_frame().set_linewidth(0.0)
        leg.get_frame().set_facecolor('white')
        leg.get_frame().set_alpha(0.9)

    fig, ax = plt.subplots(nrows=1, ncols=2, figsize=(12,5.2))

    idx = [i+1 for i in range(5)]

    draw_one(ax[0], idx, uni_cov_s[0], size_cov_s[0])
    ax[0].set_title(names[0], fontsize=28, y=1.01, color='black')
    draw_one(ax[1], idx, uni_cov_s[1], size_cov_s[1])
    ax[1].set_title(names[1], fontsize=28, y=1.01, color='black')


    fig.subplots_adjust(left=None, bottom=0.18, right=None, top=0.9, wspace=0.30, hspace=0.00)

    plt.savefig(save_name, dpi=400)
    plt.show()

def figure11_plot(cache_sizes, exact_bomrc, profile_bomrc, save_name):
    """
    Plots the Figure 11 graph.

    Args:
        cache_sizes: List of cache sizes.
        exact_bomrc: List of exact BMRC values.
        profile_bomrc: List of profile BMRC values.
        save_name: The filepath to save the plot.
    Returns:
        None
    """
    if(trace_info_list == 18):
        fig, axs = plt.subplots(3, 6, figsize=(16, 6))
    else:
        fig, axs = plt.subplots(1, 3, figsize=(16, 6))
    axs = axs.flatten()
    for i in range(len(trace_info_list)):
        trace_info = trace_info_list[i]
        cache_size = cache_sizes[i]
        exact_bmrc = exact_bomrc[i]
        profile_bmrc = profile_bomrc[i]
        
        plot_start = 0
        for j in range(len(exact_bmrc[0])):
            if exact_bmrc[0,j] != 1 or exact_bmrc[1,j] != 1:
                plot_start = j
                break

        byte_maeq, obj_maeq = get_trace_maeq(exact_bmrc.T, profile_bmrc.T)
        print(f"{trace_info['name']}, byte_maeq: {byte_maeq:.4f}, obj_maeq: {obj_maeq:.4f}")
        cache_size = cache_size /(1024*1024)
        axs[i].plot(cache_size[plot_start:], exact_bmrc[0,plot_start:], linewidth = 2, color = 'black', label="Exact-BMRC")
        axs[i].plot(cache_size[plot_start:], profile_bmrc[0,plot_start:], linewidth = 2,  linestyle='--', label="FLOWS-BMRC")
        axs[i].plot(cache_size[plot_start:], exact_bmrc[1,plot_start:],linewidth = 2,  label="Exact-OMRC")
        axs[i].plot(cache_size[plot_start:], profile_bmrc[1,plot_start:], linewidth = 2,  linestyle='--', label="FLOWS-OMRC")
        axs[i].set_xscale("log")
        trace_name = trace_info["name"]
        axs[i].set_title(f"{trace_name}", fontsize=12)

        axs[i].grid(linestyle = "--")            #设置背景网格线为虚线
        axs[i].set_xlim(cache_size[plot_start], cache_size[-1])
        axs[i].set_ylim(0, 1.05)
        axs[i].tick_params(labelsize=10)

        if i == len(trace_info_list) - 1:
            axs[i].legend(bbox_to_anchor=(0.6, 0.9), fontsize=10, frameon=True)
            
    fig.text(0.5, 0.05, 'Cache Size (MB)', ha='center', va='center', fontsize=16)
    fig.text(0.1, 0.5, 'Miss Ratio', ha='center', va='center', rotation='vertical', fontsize=16)

    plt.subplots_adjust(wspace =0.3, hspace =0.6)
    plt.savefig(save_name)
    plt.show()

def figure12_plot(methods_list, bmrc_maeq, omrc_maeq, save_name):
    """
    Plot the mean absolute error (MAEQ) for different methods and traces.

    Args:
        methods_list: List of method names.
        bmrc_maeq: Dictionary containing BMRC MAEQ values for each trace and method.
        omrc_maeq: Dictionary containing OMRC MAEQ values for each trace and method.
        save_name: The path of the file to save the plot.

    Returns:
        None
    """
    try:
        plt.style.use('seaborn-v0_8-colorblind')
    except:
        pass
    plt.rc('axes', axisbelow=True)
    plt.figure(figsize=(12, 4))

    bars = np.array([i for i in range(len(trace_info_list))])
    plt.grid(True,linestyle='--')
    markers = ['D', 'v', 'o', '*', 'p', 'P', '*', 'X', 'd', '1', '2', '3', '4', '8', 'h', 'H', '+', 'x', '|', '_']
    offset = -0.4
    idx = 0
    for method in methods_list:
        bmrc_maeq_list = []
        omrc_maeq_list = []
        method_name = method.split("-")[0]
        for info in trace_info_list:
            trace_path = info["path"]
            trace_file = os.path.basename(trace_path)
            trace_name = info["name"]

            byte_maeq = bmrc_maeq[f"{trace_file}-{method}"]
            obj_maeq = omrc_maeq[f"{trace_file}-{method}"]
            bmrc_maeq_list.append(np.mean(byte_maeq))
            omrc_maeq_list.append(np.mean(obj_maeq))

        if method_name == "CARRA":
            plot_name = "Carra's"
        elif method_name == "SHARDS":
            plot_name = "SHARDS"
        elif method_name == "DL":
            plot_name = "LPCA"
        else:
            plot_name = method_name
        
        if(method_name == "FLOWS"):
            plt.bar([],[])
            plt.bar(bars + offset, bmrc_maeq_list, color = 'pink', edgecolor = 'k', linewidth=1, width=0.2, label=f"{plot_name}-BMRC")
        else:
            plt.bar(bars + offset, bmrc_maeq_list, edgecolor = 'k', width=0.2,linewidth=1, label=f"{plot_name}-BMRC")

        offset += 0.2
        idx += 1

    offset = -0.4
    idx = 0
    for method in methods_list:
        bmrc_maeq_list = []
        omrc_maeq_list = []
        method_name = method.split("-")[0]
        for info in trace_info_list:
            trace_path = info["path"]
            trace_file = os.path.basename(trace_path)
            trace_name = info["name"]

            byte_maeq = bmrc_maeq[f"{trace_file}-{method}"]
            obj_maeq = omrc_maeq[f"{trace_file}-{method}"]
            bmrc_maeq_list.append(np.mean(byte_maeq))
            omrc_maeq_list.append(np.mean(obj_maeq))

        if method_name == "CARRA":
            plot_name = "Carra's"
        elif method_name == "SHARDS":
            plot_name = "SHARDS"
        elif method_name == "DL":
            plot_name = "LPCA"
        else:
            plot_name = method_name
        
        if(idx == 0):
            plt.scatter([],[])
            plt.scatter(bars + offset, omrc_maeq_list, marker=markers[idx],s = 80,linewidth=0.5, edgecolor = 'k', label=f"{plot_name}-OMRC", zorder=999) 
        elif(method_name == "FLOWS"):
            plt.scatter(bars + offset, omrc_maeq_list, marker=markers[idx],s = 150,linewidth=0.5, color = 'yellow', edgecolor = 'k', label=f"{plot_name}-OMRC", zorder=999)         
        else:
            plt.scatter(bars + offset, omrc_maeq_list, marker=markers[idx], s = 80,linewidth=0.5, edgecolor = 'k', label=f"{plot_name}-OMRC", zorder=999) 

        offset += 0.2
        idx += 1

    x_label = []
    for info in trace_info_list:
        trace_name = info["name"]
        if(trace_name[:7] == "MetaCDN"):
            trace_name = "MC-" + trace_name[8:]
        if(trace_name[:6] == "MetaKV"):
            trace_name = "MK-" + trace_name[7:]
        x_label.append(trace_name)

    plt.xticks(bars, x_label, fontsize = 10)
    plt.yticks(fontsize = 16)
    plt.yscale('log')
    plt.xlabel("Trace", fontsize= 24)
    plt.ylabel("MAEQ", fontsize= 24)

    plt.legend(ncol=4, fontsize=14,frameon=False, loc='upper center', bbox_to_anchor=(0.5, 1.3))  
    plt.tight_layout()
    plt.savefig(save_name, dpi=400)
    plt.show()

def figure13_14_plot(methods, method_cluster, bmrc_maeq, omrc_maeq, cluster_name, labels, save_name, x_label_name):
    """
    Plot a figure with two subplots showing the boxplot of MAEQ values for BMRC and OMRC methods.

    Parameters:
    - methods: List of method names.
    - method_cluster: List of lists representing the clusters of methods.
    - bmrc_maeq: Dictionary containing the BMRC MAEQ values for each method.
    - omrc_maeq: Dictionary containing the OMRC MAEQ values for each method.
    - cluster_name: List of cluster names.
    - labels: List of labels for the legend.
    - save_name: Filepath to save the figure.
    - x_label_name: Label for the x-axis.

    Returns:
    - None
    """
    config = {
        "mathtext.fontset":'stix',
    }
    plt.rcParams.update(config)
    try:
        plt.style.use('seaborn-v0_8-colorblind')
    except:
        pass

    fig, ax = plt.subplots(nrows=1, ncols=2, figsize=(12, 6))

    boxcolor_list = plt.rcParams['axes.prop_cycle'].by_key()['color']
    def plot_one_box(method_cluster, this_ax, data):
        cluster_center = [i*5 + 3 for i in range(len(method_cluster))]
        positions = []
        box_colors = []


        cluster_idx = 0
        bar_width = 1

        for one_cluster in method_cluster:
            for idx in range(len(one_cluster)):
                it_x = cluster_center[cluster_idx] + (idx)*bar_width - len(one_cluster) * bar_width / 2 + bar_width/2
                positions.append(it_x)
                if(len(one_cluster) != 4):
                    box_colors.append(boxcolor_list[idx+1])
                else:
                    box_colors.append(boxcolor_list[idx])
            cluster_idx += 1

        _my_lw = 1
        error_point_style = dict(markerfacecolor='k', marker='+', markersize = 10, markeredgecolor='k')

        bp = this_ax.boxplot(data,patch_artist=True, flierprops=error_point_style, positions = positions, widths = 0.8)
        [bp['boxes'][i].set(alpha=1.0, color = box_colors[i], edgecolor = 'k', linewidth = _my_lw) for i in range(len(data))]
        [bp['medians'][i].set(color = 'k', linewidth = _my_lw)for i in range(len(data))]
        [bp['whiskers'][i].set(color = 'k', linewidth = _my_lw)for i in range(len(data))]
        [bp['caps'][i].set(color = 'k', linewidth = _my_lw)for i in range(len(data))]
        [bp['fliers'][i].set(markeredgecolor='k', linewidth = _my_lw)for i in range(len(data))]

        this_ax.yaxis.grid(True, linestyle = "--")
        this_ax.set_xticks(cluster_center)
        this_ax.tick_params(labelsize=18)
        this_ax.set_xticklabels([])
        this_ax.set_ylim(0, 0.45)
        this_ax.set_xticklabels(cluster_name, fontsize=18)
        this_ax.set_xlabel(x_label_name, fontsize=24)


    all_OMRC_MAEQ = []
    all_BMRC_MAEQ = []

    for method in methods:
        all_OMRC_MAEQ.append(omrc_maeq[method])
        all_BMRC_MAEQ.append(bmrc_maeq[method])
    plot_one_box(method_cluster, ax[0], all_BMRC_MAEQ)
    ax[0].set_ylabel('MAEQ', fontsize=24)
    ax[0].set_title("BMRC", fontsize=24, y=1.01, color='black')

    plot_one_box(method_cluster, ax[1], all_OMRC_MAEQ)
    ax[1].set_ylabel('MAEQ', fontsize=24)
    ax[1].set_title("OMRC", fontsize=24, y=1.01, color='black')

    if(len(labels) == 4):
        color = boxcolor_list[:4]
    else:
        color = boxcolor_list[1:1+len(labels)]
    ec_color = ["k" for i in range(len(labels))]

    patches = [mpatches.Rectangle((0.5,0.5,), 0.25,0.25, facecolor=color[i],ec = ec_color[i], ls="-", linewidth=1, label="{:s}".format(labels[i]) ) for i in range(len(color))]

    leg = fig.legend(patches, labels, ncol=len(labels),  fontsize=24, loc = 'upper center', columnspacing=1)
    leg.get_frame().set_linewidth(0.0)
    leg.get_frame().set_facecolor('white')
    leg.get_frame().set_alpha(0.8)

    fig.subplots_adjust(left=None, bottom=0.15, right=0.95, top=0.78, wspace=None, hspace=None)

    plt.savefig(save_name, dpi=400)
    plt.show()

def figure15_plot(methods_list, bmrc_maeq, omrc_maeq, namelist, save_name):
    """
    Plot a bar chart comparing the mean absolute error (MAEQ) for different methods.

    Parameters:
    methods_list: List of methods to compare.
    bmrc_maeq: Dictionary containing the BMRC MAEQ values for each method.
    omrc_maeq: Dictionary containing the OMRC MAEQ values for each method.
    namelist: List of sample configurations.
    save_name: File name to save the plot.

    Returns:
    None
    """
    plt.rc('axes', axisbelow=True)
    try:
        plt.style.use('seaborn-v0_8-colorblind')
    except:
        pass
    plt.figure(figsize=(12, 4))

    bars = np.array([i*1.2+1 for i in range(len(methods_list))])
    plt.grid(True,linestyle='--')
    offset = 0.25
    bmrc_maeq_list = [np.mean(bmrc_maeq[method]) for method in methods_list]
    omrc_maeq_list = [np.mean(omrc_maeq[method]) for method in methods_list]  
    boxcolor_list = plt.rcParams['axes.prop_cycle'].by_key()['color'] 
    boxcolor_list = boxcolor_list[:1] + boxcolor_list[2:] 

    plt.bar(bars - offset, bmrc_maeq_list, color = boxcolor_list[0], edgecolor = 'k', width=0.4, label=f"BMRC")
    plt.bar(bars + offset, omrc_maeq_list, color = boxcolor_list[1], edgecolor = 'k', width=0.4, label=f"OMRC")
        
    x_label = namelist
    plt.xticks(bars, x_label, fontsize = 16)
    plt.yticks(fontsize = 16)
    plt.xlabel("SampleConfiguration", fontsize= 20)
    plt.ylabel("MAEQ", fontsize= 20)

    plt.legend(fontsize = 16, borderaxespad=0.)
    plt.tight_layout()
    plt.savefig(save_name, dpi=400)
    plt.show()

def figure17_plot(cache_size, BMR_s, OMR_s, sample_bmrcs, sample_omrcs, names, save_name):
    try:
        plt.style.use('seaborn-v0_8-colorblind')
    except:
        pass
    fig, ax = plt.subplots(tight_layout=True, nrows=1, ncols=2, figsize=(13, 6))
    
    x_idx = cache_size/(1024*1024*1024)

    _linewidth = 2
    marker = ['o', 'o', 'o', 'o', 'o']
    
    def plot_data(this_ax, datas):
        this_ax.plot(x_idx, datas[3], linewidth=3*_linewidth, label=names[3], alpha=0.5,)
        this_ax.plot(x_idx, datas[2], linewidth=3*_linewidth, label=names[2],alpha=0.5,)
        this_ax.plot(x_idx, datas[1], linewidth=3*_linewidth, label=names[1], alpha=0.5,)
        this_ax.plot(x_idx, datas[0], linewidth=3*_linewidth, label=names[0], alpha=0.5,)
        
        this_ax.tick_params(labelsize=24)

        this_ax.spines['bottom'].set_linewidth(2)
        this_ax.spines['left'].set_linewidth(2)
        this_ax.spines['right'].set_linewidth(2)
        this_ax.spines['top'].set_linewidth(2)
    
    def scatter_sample(this_ax, samples):
        evenly_sample = 7
        this_ax.scatter(x_idx[::evenly_sample], samples[3][::evenly_sample], label=names[3], marker = marker[3], edgecolor='None', s = _linewidth*30,)
        this_ax.scatter(x_idx[::evenly_sample], samples[2][::evenly_sample], label=names[2], marker = marker[2], edgecolor='None', s = _linewidth*30,)
        this_ax.scatter(x_idx[::evenly_sample], samples[1][::evenly_sample], label=names[1], marker = marker[1], edgecolor='None', s = _linewidth*30,)
        this_ax.scatter(x_idx[::evenly_sample], samples[0][::evenly_sample], label=names[0], marker = marker[0], edgecolor='None', s = _linewidth*30,)
        
        this_ax.tick_params(labelsize=24)

        this_ax.spines['bottom'].set_linewidth(2)
        this_ax.spines['left'].set_linewidth(2)
        this_ax.spines['right'].set_linewidth(2)
        this_ax.spines['top'].set_linewidth(2)

    plot_data(ax[0], BMR_s)
    scatter_sample(ax[0], sample_bmrcs)
    plot_data(ax[1], OMR_s)
    scatter_sample(ax[1], sample_omrcs)

    where = (x_idx <= small_cache_size_B/(1024*1024))
    ax[0].fill_between(x_idx, 1, 0, where = where, color = 'lightgray')
    ax[1].fill_between(x_idx, 1, 0, where = where, color = 'lightgray')

    def set_subpolt_info(this_ax, title, x_label, y_label):
        this_ax.set_xlabel(x_label, fontsize=28, color='black')
        this_ax.set_ylabel(y_label, fontsize=28, color='black')

        this_ax.grid(linestyle = "--")
        this_ax.set_xlim(0.001, max(x_idx))

    lines, labels = fig.axes[0].get_legend_handles_labels()
    print(lines)
    print(labels)
    
    exact_line = plt.Line2D([], [], color='black', linewidth=3*_linewidth, label='Exact', alpha=0.5,)
    sample_dots = plt.scatter([], [], label='Sampled(0.01)', c = 'k', marker = marker[0], edgecolor='None', s = _linewidth*30,)
    lines = lines[:4] + [sample_dots, exact_line]
    labels = labels[:4] + ["WeightedSample(0.01)", "Exact"]
    
    fig.legend(lines, labels, loc = 'upper center', ncol=6, fontsize=18, borderaxespad=0.1, frameon=False)

    set_subpolt_info(ax[0], "BMRC", "Cache Size (GB)", "Byte Miss Ratio")
    set_subpolt_info(ax[1], "OMRC", "Cache Size (GB)", "Object Miss Ratio")

    fig.tight_layout()
    plt.subplots_adjust(left=0.1, right=0.9, top=0.9, bottom=0.15)

    plt.savefig(save_name, dpi=400)
    plt.show()
