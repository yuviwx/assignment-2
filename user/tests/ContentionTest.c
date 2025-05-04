#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define ITERATIONS 1000
volatile int counter = 0;

int main() {
    int lock_id = peterson_create();
    if (lock_id < 0) {
        printf("Failed to create lock\n");
        exit(1);
    }
    
    printf("Starting contention test\n");
    int pid = fork();
    int role = pid > 0 ? 0 : 1;
    
    for (int i = 0; i < ITERATIONS; i++) {
        if (peterson_acquire(lock_id, role) < 0) {
            printf("Failed to acquire lock\n");
            exit(1);
        }
        
        // Critical section - increment counter
        int temp = counter;
        sleep(0);  // Yield to increase chances of race condition
        counter = temp + 1;
        
        if (peterson_release(lock_id, role) < 0) {
            printf("Failed to release lock\n");
            exit(1);
        }
    }
    
    if (pid > 0) {
        wait(0);
        printf("Final counter value: %d (expected: %d)\n", counter, ITERATIONS * 2);
        if (counter == ITERATIONS * 2) {
            printf("Contention test PASSED\n");
        } else {
            printf("Contention test FAILED - race condition detected\n");
        }
        peterson_destroy(lock_id);
    }
    
    exit(0);
}