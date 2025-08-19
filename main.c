// Previous program for 3 simultaneous counters
// #include "c_atomic_rwlock.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <sys/wait.h>
// #include <unistd.h>
// #include <sys/mman.h>

// int *counter;
// RwLock *lock;

// void count_up_1k(){
//   for(int i=0; i < 1000; i++) {
//     acquire_write(lock);
//     *counter +=1;
//     free_write(lock);
//   }
// }

// int main(int argc, char** argv) {
//   counter = mmap(NULL, sizeof(int),
//                                PROT_READ | PROT_WRITE,
//                                MAP_SHARED | MAP_ANONYMOUS, -1, 0);
//   *counter = 0;
//   lock = mmap(NULL, sizeof(RwLock), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
//   init_rwlock(lock);
//   pid_t pid1 = fork();
//   pid_t pid2 = fork();
//   if (pid1 < 0 || pid2 < 0) {
//     perror("fork failed");
//     return 1;
//   }
//   if(pid1 == 0 || pid2==0) {
//     count_up_1k();
//     exit(0);
//   }
//   waitpid(pid1,NULL,0);
//   waitpid(pid2,NULL,0);
//   acquire_read(lock);
//   printf("count: %i\n", *counter);
//   free_read(lock);
//   munmap(counter, sizeof(int));
//   munmap(lock, sizeof(RwLock));
// }
#include "c_atomic_rwlock.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>

int *counter;
RwLock *lock;

void count_up_1k() {
    for (int i = 0; i < 1000; i++) {
        acquire_write(lock);
        (*counter) += 1;
        free_write(lock);
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_processes>\n", argv[0]);
        return 1;
    }

    int num_procs = atoi(argv[1]);
    if (num_procs <= 0) {
        fprintf(stderr, "Number of processes must be positive.\n");
        return 1;
    }

    // Allocate shared memory
    counter = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (counter == MAP_FAILED) {
        perror("mmap failed for counter");
        return 1;
    }

    lock = mmap(NULL, sizeof(RwLock), PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (lock == MAP_FAILED) {
        perror("mmap failed for lock");
        return 1;
    }

    *counter = 0;
    init_rwlock(lock);

    // Fork multiple child processes
    for (int i = 0; i < num_procs; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            return 1;
        } else if (pid == 0) {
            // Child process
            count_up_1k();
            exit(0);
        }
    }

    // Parent waits for all children
    for (int i = 0; i < num_procs; i++) {
        wait(NULL);
    }

    acquire_read(lock);
    printf("Final count (should be %d): %d\n", num_procs * 1000, *counter);
    free_read(lock);

    munmap(counter, sizeof(int));
    munmap(lock, sizeof(RwLock));

    return 0;
}

