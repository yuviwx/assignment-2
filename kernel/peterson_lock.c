#include "peterson_lock.h"
#include "proc.c"

int locksCreated = 0; 

 void peterson_init () { 
    for(int i=0; i<MAX_PETERSON_LOCKS; i++) {
       petersonLocks[i].active =0;
       petersonLocks[i].flags[0]=0;
       petersonLocks[i].flags[1]=0;
       petersonLocks[i].turn=0; 
    }
}

int peterson_create(void) {
    for(int i=0; i<MAX_PETERSON_LOCKS;i++){
        __sync_synchronize();
        if(__sync_lock_test_and_set(&petersonLocks[i].active, 1) == 0) {
            petersonLocks[i].flags[0]=0;
            petersonLocks[i].flags[1]=0;
            petersonLocks[i].turn=0; 
            __sync_synchronize();
            return i;
        }
    }
    return -1;
}

int peterson_acquire(int lock_id, int role){
    int other = 1 - role;

    // Check the correctness of the arguments
    if(lock_id < 0 || lock_id >= MAX_PETERSON_LOCKS || role < 0 || role > 1 || !petersonLocks[lock_id].active){
        return -1;
    }
    
    // Lock the lock..
    __sync_synchronize();
    __sync_lock_test_and_set(&petersonLocks[lock_id].flags[role], 1);
    petersonLocks[lock_id].turn = role;
    __sync_synchronize();

    // check if you were first
    while(petersonLocks[lock_id].flags[other] == 1 && petersonLocks[lock_id].turn == role){
        yield();
        __sync_synchronize();
    }
    return 0;
}

int peterson_release(int lock_id, int role){
     // Check the correctness of the arguments
     if(lock_id < 0 || lock_id >= MAX_PETERSON_LOCKS || role < 0 || role > 1 || !petersonLocks[lock_id].active){
        return -1;
    }
    __sync_lock_release(&petersonLocks[lock_id].flags[role]);
    __sync_synchronize();

    return 0;
}

int peterson_destroy(int lock_id) {

    if(lock_id < 0 || lock_id >= MAX_PETERSON_LOCKS || !petersonLocks[lock_id].active){
        return -1;
    }
    __sync_lock_release(&petersonLocks[lock_id].active);
    __sync_synchronize();
    return 0;
}