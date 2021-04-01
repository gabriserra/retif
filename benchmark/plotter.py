#!/usr/bin/env python3

import os
import sys
import tikzplotlib
import pandas as pd
from cycler import cycler
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm

##############################
# BENCHMARK PARAMETERS
##############################

repetition  = 500
cpumaxnum   = 3
tasknum     = 1024

##############################
# PLOT PARAMETERS
##############################

lineWidth   = 1.5
markerSize  = 3
fontFamily  = 'serif'
fontSize    = 8
colors      = ["red", "green", "blue", "purple"]

##############################
# PLOT FUNCTIONS
##############################

def openDF(filename, separator=","):
    path = os.path.dirname(os.path.realpath(__file__))
    df = pd.read_csv(path + "/" + filename)

    return df

def plotAttach(filename, ax, title="", xlabel="", ylabel=""):
    
    labels = ["EDF", "RM", "FP", "RR"]
    df = openDF(filename)

    # TODO: check if we can use this as CI 95%
    # read here: https://stackoverflow.com/questions/43016380/python-matplotlib-plotting-sample-means-in-bar-chart-with-confidence-intervals-b
    value = df.mean()
    std = df.std()
    x = range(len(df.columns))

    plt.bar(x, value+np.abs(df.values.min()), bottom=df.values.min(), 
        yerr=std, align='center', alpha=0.5, color=colors)

    ax.set_title(title, pad=0.5)
    ax.set_ylabel(ylabel, fontsize=fontSize)
    ax.set_xlabel(xlabel, fontsize=fontSize)
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    ax.grid()

def plotCreate1(filename, ax, title="", xlabel="", ylabel=""): 
    df = openDF(filename)
    multi = df.set_index(['cpu', 'rip'])

    x = range(len(df.columns))
    value = df.mean(level=['rip'])

def plotData(filename, ax, title="", xlabel="", ylabel=""):
    df = openDF(filename)
    multi = df.set_index(['cpu', 'rip'])

    # https://stackoverflow.com/questions/24954117/advanced-averaging-with-multiindex-dataframe-in-pandas

    x = range(1, tasknum+1)
    y = range(1, cpumaxnum+1)
    x, y = np.meshgrid(x, y)
    z = multi.groupby(level=['cpu']).mean().dropna()

    #ax.set_zlim(50000, 180000)
    surf = ax.plot_surface(x, y, z, ccount=10, cmap=cm.coolwarm,
        linewidth=0.5, antialiased=False)

    ax.view_init(30, 140)

##############################
# MAIN
##############################

def mainPlotAttach():
    
    fig, ax = plt.subplots()

    plotAttach("results/benchmark0.csv", ax, title="", xlabel="", ylabel="")
    plt.legend()
    plt.show()

def mainPlotData():
    plt.rcParams['font.family'] = fontFamily
    plt.rcParams['font.size']   = fontSize
    fig, ax = plt.subplots(subplot_kw={"projection": "3d"})

    plotData("results/benchmark3.csv", ax, title="", xlabel="", ylabel="")
    plt.show()

mainPlotData()
