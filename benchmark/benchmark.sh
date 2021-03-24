#!/bin/bash

# enable if you want line by line execution
# set -x
# trap read debug

################################################################################
# BENCHMARK TEST PLAN
################################################################################

# COMPARE ATTACH OVERHEAD
# 0)
# - Evaluate latency to call "sched_setscheduler" w/o daemon support
# - Evaluate latency to "attach" a thread with given TID to a logic task
# -- Vary task num (1-1024)
# 
# EVALUATE CREATE OVERHEAD
# 1)
# - Enable only EDF plugin performing 500 trials
# -- Vary CPUs num (1-20)
# -- Vary task num (1-1024)
# 2)
# - Enable only RM plugin performing 500 trials
# -- Vary CPUs num (1-20)
# -- Vary task num (1-1024)
# 3)
# - Enable only FP plugin performing 500 trials
# -- Vary CPUs num (1-20)
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
    # $1 plugin name
    # $2 cpu limit

    cp "$CWD/configs/benchmark.cfg" "$CWD/configs/schedconfig.cfg" 

    sed -i -e "s/PLG/$1/g" "$CWD/configs/schedconfig.cfg"
    sed -i -e "s/LIMIT/$2/g" "$CWD/configs/schedconfig.cfg"
    
    sudo mv -f "$CWD/configs/schedconfig.cfg" "$SCHED_CFG" 
}

function do_benchmark () {
    # $1 output file name
    # $2 cpu num
    # $3 test type (create / attach)
    # $4 policy
        # define SCHED_FIFO		1
        # define SCHED_RR		2
        # define SCHED_DD		6

    # bootstrap daemon (pin on CPU 0) and let it run in background
    sudo taskset 0x00000001 chrt -r 99 rtsd &
    DPID=$!

    # wait daemon is up and running
    sleep 1s

    # execute benchmark (pin on CPU 1)
    sudo taskset 0x00000002 chrt -r 99 ./benchmark "$1" "$2" "$3" "$4"
    sudo chown "$USER" "$1"

    # tear down daemon
    sudo kill -INT $DPID
    sudo killall rtsd
}

################################################################################
# POWER SAVING / TURBO / HYPERTHREADING / MSR TSC MANAGEMENT
################################################################################

function load_msr() {
    if ! lsmod | grep -w "msr" &> /dev/null ; then
        sudo modprobe msr
    fi
}

function set_min_freq() {
    # Sets "performance" as the new cpufreq governor
    for CPU_FREQ_GOVERNOR in $(ls /sys/devices/system/cpu/cpufreq/); 
    do
        sudo cp /sys/devices/system/cpu/cpufreq/"$CPU_FREQ_GOVERNOR"/scaling_{max,min}_freq
    done

    sleep 3s

    # Displays CPU frequency for each core 
    # grep -E '^model name|^cpu MHz' /proc/cpuinfo
}

function disable_powersaving() {
    # Sets "performance" as the new cpufreq governor
    for CPU_FREQ_GOVERNOR in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; 
    do [ -f "$CPU_FREQ_GOVERNOR" ] || continue;
        sudo echo -n performance | sudo tee "$CPU_FREQ_GOVERNOR" > /dev/null;
    done
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

    # Cores 0-4, 1-5, 2-6 and 3-7 are siblings, hence I will disable half of them
    for i in {4..7}; do
        echo -n 0 | sudo tee /sys/devices/system/cpu/cpu${i}/online > /dev/null;
    done
}

function print_cpus_freq() {
    # Displays CPU frequency for each core 
    grep -E '^model name|^cpu MHz' /proc/cpuinfo
}

################################################################################
# SETUP
################################################################################

MAX_TEST_NUM=3
MAX_CPU_NUM_CREATE=3
MAX_CPU_NUM_ATTACH=3
SCHED_CFG="/usr/share/rtsd/schedconfig.cfg"
CWD=$(pwd)
FILES=()

if [ "$1" == "-freq" ]; then
    disable_powersaving
    disable_turbo
    set_min_freq
    disable_hyperthreading
    print_cpus_freq
    
    exit
fi

for i in $(seq 0 $MAX_TEST_NUM); do FILES+=("results/benchmark$i.csv"); done

load_msr
benchmarks_setup

################################################################################
# PERFORM BENCHMARKS
################################################################################

# to keep cpu load constant spawn background workload (and then frequency)
nice -n +19 stress --cpu 4 &

# benchmark 0 -> attach vs sched_setscheduler

for i in $(seq $MAX_CPU_NUM_ATTACH); 
do
    generate_conf EDF "$i"
    do_benchmark "${FILES[0]}" "$i" "attach" "6"
done

# benchmark 1 -> EDF with variable core num

for i in $(seq $MAX_CPU_NUM_CREATE); 
do
    generate_conf EDF "$i"
    do_benchmark "${FILES[1]}" "$i" "create" ""
done

# benchmark 2 -> RM with variable core num

for i in $(seq $MAX_CPU_NUM_CREATE); 
do
    generate_conf RM "$i"
    do_benchmark "${FILES[2]}" "$i" "create" ""
done

# benchmark 3 -> RR with variable core num

for i in $(seq $MAX_CPU_NUM_CREATE); 
do
    generate_conf RR "$i"
    do_benchmark "${FILES[3]}" "$i" "create" ""
done

sudo killall stress