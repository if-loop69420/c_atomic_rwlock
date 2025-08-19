#ifndef C_ATOMIC_RWLOCK_H
#define C_ATOMIC_RWLOCK_H

#include <stdint.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_REF_COUNT (1ULL << 62) -1
#define READ_FLAG     1ULL << 62
#define WRITE_FLAG    1ULL << 63
#define COUNTER_MASK  (UINT_FAST64_MAX << 2) >> 2

typedef struct {
  atomic_uint_fast64_t flags;
} RwLock;

void init_rwlock(RwLock* lock);
void acquire_read(RwLock* lock);
void acquire_write(RwLock* lock);
void free_read(RwLock* lock);
void free_write(RwLock* lock);

#endif
