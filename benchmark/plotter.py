#!/usr/bin/env python3

import os
import sys
import tikzplotlib
import pandas as pd
from cycler import cycler
import numpy as np
from matplotlib import cm
from matplotlib import colors
import matplotlib.pyplot as plt

##############################
# PLOT PARAMETERS
##############################

lineWidth   = 1.5
markerSize  = 3
fontFamily  = 'serif'
fontSize    = 9
titles      = ["EDF Plugin", "RM Plugin", "RR Plugin"]


##############################
# PLOT FUNCTIONS
##############################

def openDF(filename, separator=","):
    path = os.path.dirname(os.path.realpath(__file__))
    df = pd.read_csv(path + "/" + filename)

    return df

def generateFileNames(name, start=0, end=3):
    files = []

    for i in range(start, end+1):
        files.append(name + str(i) + ".csv")

    return files

def plotAttach(filename, ax, title="", xlabel="", ylabel=""):
    
    # open the file
    df = openDF(filename)
    
    # open the df as a multi index table
    multi = df.set_index(['cpu', 'rip'])

    # compute average over repetition
    y = multi.groupby(level=['cpu']).mean().dropna()

    # fix the number of cpu to 3 and get retif vs sys
    y_retif = y.loc[3, y.columns.take(range(1,2048,2))]
    y_sys = y.loc[3, y.columns.take(range(0,2048,2))]

    #
    plt.plot(range(1,1025), y_retif)
    plt.plot(range(1,1025), y_sys)

    #plt.plot(i, y[i])

    return
    # TODO: check if we can use this as CI 95%
    # read here: https://stackoverflow.com/questions/43016380/python-matplotlib-plotting-sample-means-in-bar-chart-with-confidence-intervals-b
    value = df.mean()
    std = df.std()
    x = range(len(df.columns))

    #plt.bar(x, value+np.abs(df.values.min()), bottom=df.values.min(), 
    #    yerr=std, align='center', alpha=0.5, color=colors)

    ax.set_title(title, pad=0.5)
    ax.set_ylabel(ylabel, fontsize=fontSize)
    ax.set_xlabel(xlabel, fontsize=fontSize)
    ax.set_xticks(x)
    #ax.set_xticklabels(labels)
    ax.grid()

def createPlot(filename, ax, title="", xlabel="", ylabel="", zlabel="", zlim=[15, 40], filter=True):
    
    # open the file
    df = openDF(filename)

    # open the df as a multi index table
    # https://stackoverflow.com/questions/24954117/advanced-averaging-with-multiindex-dataframe-in-pandas
    multi = df.set_index(['cpu', 'rip'])
    
    # filter out outliers 1%/99%
    if filter:
        filt_df = multi.loc[:]
        low = .01
        high = .99
        quant_df = filt_df.quantile([low, high])
        filt_df = filt_df.apply(lambda x: x[(x>quant_df.loc[low,x.name]) & (x < quant_df.loc[high,x.name])], axis=0)
    else:
        filt_df = multi
    
    # compute average over repetition and bring all to microsec
    z = filt_df.groupby(level=['cpu']).mean().dropna().divide(1000)
    
    # create mesh grid
    x = range(1, len(z.columns)+1)
    y = range(1, len(z)+1)

    x, y = np.meshgrid(x, y)

    # set fixed limits
    ax.set_zlim(zlim[0], zlim[1])
    ax.set_ylim(0, 21)
    
    # set ticks step, title and labels
    ax.yaxis.set_ticks(np.arange(0, len(z)+2, 5.0))
    ax.set_title(title, pad=0.5)    
    ax.set_xlabel(xlabel, fontsize=fontSize)
    ax.set_ylabel(ylabel, fontsize=fontSize)
    ax.set_zlabel(zlabel, fontsize=fontSize)

    # plot 3d surface
    surf = ax.plot_surface(x, y, z, ccount=10, cmap=cm.coolwarm,
        linewidth=0.5, antialiased=False, norm=colors.PowerNorm(0.5, vmin=13.5, vmax=30))

    # rotate view
    ax.view_init(30, 140)

##############################
# MAIN
##############################

def mainAttachPlot(filename, title):
    
    fig, ax = plt.subplots()

    plotAttach(filename, ax, title="", xlabel="", ylabel="")
    
    #plt.legend()

    # plt.savefig(filename + "_fig.png")
    plt.show()

def mainCreatePlot(filename, titles, xLab, yLab, zLab):
    # get file name
    dataFiles = generateFileNames(filename, 1)
    
    # set font size and family
    plt.rcParams['font.family'] = fontFamily
    plt.rcParams['font.size']   = fontSize
    
    # 1 x 3 3d surface plots
    fig, ax = plt.subplots(1, 3, subplot_kw={"projection": "3d"}, figsize=(15, 5))

    for i, f in enumerate(dataFiles):
        createPlot(f, ax[i], titles[i], xLab, yLab, zLab, [15, 40])

    plt.savefig(filename + "_fig.png")
    #plt.show()


mainCreatePlot("results_arm/benchmark", titles, "Task number", "CPU number", "Latency (μs)")
#mainAttachPlot("results_arm/benchmark0.csv", "ATTACH")