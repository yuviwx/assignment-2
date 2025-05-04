#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
    int lock1 = peterson_create();
    int lock2 = peterson_create();
    
    if (lock1 < 0 || lock2 < 0) {
        printf("Failed to create locks\n");
        exit(1);
    }
    
    printf("Created locks: %d and %d\n", lock1, lock2);
    
    int pid = fork();
    int role = pid > 0 ? 0 : 1;
    
    // Use both locks in sequence
    for (int i = 0; i < 10; i++) {
        // Use first lock
        if (peterson_acquire(lock1, role) < 0) {
            printf("Failed to acquire lock1\n");
            exit(1);
        }
        
        printf("Process %d using lock1\n", role);
        sleep(1); // Force some contention
        
        if (peterson_release(lock1, role) < 0) {
            printf("Failed to release lock1\n");
            exit(1);
        }
        
        // Use second lock
        if (peterson_acquire(lock2, role) < 0) {
            printf("Failed to acquire lock2\n");
            exit(1);
        }
        
        printf("Process %d using lock2\n", role);
        sleep(1);
        
        if (peterson_release(lock2, role) < 0) {
            printf("Failed to release lock2\n");
            exit(1);
        }
    }
    
    if (pid > 0) {
        wait(0);
        peterson_destroy(lock1);
        peterson_destroy(lock2);
        printf("Parent destroyed both locks\n");
    }
    
    exit(0);
}