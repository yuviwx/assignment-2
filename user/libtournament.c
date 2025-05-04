#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/peterson_lock.h"

// Global variables for the tournament tree
static int proc_id = 0;  // ID of the current process in the tournament
static int num_processes = 0;   // Number of processes in the tournament
static int num_levels = 0;      // Number of levels in the tournament tree
static int *lock_ids = 0;       // Array of Peterson lock IDs

// Helper functions to calculate lock indices and roles
int calculate_role(int level) {
    int shift = num_levels - level - 1;
    return (proc_id & (1 << shift)) >> shift;
}

int calculate_lock_index(int level) {
    int lock_level_idx = proc_id >> (num_levels - level);
    return lock_level_idx + (1 << level) - 1;
}

int calculate_the_levels() {
    int levels = 0;
    for(int i=num_processes; i>1; i >>= 1)
        levels++;
    return levels;
}

int tournament_create(int processes) {
     // Check if the number of processes is valid (power of 2 up to 16)
     if (processes <= 0 || processes > 16 || (processes & (processes - 1)) != 0) {
        return -1;  // Not a power of 2 or out of range
    }

    //initialize locks
    num_processes=processes;
    lock_ids= malloc(sizeof(int)*(num_processes-1));
    if (!lock_ids) {
        return -1;  // Memory allocation failed
    }

    for(int i=0; i<processes-1; i++) {
        lock_ids[i] = peterson_create();
        if (lock_ids[i] < 0) {
            // Failed to create a lock, cleanup and return error
            for (int j = 0; j < i; j++) {
                peterson_destroy(lock_ids[j]);
            }
            free(lock_ids);
            lock_ids = 0;
            return -1;
        }
    }

    // Fork processes
    num_levels = calculate_the_levels();
    for(int i=1; i<processes; i++) {
        int pid = fork();
        if(pid < 0) {
            printf("fork failed!\n");
            return -1;
        }
        if (pid == 0) {
            proc_id = i;
            return proc_id;
        }
    }
    return proc_id;
}

int tournament_acquire(void) {
    if (num_processes == 0 || num_levels == 0 || lock_ids == 0) {
        return -1;  // Tournament not initialized
    }
    int node = proc_id, role;
    for(int i = num_levels-1; i >= 0; i--) {
        role = calculate_role(i);
        node = calculate_lock_index(i);
        if (peterson_acquire(node, role) < 0) {
            // If we fail to acquire a lock, release all previously acquired locks
            printf("failed to acquire: %d \n", proc_id);
            for(int j = i; j < num_levels; j++) {
                role = calculate_role(j);
                node = calculate_lock_index(j);
                if(peterson_release(node, role) <0) {
                    return -1;
                }
            }
            return -1;
        }
    }
    return 0;   
}

int tournament_release(void) {
    int node = proc_id, role; 
    for(int i = 0; i < num_levels; i++) {
        role = calculate_role(i);
        node = calculate_lock_index(i);
        if(peterson_release(node, role) <0) {
            return -1;
        }
    }
    return 0;
}

