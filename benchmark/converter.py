#!/usr/bin/env python3

import os
import argparse
from pathlib import Path
import pandas as pd
import numpy as np
from matplotlib import pyplot as plt

# +--------------------------------------------------------+
# |          Command-line Arguments Configuration          |
# +--------------------------------------------------------+


def dir_path(string):
    Path(string).mkdir(parents=True, exist_ok=True)
    return string
#-- dir_path


options = [
    {
        'short': None,
        'long': 'in_files',
        'opts': {
            'metavar': 'in-files',
            'type': argparse.FileType('r'),
            'nargs': '+',
        },
    },
    {
        'short': '-o',
        'long': '--out-dir',
        'opts': {
            'help': 'The output directory where to create all the tables',
            'type': dir_path,
            'default': 'out.d',
        },
    },
]


def parse_cmdline_args():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )

    for o in options:
        if o['short']:
            parser.add_argument(o['short'], o['long'], **o['opts'])
        else:
            parser.add_argument(o['long'], **o['opts'])

    return parser.parse_args()
#-- parse_cmdline_args

# +--------------------------------------------------------+
# |                  Benchmark Parameters                  |
# +--------------------------------------------------------+


repetitions = 500
cpumaxnum = 8
tasknum = 4

# +--------------------------------------------------------+
# |                    Plot Parameters                     |
# +--------------------------------------------------------+

lineWidth = 1.5
markerSize = 3
fontFamily = 'serif'
fontSize = 8
colors = ["red", "green", "blue", "purple"]

# +--------------------------------------------------------+
# |                     Plot Functions                     |
# +--------------------------------------------------------+

# TODO: which params do we need?


def plot_per_indexes(df, indexes, params):
    pass
#-- plot_per_indexes


# +--------------------------------------------------------+
# |                          Main                          |
# +--------------------------------------------------------+


def safe_save_to_csv(out_df, out_file):
    # Create a temporary file in the destination mount fs
    # (using tmp does not mean that moving = no copy)
    out_dir = os.path.dirname(os.path.abspath(out_file))
    Path(out_dir).mkdir(parents=True, exist_ok=True)
    tmpfile_name = out_dir + '/raw_' + str(os.getpid()) + '.tmp'
    out_df.to_csv(tmpfile_name, index=None)

    out_df.to_csv(tmpfile_name, index=None)

    # NOTE: It should be safe this way, but otherwise please
    # disable signal interrupts before this operation

    os.rename(tmpfile_name, out_file)

    # NOTE: If disabled, re-enable signal interrupts here
    # (or don't, the program will terminate anyway)
#-- safe_save_to_csv

# remove prefix polyfill
def remove_prefix(text, prefix):
    if text.startswith(prefix):
        return text[len(prefix):]
    return text  # or whatever

# remove suffix polyfill
def remove_suffix(text, suffix):
    if text.startswith(suffix):
        return text[len(suffix):]
    return text  # or whatever

def main():
    args = parse_cmdline_args()

    index_col_in = ['cpu', 'rip']
    index_col_out = index_col_in + ['taskn']

    for inf in args.in_files:
        df = pd.read_csv(inf, sep=',')
        has_sys = False
        taskn_max = 0
        out_cols = index_col_out + ['t_usr']

        for c in df.columns:
            if c in index_col_in:
                continue
            if c.endswith('sys'):
                has_sys = True
            cc = str(c)
            cc = remove_prefix(cc, 't')
            cc = cc.removesuffix('_sys')
            taskn = int(cc)
            if taskn > taskn_max:
                taskn_max = taskn
        #:

        if has_sys:
            out_cols += ['t_sys']

        # -- Lines from here on courtesy of Filippo Scotto

        if has_sys:
            headers = [
                [
                    'cpu', 'rip',
                    't' + str(i),
                    't' + str(i) + '_sys'
                ] for i in range(0, taskn_max+1)
            ]
        else:
            headers = [
                [
                    'cpu', 'rip',
                    't' + str(i),
                ] for i in range(0, taskn_max+1)
            ]

        bmt0 = [df[x] for x in headers]
        bmt0 = [bmt0[i].rename(
            columns={
                't' + str(i): 't_usr',
                't' + str(i) + '_sys': 't_sys'
            }) for i in range(0, taskn_max+1)]

        for i in range(0, taskn_max+1):
            bmt0[i]['taskn'] = i

        df_out = pd.concat(bmt0, ignore_index=True)
        df_out = df_out[out_cols]

        # --

        # df_out = pd.DataFrame(rows, columns=out_cols)
        safe_save_to_csv(df_out, os.path.abspath(inf.name) + '.out.csv')
    #:
#-- main


if __name__ == "__main__":
    main()
