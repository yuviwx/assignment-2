// Test program for tournament synchronization
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(2, "Usage: tournament <num_processes>\n");
        exit(1);
    }

    int num_processes = atoi(argv[1]);
    
    // Check if num_processes is a power of 2 and <= 16
    if (num_processes <= 0 || num_processes > 16 || (num_processes & (num_processes - 1)) != 0) {
        fprintf(2, "Number of processes must be a power of 2 up to 16\n");
        exit(1);
    }

    int id = tournament_create(num_processes);
    if (id < 0) {
        fprintf(2, "Failed to create tournament\n");
        exit(1);
    }

    // Number of times each process will try to acquire the lock
    int iterations = 3;
    
    for (int i = 0; i < iterations; i++) {
        // Try to acquire the tournament lock
        if (tournament_acquire() < 0) {
            fprintf(2, "Failed to acquire tournament lock\n");
            exit(1);
        }

        // Critical section
        printf("Iteration %d: Process with PID %d and tournament ID %d acquired the lock\n", 
               i+1, getpid(), id);
        
        // Sleep a bit to demonstrate mutual exclusion
        sleep(1);

        // Release the lock
        if (tournament_release() < 0) {
            fprintf(2, "Failed to release tournament lock\n");
            exit(1);
        }
        
        // Wait a moment before trying again to give other processes a chance
        sleep(1);
    }

    // If this is the parent process (id 0), wait for all children
    if (id == 0) {
        for (int i = 1; i < num_processes; i++) {
            wait(0);
        }
        printf("All processes have completed\n");
    }

    exit(0);
}