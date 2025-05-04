// Tournament tree mutual exclusion test using file-based shared counter
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define COUNTER_FILE "counter.txt"
#define ITERATIONS 5

// Convert integer to string
void itoa(int n, char *str) {
    int i = 0;
    int sign = 0;
    
    if (n < 0) {
        sign = 1;
        n = -n;
    }
    
    // Generate digits in reverse order
    do {
        str[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    
    if (sign) {
        str[i++] = '-';
    }
    
    str[i] = '\0';
    
    // Reverse the string
    int j, len = i;
    char temp;
    for (i = 0, j = len - 1; i < j; i++, j--) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
}

// Read counter value from file
int read_counter() {
    int fd = open(COUNTER_FILE, O_RDONLY);
    if (fd < 0) {
        return 0; // File doesn't exist yet
    }
    
    char buf[16];
    int n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    
    if (n <= 0) {
        return 0;
    }
    
    buf[n] = '\0';
    return atoi(buf);
}

// Write counter value to file
void write_counter(int value) {
    int fd = open(COUNTER_FILE, O_CREATE | O_RDWR);
    if (fd < 0) {
        fprintf(2, "Failed to open counter file\n");
        exit(1);
    }
    
    char buf[16];
    itoa(value, buf);
    
    // Add newline
    int len = strlen(buf);
    buf[len] = '\n';
    buf[len+1] = '\0';
    
    write(fd, buf, strlen(buf));
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(2, "Usage: tournament_test <num_processes>\n");
        exit(1);
    }

    int num_processes = atoi(argv[1]);
    
    // Check if num_processes is a power of 2 and <= 16
    if (num_processes <= 0 || num_processes > 16 || (num_processes & (num_processes - 1)) != 0) {
        fprintf(2, "Number of processes must be a power of 2 up to 16\n");
        exit(1);
    }

    // Initialize counter file
    if (num_processes == 1) {
        write_counter(0);
    }

    int id = tournament_create(num_processes);
    if (id < 0) {
        fprintf(2, "Failed to create tournament\n");
        exit(1);
    }

    // Each process increments the counter ITERATIONS times
    for (int i = 0; i < ITERATIONS; i++) {
        // Try to acquire the tournament lock
        if (tournament_acquire() < 0) {
            fprintf(2, "Process %d failed to acquire tournament lock\n", id);
            exit(1);
        }

        // Critical section begins
        int counter = read_counter();
        
        printf("Process %d: Read counter = %d\n", id, counter);
        
        // Sleep to increase the chance of race conditions if locking fails
        sleep(10);
        
        // Increment counter
        counter++;
        
        // Write the new value
        write_counter(counter);
        
        printf("Process %d: Wrote counter = %d\n", id, counter);
        // Critical section ends
        
        if (tournament_release() < 0) {
            fprintf(2, "Process %d failed to release tournament lock\n", id);
            exit(1);
        }
        
        // Give other processes a chance to run
        sleep(5);
    }

    // For parent process, wait for all children and check final counter value
    if (id == 0) {
        for (int i = 1; i < num_processes; i++) {
            wait(0);
        }
        
        int final_counter = read_counter();
        int expected = num_processes * ITERATIONS;
        
        printf("\n--- Test Results ---\n");
        printf("Final counter value: %d\n", final_counter);
        printf("Expected value: %d\n", expected);
        
        if (final_counter == expected) {
            printf("TEST PASSED: Mutual exclusion is working correctly!\n");
        } else {
            printf("TEST FAILED: Race conditions detected. Lock is not enforcing mutual exclusion.\n");
            printf("Some increments were lost due to improper synchronization.\n");
        }
    }

    exit(0);
}