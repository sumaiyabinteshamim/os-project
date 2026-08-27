/*
 * Adaptive Process Monitor & Optimizer
 * OS Project — demonstrates:
 *   1. System calls  : getpriority(), setpriority(), sysinfo()
 *   2. OS data collect: reads from the /proc filesystem
 *   3. Optimization  : detects the highest-CPU process and lowers
 *                      its scheduling priority (renices it) so
 *                      interactive tasks stay responsive.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>

/* Read a process name and CPU time from /proc/[pid]/stat */
int read_proc(int pid, char *name, unsigned long *utime, unsigned long *stime) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    int p; char comm[256]; char state;
    /* fields: pid (comm) state ... utime(14) stime(15) */
    if (fscanf(f, "%d %255s %c", &p, comm, &state) != 3) { fclose(f); return 0; }
    /* skip to fields 14 and 15 */
    for (int i = 4; i <= 13; i++) { long tmp; if (fscanf(f, "%ld", &tmp) != 1) break; }
    fscanf(f, "%lu %lu", utime, stime);
    fclose(f);
    strncpy(name, comm, 255);
    return 1;
}

int main(void) {
    /* ---- 1. Collect OS-wide data via the sysinfo() system call ---- */
    struct sysinfo info;
    sysinfo(&info);
    printf("=========================================\n");
    printf("   Adaptive Process Monitor & Optimizer\n");
    printf("=========================================\n\n");
    printf("System uptime      : %ld seconds\n", info.uptime);
    printf("Total RAM          : %lu MB\n", info.totalram / 1024 / 1024);
    printf("Free RAM           : %lu MB\n", info.freeram  / 1024 / 1024);
    printf("Processes running  : %d\n\n", info.procs);

    /* ---- 2. Collect per-process data from the /proc filesystem ---- */
    printf("Scanning /proc for running processes...\n\n");
    printf("%-8s %-20s %-10s %-8s\n", "PID", "NAME", "CPU_TICKS", "NICE");
    printf("-------------------------------------------------------\n");

    DIR *proc = opendir("/proc");
    struct dirent *entry;
    int   busiest_pid  = -1;
    unsigned long busiest_cpu = 0;

    while ((entry = readdir(proc)) != NULL) {
        /* only numeric directory names are PIDs */
        int pid = atoi(entry->d_name);
        if (pid <= 0) continue;

        char name[256];
        unsigned long ut = 0, st = 0;
        if (!read_proc(pid, name, &ut, &st)) continue;
        unsigned long total = ut + st;

        /* system call: getpriority() reads the process nice value */
        int nice_val = getpriority(PRIO_PROCESS, pid);

        printf("%-8d %-20.20s %-10lu %-8d\n", pid, name, total, nice_val);

        if (total > busiest_cpu) { busiest_cpu = total; busiest_pid = pid; }
    }
    closedir(proc);

    /* ---- 3. Optimization: renice the busiest process ---- */
    printf("\n=========================================\n");
    if (busiest_pid != -1) {
        printf("Busiest process detected: PID %d (%lu CPU ticks)\n",
               busiest_pid, busiest_cpu);
        printf("Optimizing: lowering its priority (renice +5)...\n");
        /* system call: setpriority() changes the nice value */
        if (setpriority(PRIO_PROCESS, busiest_pid, 5) == 0)
            printf("Success: PID %d priority lowered so other\n"
                   "         processes get fairer CPU time.\n", busiest_pid);
        else
            printf("Note: could not change priority (need higher\n"
                   "      permission for some system processes).\n");
    }
    printf("=========================================\n");
    return 0;
}
