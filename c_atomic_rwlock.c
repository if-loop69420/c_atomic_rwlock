#include "./c_atomic_rwlock.h"
#include <stdatomic.h>
#include <stdint.h>

void init_rwlock(RwLock* lock) {
  atomic_init(&lock->flags, 0);
}

// Set read flag
// Wait for write flag to disappear.
// increment counter if counter is below MAX_REF_COUNT
void acquire_read(RwLock* lock) {
  uint64_t old_flags, new_flags;
  while (1) {
    old_flags = atomic_load(&lock->flags);

    if ((old_flags & WRITE_FLAG) != 0) continue;  // Writer is active

    uint64_t ref_count = old_flags & COUNTER_MASK;
    if (ref_count >= MAX_REF_COUNT) continue;

    new_flags = old_flags + 1;

    if (atomic_compare_exchange_weak(&lock->flags, &old_flags, new_flags)) break;
  }
}


// Set write flag
// Wait for read flag to disappear and ref counter to hit 0
void acquire_write(RwLock* lock) {
  uint_fast64_t old_flags, new_flags;

  // First, try to atomically set WRITE_FLAG (without disturbing other bits)
  while (1) {
    old_flags = atomic_load(&lock->flags);
    if ((old_flags & WRITE_FLAG) != 0) continue;  // Another writer owns it

    new_flags = old_flags | WRITE_FLAG;

    if (atomic_compare_exchange_weak(&lock->flags, &old_flags, new_flags)) {
      break;  // Successfully acquired write flag
    }
    // Retry if CAS failed
  }

  // Wait for readers to drain (ref_count == 0)
  do {
    old_flags = atomic_load(&lock->flags);
  } while ((old_flags & COUNTER_MASK) != 0);
}

// Unset read flag
void free_read(RwLock* lock) {
  uint64_t old_flags, new_flags;
  while (1) {
    old_flags = atomic_load(&lock->flags);
    uint64_t ref_count = old_flags & COUNTER_MASK;
    if (ref_count == 0) {
      // Should never happen
      return;
    }

    new_flags = old_flags - 1;

    if (atomic_compare_exchange_weak(&lock->flags, &old_flags, new_flags)) break;
  }
}

// Subtract 1 from ref counter (if it's > 0)
// If ref counter is 0 then unset read flag
void free_write(RwLock* lock) {
  uint64_t old_flags, new_flags;
  while (1) {
    old_flags = atomic_load(&lock->flags);
    new_flags = old_flags & ~WRITE_FLAG;
    if (atomic_compare_exchange_weak(&lock->flags, &old_flags, new_flags)) break;
  }
}
