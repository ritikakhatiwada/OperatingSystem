# Task 1 — Process Management and Threading

## Description
Multi-threaded application in C demonstrating thread creation, mutex synchronisation, semaphore producer-consumer, and Round Robin CPU scheduling simulation.

## Files
| File | Description |
|------|-------------|
| `thread_basic.c` | Layer 1 — Basic thread creation with 3 concurrent threads |
| `mutex_demo.c` | Layer 2 — Mutex and race condition demonstration |
| `semaphore_demo.c` | Layer 3 — Semaphore producer-consumer pattern |
| `round_robin.c` | Layer 4 — Round Robin scheduler simulation |
| `task1complete.c` | Final — All four concepts combined |

## Compile and Run
```bash
gcc -o task1complete task1complete.c -lpthread
./task1complete
```

## Requirements
- Ubuntu 24.04 LTS
- GCC compiler
- POSIX threads library (lpthread)
