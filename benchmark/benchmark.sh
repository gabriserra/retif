#!/bin/bash

################################################################################
# BENCHMARK TEST PLAN
################################################################################

# COMPARE ATTACH OVERHEAD
# 0)
# - Evaluate latency to call "sched_setscheduler" w/o daemon support
# - Evaluate latency to "attach" a thread with given TID to a logic task
# 
# EVALUATE CREATE OVERHEAD
# 1)
# - Enable a set of plugins with a given set of CPUs
# -- Vary task num (1-1024)
# 2)
# - Enable only EDF plugin performing 500 trials
# -- Vary CPUs num (1-40)
# -- Vary task num (1-1024)
# 3)
# - Enable only RM plugin performing 500 trials
# -- Vary CPUs num (1-40)
# -- Vary task num (1-1024)
# 4)
# - Enable only FP plugin performing 500 trials
# -- Vary CPUs num (1-40)
# -- Vary task num (1-1024)

################################################################################
# BENCHMARK FUNCTIONS
################################################################################

function benchmarks_setup() {
    
    if [ -d results ]; then
        rm -rf results/*.csv
    else
        mkdir results
    fi
    
    make clean
    make
}

function generate_conf() {
    # $1 test number
    # $2 plugin name
    # $3 cpu limit

    if [ "$1" == 0 ] || [ "$1" == 1 ]; then
        N=$1
    else
        N="n"
    fi
    
    cp "$CWD/configs/benchmark$N.cfg" "$CWD/configs/schedconfig.cfg" 

    if [ "$1" != 0 ] && [ "$1" != 1 ]; then
        sed -i -e "s/PLG/$2/g" "$CWD/schedconfig.cfg"
        sed -i -e "s/LIMIT/$3/g" "$CWD/schedconfig.cfg"
    fi
    
    mv -f "$CWD/configs/schedconfig.cfg" "$SCHED_CFG" 
}

function do_benchmark () {
    # $1 output file name

    # to avoid interference with test tasks, run them using a lower priority
    # nice -n +20 stress --cpu 4
    
    # bootstrap daemon (pin on CPU 1) and let it run in background
    sudo taskset 0x00000010 chrt -r 99 rtsd 
    DPID=$!

    # execute benchmark 0 (pin on CPU 0)
    sudo taskset 0x00000001 chrt -r 99 ./benchmark "$1"
    sudo chown "$USER" "$1"

    # tear down daemon
    kill -INT $DPID
}

################################################################################
# POWER SAVING / TURBO / HYPERTHREADING / MSR TSC MANAGEMENT
################################################################################

function load_msr() {
    if ! lsmod | grep "msr" &> /dev/null ; then
        sudo modprobe msr
    fi
}

function disable_powersaving() {
    # Sets "performance" as the new cpufreq governor
    for CPU_FREQ_GOVERNOR in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; 
    do [ -f "$CPU_FREQ_GOVERNOR" ] || continue;
        sudo echo -n performance | sudo tee "$CPU_FREQ_GOVERNOR" > /dev/null;
    done

    # Displays CPU frequency for each core 
    grep -E '^model name|^cpu MHz' /proc/cpuinfo
}

function disable_turbo() {
    # Let's move to intel pstate directory
    cd /sys/devices/system/cpu/intel_pstate || 
        echo "Intel pstate not available" && return

    FREQ_PERCENTAGE_1=$(($(cat max_perf_pct) - $(cat turbo_pct) - 5))
    FREQ_PERCENTAGE_2=$(($(cat turbo_pct) - 5))

    if [ $FREQ_PERCENTAGE_1 -lt $FREQ_PERCENTAGE_2 ] ; then
        FREQ_PERCENTAGE=$FREQ_PERCENTAGE_1
    else
        FREQ_PERCENTAGE=$FREQ_PERCENTAGE_2
    fi

    # Disable turbo boost
    echo 1 | sudo tee no_turbo > /dev/null

    # And set cpu Pstate min and max to be equal
    echo $FREQ_PERCENTAGE | sudo tee max_perf_pct > /dev/null
    echo $FREQ_PERCENTAGE | sudo tee min_perf_pct > /dev/null

    cd "$CWD" || exit
}

function disable_hyperthreading() {
    # Which physical core is assigned to each logical core by checking
    grep -H . /sys/devices/system/cpu/cpu*/topology/thread_siblings_list

    Cores 0-4, 1-5, 2-6 and 3-7 are siblings, hence I will disable half of them
    for i in {4..7}; do
        echo -n 0 | sudo tee /sys/devices/system/cpu/cpu${i}/online > /dev/null;
    done
}

################################################################################
# SETUP
################################################################################

TEST_NUM=4
MAX_CPU_NUM=8
SCHED_CFG="/usr/share/rtsd/schedconfig.cfg"
CWD=$(pwd)
FILES=()

for i in $(seq 0 $TEST_NUM); do FILES+=("results/benchmark$i.csv"); done

#disable_powersaving
#disable_turbo
#disable_hyperthreading
#load_msr
benchmarks_setup

################################################################################
# PERFORM BENCHMARKS
################################################################################

# benchmark 0 -> attach time

generate_conf 0
do_benchmark "${FILES[0]}"

# benchmark 1 -> fixed config

generate_conf 1
do_benchmark "${FILES[1]}"

# benchmark 2 -> EDF with variable core num

for i in $(seq $MAX_CPU_NUM); 
do
    generate_conf 2 EDF "$i"
    do_benchmark "${FILES[2]}"
done

# benchmark 3 -> RM with variable core num

for i in $(seq $MAX_CPU_NUM); 
do
    generate_conf 3 RM "$i"
    do_benchmark "${FILES[3]}"
done

# benchmark 4 -> FP with variable core num

for i in $(seq $MAX_CPU_NUM); 
do
    generate_conf 4 RM "$i"
    do_benchmark "${FILES[4]}"
done