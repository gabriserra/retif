#!/bin/bash

################################################################################
# SETUP
################################################################################

FILE="output.csv"
CWD=$(pwd)

################################################################################
# DISABLE POWER SAVING
################################################################################

# Sets "performance" as the new cpufreq governor
for CPU_FREQ_GOVERNOR in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; 
do [ -f $CPU_FREQ_GOVERNOR ] || continue;
    sudo echo -n performance | sudo tee $CPU_FREQ_GOVERNOR > /dev/null;
done

# Displays CPU frequency for each core 
grep -E '^model name|^cpu MHz' /proc/cpuinfo

################################################################################
# MODIFY TURBO SETTINGS
################################################################################

# Let's move to intel pstate directory
cd /sys/devices/system/cpu/intel_pstate

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

cd $CWD

################################################################################
# DISABLE HYPERTHREADING
################################################################################

# Which physical core is assigned to each logical core by checking
# grep -H . /sys/devices/system/cpu/cpu*/topology/thread_siblings_list

# Cores 0-4, 1-5, 2-6 and 3-7 are siblings, hence I will disable half of them
# for i in {4..7}; do
    # echo -n 0 | sudo tee /sys/devices/system/cpu/cpu${i}/online > /dev/null;
# done

################################################################################
# CLEAN & BUILD
################################################################################
rm $FILE
make clean
make

################################################################################
# LOAD MSR KERNEL MODULE
################################################################################

if ! lsmod | grep "msr" &> /dev/null ; then
    sudo modprobe msr
fi

################################################################################
# FORCE CPU USAGE & BENCHMARK
################################################################################

# to avoid interference with test tasks, run them using a lower priority
# nice -n +20 stress --cpu 4

sudo taskset 0x00000001 chrt -r 99 ./benchmark $FILE
sudo chown $USER $FILE