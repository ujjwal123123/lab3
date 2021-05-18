#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> // for exit
#include <string.h> // for strcpy

#define MAX_PROCESS 100
#define START_NOT_SET -1

typedef struct {
    char *name;
    int arrival;
    int cpuburst;
    int remaining; // for use by RR and SRTF
    int start;
    int end;
} Process;

Process
    globalProcessTable[MAX_PROCESS]; // global variable to store process data
int pcount = 0;                      // process count

int min(int a, int b) { return a < b ? a : b; }

// Parse a text file to globalProcessTable
void readProcessTable(const char *filename) {
    FILE *infile = fopen(filename, "r");

    if (infile == NULL) {
        fprintf(stderr, "File could not be read\n");
    }

    char processName[40];
    int arrival, cpuburst;

    while (fscanf(infile, " %s %d %d", processName, &arrival, &cpuburst) !=
           EOF) {
        globalProcessTable[pcount] = (Process){
            .name = (char *)malloc(sizeof(char) * (strlen(processName)) + 1),
            .arrival = arrival,
            .cpuburst = cpuburst,
            .remaining = cpuburst,
            .start = START_NOT_SET};
        strcpy(globalProcessTable[pcount++].name, processName);
    }
}

void printStatistics(Process *processTable) {
    int totalTurnaround = 0, totalWait = 0;

    printf("\n%-10s %-10s %-20s\n", "Name", "Turnaround", "Waiting");

    for (int i = 0; i < pcount; i++) {
        Process *process = &processTable[i]; // this process
        int turnaroundTime = process->end - process->arrival;
        int waitingTime = turnaroundTime - process->cpuburst;

        printf("%-10s %-10d %-10d\n", process->name, turnaroundTime,
               waitingTime);

        totalTurnaround += turnaroundTime;
        totalWait += waitingTime;
    }

    printf("\nAverage turnaround time: %f\nAverage wait time: %f\n\n",
           (double)totalTurnaround / pcount, (double)totalWait / pcount);
}

// First come first serve
void FCFS() {
    // Copy to globalProcessTable to local variable
    Process *processTable = (Process *)malloc(sizeof(Process) * pcount);
    memcpy(processTable, globalProcessTable, sizeof(Process) * pcount);

    int currentTime = 0;

    printf("%-10s %-10s %-10s %-10s %-10s\n", "Name", "Arrival", "CPU Burst",
           "Start", "End");

    for (int i = 0; i < pcount; i++) {
        Process *process = &processTable[i]; // this process
        if (process->arrival <= currentTime) {
            process->start = currentTime;

            currentTime += process->cpuburst;
            process->end = currentTime;

            printf("%-10s %-10d %-10d %-10d %-10d\n", process->name,
                   process->arrival, process->cpuburst, process->start,
                   process->end);
        }
    }

    printStatistics(processTable);
    free(processTable);
}

// Round robin
void RR(int timeQuantum) {
    // Copy to globalProcessTable to local variable
    Process *processTable = (Process *)malloc(sizeof(Process) * pcount);
    memcpy(processTable, globalProcessTable, sizeof(Process) * pcount);

    int currentTime = 0;

    printf("%-10s %-10s %-10s %-10s %-10s\n", "Name", "Arrival", "Remaining",
           "Start", "End");

    bool allExecuted = false; // true if all processes have been executed
    while (!allExecuted) {
        allExecuted = true;

        for (int i = 0; i < pcount; i++) {
            Process *process = &processTable[i]; // this process
            if (process->arrival <= currentTime && process->remaining != 0) {
                if (process->start == START_NOT_SET) {
                    process->start = currentTime;
                }
                // time we spend on this process in this iteration
                int timeSpent = min(timeQuantum, process->remaining);

                process->remaining -= timeSpent;
                currentTime += timeSpent;
                process->end = currentTime;

                printf("%-10s %-10d %-10d %-10d %-10d\n", process->name,
                       process->arrival, process->remaining, process->start,
                       process->end);

                allExecuted = false;
            }
        }
    }

    printStatistics(processTable);
    free(processTable);
}

// Select the process with shortest remaining execution time.
// Return NULL if all processes have been executed.
Process *getShortestProcess(Process *processTable, int currentTime) {
    Process *shortest = NULL;
    for (int i = 0; i < pcount; i++) {
        Process *process = &processTable[i];
        if (process->remaining != 0) {
            if (shortest == NULL) {
                shortest = process;
            }
            else if (process->remaining < shortest->remaining &&
                     process->arrival <= currentTime) {
                shortest = process;
            }
        }
    }
    return shortest;
}

// Shortest Remaining Burst First
void SRBF() {
    Process *processTable = (Process *)malloc(sizeof(Process) * pcount);
    memcpy(processTable, globalProcessTable, sizeof(Process) * pcount);

    int currentTime = 0;

    printf("%-10s %-10s %-10s %-10s %-10s\n", "Name", "Arrival", "Remaining",
           "Start", "End");

    while (true) {
        Process *process = getShortestProcess(processTable, currentTime);

        if (process == NULL) {
            break;
        }

        process->remaining -= 1;

        if (process->start == START_NOT_SET) {
            process->start = currentTime;
        }
        process->end = ++currentTime;

        if (process->remaining == 0) {
            printf("%-10s %-10d %-10d %-10d %-10d\n", process->name,
                   process->arrival, process->remaining, process->start,
                   process->end);
        }
    }

    printStatistics(processTable);
    free(processTable);
}

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Provide input file name\n");
        exit(1);
    }

    const char *infile = argv[1];
    printf("filename: %s\n", infile);

    readProcessTable(infile);

    printf("-------------------------------------------------\n");
    printf("First Come First Serve Scheduling\n");
    printf("-------------------------------------------------\n");
    FCFS();

    int RRTimeQuantum;
    printf("Enter Round Robin Time Quantum: ");
    scanf(" %d", &RRTimeQuantum);
    printf("\n");
    printf("-------------------------------------------------\n");
    printf("Round robin\n");
    printf("-------------------------------------------------\n");
    RR(RRTimeQuantum);
    printf("-------------------------------------------------\n");
    printf("Shortest remaining burst first\n");
    printf("-------------------------------------------------\n");
    SRBF();

    return 0;
}
