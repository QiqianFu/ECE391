#include "spinlock_ece391.h"

// Initialize a spinlock
void spinlock_init_ece391(spinlock_t *lock) {
    pthread_mutex_init(&(lock->mutex), NULL);
}

// Acquire the spinlock
void spinlock_lock_ece391(spinlock_t *lock) {
    pthread_mutex_lock(&(lock->mutex));
}

// Release the spinlock
void spinlock_unlock_ece391(spinlock_t *lock) {
    pthread_mutex_unlock(&(lock->mutex));
}

// Destroy the spinlock
void spinlock_destroy_ece391(spinlock_t *lock) {
    pthread_mutex_destroy(&(lock->mutex));
}