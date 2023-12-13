#include "spinlock_ece391.h"
#include "solution.h"
#include <bits/types.h>

// init the parameters 
ps_lock ps_lock_create(spinlock_t *lock) {
    // Fill this out!!!
    ps_lock res;
    res.mlock = lock;
    spinlock_init_ece391(res.mlock);
    res.room_num = 0;
    res.prof_num = 0;
    res.prof_waiting  = 0;
    res.stu_waiting = 0; 
    res.ta_waiting = 0; 

    return res;
}

void professor_enter(ps_lock *ps) {
    // Fill this out!!!
    spinlock_lock_ece391(ps->mlock);
    ps->prof_waiting += 1;
    spinlock_unlock_ece391(ps->mlock);
    while (1){                                                          // use a while loop to make sure while the professor is waiting, the function is still running to check the number of students and tas
        spinlock_lock_ece391(ps->mlock);


        if (ps->room_num < 20 && ps->room_num == ps->prof_num){         //make sure there's only professors in the rooms and less than 20
            ps->room_num += 1;
            ps->prof_num += 1;
            ps->prof_waiting-=1;
            spinlock_unlock_ece391(ps->mlock);
            return;
        }
        spinlock_unlock_ece391(ps->mlock);
    }
}

void professor_exit(ps_lock *ps) {
    // Fill this out!!!
    spinlock_lock_ece391(ps->mlock);
    if (ps->prof_num >= 1){                     
        ps->room_num -= 1;
        ps->prof_num -= 1;
    }
    spinlock_unlock_ece391(ps->mlock);
}

void ta_enter(ps_lock *ps) {
    // Fill this out!!!
    spinlock_lock_ece391(ps->mlock);
    ps->ta_waiting += 1;
    spinlock_unlock_ece391(ps->mlock);
    while (1){
        spinlock_lock_ece391(ps->mlock);


        if (ps->room_num < 20 && ps->prof_num == 0 && ps->prof_waiting==0){             //make sure no professor in room and no professor waiting since professors have the highest priority
            ps->room_num += 1;
            ps->ta_waiting-=1;
            spinlock_unlock_ece391(ps->mlock);
            return;
        }
        spinlock_unlock_ece391(ps->mlock);
    }
}

void ta_exit(ps_lock *ps) {
    // Fill this out!!!
    spinlock_lock_ece391(ps->mlock);
    if (ps->room_num >= 1){
        ps->room_num -= 1;
    }
    spinlock_unlock_ece391(ps->mlock);
}

void student_enter(ps_lock *ps) {
    // Fill this out!!!
    spinlock_lock_ece391(ps->mlock);
    ps->stu_waiting += 1;
    spinlock_unlock_ece391(ps->mlock);
    while (1){
        spinlock_lock_ece391(ps->mlock);

        if (ps->room_num < 20 && ps->prof_num == 0 && ps->prof_waiting==0 && ps->ta_waiting==0){        // the same as ta_enter
            ps->room_num += 1;
            ps->stu_waiting-=1;
            spinlock_unlock_ece391(ps->mlock);
            return;
        }
        spinlock_unlock_ece391(ps->mlock);
    }
}

void student_exit(ps_lock *ps) {
    // Fill this out!!!
    spinlock_lock_ece391(ps->mlock);
    if (ps->room_num >= 1){
        ps->room_num -= 1;
    }
    spinlock_unlock_ece391(ps->mlock);
}
