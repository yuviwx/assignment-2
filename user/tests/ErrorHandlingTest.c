#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/peterson_lock.h"

int main() {
    printf("Testing error conditions:\n");
    
    // Test using invalid lock ID
    if (peterson_acquire(-1, 0) >= 0) {
        printf("ERROR: Should fail with negative lock ID\n");
    } else {
        printf("OK: Rejected negative lock ID\n");
    }
    
    if (peterson_acquire(100, 0) >= 0) {
        printf("ERROR: Should fail with out-of-range lock ID\n");
    } else {
        printf("OK: Rejected out-of-range lock ID\n");
    }
    
    // Test using invalid role
    int lock_id = peterson_create();
    if (lock_id < 0) {
        printf("Failed to create lock\n");
        exit(1);
    }
    
    if (peterson_acquire(lock_id, -1) >= 0) {
        printf("ERROR: Should fail with negative role\n");
    } else {
        printf("OK: Rejected negative role\n");
    }
    
    if (peterson_acquire(lock_id, 2) >= 0) {
        printf("ERROR: Should fail with invalid role (>1)\n");
    } else {
        printf("OK: Rejected invalid role (>1)\n");
    }
    
    // Test destroy/reuse
    peterson_destroy(lock_id);
    
    if (peterson_acquire(lock_id, 0) >= 0) {
        printf("ERROR: Could acquire destroyed lock\n");
    } else {
        printf("OK: Could not acquire destroyed lock\n");
    }
    
    // Create and fill all possible locks
    int locks[MAX_PETERSON_LOCKS+1];
    int i;
    for (i = 0; i < MAX_PETERSON_LOCKS; i++) {
        locks[i] = peterson_create();
        if (locks[i] < 0) {
            printf("Failed to create lock %d\n", i);
            break;
        }
    }
    
    printf("Created %d locks successfully\n", i);
    
    // Try to create one more - should fail
    int extra = peterson_create();
    if (extra >= 0) {
        printf("ERROR: Created too many locks (%d)\n", extra);
    } else {
        printf("OK: Could not create more than MAX_PETERSON_LOCKS\n");
    }
    
    // Clean up
    for (int j = 0; j < i; j++) {
        peterson_destroy(locks[j]);
    }
    
    printf("Error handling tests complete\n");
    exit(0);
}