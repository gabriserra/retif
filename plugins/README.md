# Retif Default Plugins Set

This directory contains the implementation for a set of plugins provided by
Retif alongside its main components. The plugins enable the functionality of the
daemon, which cannot operate if no plugin is loaded, as it implements no actual
functionality related to real-time scheduling.

### EDF

> Requires kernel version greater than 3.14

This plugin implements the EDF scheduling algorithm, which is well known to be
optimum for single processor systems. In particular, we implemented a
fully-partitioned version of EDF applying a worst-fit task allocation strategy
among the CPU cores specified via the RTS Daemon configuration file. This plugin
ensures that the task execution will be suspended in case it overpass the
runtime parameter provided. A task could specify a runtime greater that the one
required and the daemon dinamycally will decide how much budget provide to the
task (the minimum is always guaranteed if accepted).

Stricly required parameters:
- Runtime
- Period

Other parameters:
- Runtime desired
- Deadline

### RM

This plugin implements the Rate Monotonic (RM) scheduling algorithm, which is
well known to be optimum for single processor systems among FP scheduling
algorithms. In particular, it implements a fully-partitioned version of RM
applying a worst-fit task allocation strategy on top of the POSIX `SCHED_FIFO`
scheduling policy among the CPU cores specified via the RTS Daemon configuration
file. The only required parameter that a real-time task shall declare to be
eligible to be scheduled with this plugin is its period.

Stricly required parameters:
- Period

Other parameters:
- Runtime
- Deadline

### FP & RR

The Fixed Priority (FP) and Round Robin (RR) plugins serve as wrappers to expose
underlying POSIX functionality to applications that use this framework. They
respectively provide access to `SCHED_FIFO` and `SCHED_RR` scheduling policies
and as such the only required parameter that shall be specified to be accepted
by either of these plugins is the desired POSIX priority of the task. For this
reason, no admission test is performed when submitting a task to these plugins,
although a task may still specify other parameters that may be considered by
other plugins’ admission tests. Both plugins apply a worst- fit task allocation
strategy, in this case resulting in each new task to be assigned to the CPU core
with the least number of assigned tasks.

Stricly required parameters:
- Priority
