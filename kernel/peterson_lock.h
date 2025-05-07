#ifndef _PETERSON_LOCK_H
#define  _PETERSON_LOCK_H
// Peterson lock structure
typedef struct  {
  int flags[2];         // Interest flags for each process
  int turn;             // Whose turn is it to wait
  int active;
} PetersonLock;

#define MAX_PETERSON_LOCKS 16
extern PetersonLock petersonLocks[MAX_PETERSON_LOCKS];

void peterson_init ();
int peterson_create();
int peterson_acquire(int lock_id, int role);
int peterson_release(int lock_id, int role);
int peterson_destroy(int lock_id);

#endif

