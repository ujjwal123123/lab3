#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> // for exit
#include <string.h> // for strcpy

#define MAX_PROCESS 100

typedef struct {
    char *name;
    int arrival;
    int cpuburst;
    int remaining; // for use by RR and SRTF
    int start;
    int end;
} Process;

Process globalProcessTable[MAX_PROCESS];
int pcount = 0; // process count

int min(int a, int b) { return a < b ? a : b; }

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
            .remaining = cpuburst};
        strcpy(globalProcessTable[pcount++].name, processName);
    }
}

void printExecutionDetails(Process *processTable) {
    int totalTurnaround = 0, totalWait = 0;

    printf("%-20s %-20s %-20s\n", "Name", "Turnaround", "Waiting");

    for (int i = 0; i < pcount; i++) {
        Process *process = &processTable[i]; // this process
        int turnaroundTime = process->end - process->arrival;
        int waitingTime = turnaroundTime - process->cpuburst;

        printf("%-20s %-20d %-20d\n", process->name, turnaroundTime,
               waitingTime);

        totalTurnaround += turnaroundTime;
        totalWait += waitingTime;
    }

    printf("Avg turnaround: %f, Avg wait: %f\n",
           (double)totalTurnaround / pcount, (double)totalWait / pcount);
}

// First come first serve
void FCFS() {
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

    printExecutionDetails(processTable);
    free(processTable);
}

// Round robin
void RR(int timeQuantum) {
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
                if (process->start == 0) {
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

    printExecutionDetails(processTable);
    free(processTable);
}

// // Select the process with shortest remaining execution time
// Process * getShortestProcess(Process *processTable) {
//     for (int i = 0; i < pcount; i++) {

//     }
// }

// // Shortest Remaining Burst First
// void SRBF(int timeQuantum) {
//     Process *processTable = (Process *)malloc(sizeof(Process) * pcount);
//     memcpy(processTable, globalProcessTable, sizeof(Process) * pcount);

//     int currentTime = 0;

//     printf("%-10s %-10s %-10s %-10s %-10s\n", "Name", "Arrival", "CPU Burst",
//            "Start", "End");

//     while (true) {

//     }

//     printExecutionDetails(processTable);
//     free(processTable);
// }

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Provide input file name\n");
        exit(1);
    }

    const char *infile = argv[1];
    printf("filename: %s\n", infile);

    readProcessTable(infile);

    FCFS();
    printf("\n");
    RR(2);
    printf("\n");

    return 0;
}
