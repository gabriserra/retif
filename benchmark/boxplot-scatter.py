#!/usr/bin/env python3

from cycler import cycler
import matplotlib.ticker as tkr
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import os

plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.size'] = 12
# plt.rcParams['font.serif'] = ['Computer Modern']

def my_boxplot(df,
               title='',
               xticklabels=[],
               show_scatter=False,
               logscale=False,
               ylabel='',
               xlabel='',
               ):

    data = []
    for col in df.columns:
        data.append(df[col].dropna())

    fig, ax = plt.subplots(figsize=(5, 5))

    linewidth = 1.5
    markersize = 3
    alpha = 0.1

    # Scatter plot, below the boxplot
    if show_scatter:
        for i, box in enumerate(data):
            y = box
            x = np.random.normal(1 + i, 0.02, size=len(y))
            plt.plot(x, y, 'o', markersize=markersize, alpha=alpha)

    # Use column names if not overridden by the user
    if not xticklabels:
        xticklabels = df.columns

    # First boxplot, plots one per each column
    TRANSPARENT = [0, 0, 0, 0]
    facecolor = 'white' if not show_scatter else TRANSPARENT
    bpdict = ax.boxplot(data,
                        labels=xticklabels,
                        patch_artist=True,
                        showfliers=not show_scatter,
                        whis=[0, 100],
                        whiskerprops = dict(linewidth=linewidth, color='black'),
                        boxprops = dict(linewidth=linewidth, color='black', facecolor=facecolor),
                        capprops = dict(linewidth=linewidth, color='black'),
                        medianprops=dict(linewidth=linewidth, color='black'),
                        )

    # Set title with the one provided by the user
    fig.canvas.set_window_title(title)
    ax.set_ylabel(ylabel, fontsize=16)
    ax.set_xlabel(xlabel, fontsize=16)
    ax.locator_params(axis='y', nbins=6)
    ax.set_ylim([0, 275000])
    ax.tick_params(direction='in', width=linewidth, length=10)
    for axis in ['top', 'bottom', 'left', 'right']:
        ax.spines[axis].set_linewidth(linewidth)

    # Custom formatter function
    def numfmt(x, pos):
        s = '{:.2f}'.format(x / 1000000.0)
        return s
    yfmt = tkr.FuncFormatter(numfmt)
    ax.yaxis.set_major_formatter(yfmt)

    plt.grid()

    if logscale:
        plt.yscale('log')

    # plt.show()

    return fig


path = os.path.dirname(os.path.realpath(__file__))

dropfirst_n = 10

d = {
    'POSIX': np.loadtxt(path + '/socket_pingpong.data'),
    'UDP-DPDK': np.loadtxt(path + '/udpdk_pingpong.data'),
}

# for k in d:
#     d[k] = d[k][dropfirst_n:d[k].size]

df = pd.DataFrame(dict([(k, pd.Series(v)) for k, v in d.items()]))
fig = my_boxplot(
    df,
    title='',
    show_scatter=True,
    logscale=False,
    ylabel='Round Trip Time [ms]',
    xlabel='',
    )

exts = ['pdf', 'png']

for ext in exts:
    fig.savefig(
        path + '/../figs/plot-rt.' + ext,
        bbox_inches='tight',
        dpi=300,
        )
