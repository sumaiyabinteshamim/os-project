# Adaptive Process Monitor and Optimizer

An Operating Systems project written in C for Linux. It monitors running processes, collects live data from the kernel, and optimizes CPU fairness by adjusting process scheduling priority.

## Project Requirements Covered

### 1. System Calls
- sysinfo() : retrieves system-wide stats (uptime, RAM, process count)
- getpriority() : reads a process current scheduling priority (nice value)
- setpriority() : changes a process priority (the optimization action)

### 2. OS Data Collection
- Reads the Linux /proc filesystem for per-process information
- Parses /proc/[pid]/stat for each process CPU usage (utime + stime)

### 3. Optimization
- Detects the process using the most CPU time
- Lowers its scheduling priority (renice +5) so other processes get fairer CPU time, improving overall system responsiveness

## How to Build and Run

    gcc sysmon.c -o sysmon
    ./sysmon

## Requirements

- Linux operating system (uses the /proc filesystem)
- GCC compiler

## Author

Sumaiya Binte Shamim