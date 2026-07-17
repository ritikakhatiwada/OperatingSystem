# ST5004CEM — Operating Systems and Security

A multi-threaded, secure file management and networked key-value system written in C,
demonstrating process management, memory management, file security and network
programming as one connected implementation rather than four separate demonstrations.

## What it demonstrates

| Area | Implementation | Files |
|------|----------------|-------|
| Process management | Multiple threads, mutex synchronisation, semaphores, round-robin scheduling and race condition prevention | task1_threading/ |
| Memory management | Paging system with virtual address translation, FIFO and LRU page replacement and hit/miss ratio tracking | task2_memory/ |
| File system and security | File CRUD, user authentication, Unix-style permissions, XOR encryption and hash-chained audit logging | task3_filesystem/ |
| Network programming | Multi-client TCP server, length-prefixed protocol, thread-per-client IPC and input validation | task4_network/ |

## Build and run

Requirements: GCC, make, Ubuntu 24.04 LTS, POSIX threads.

### Task 1 — Threading
gcc -o task1complete task1_threading/task1complete.c -lpthread
./task1complete

### Task 2 — Memory Management
gcc -o task2complete task2_memory/task2complete.c
./task2complete

### Task 3 — File System Security
gcc -o task3_complete task3_filesystem/task3_complete.c
./task3_complete

### Task 4 — Network Server and Client
gcc -o server task4_network/server.c -lpthread
gcc -o client task4_network/client.c

Terminal 1:
./server

Terminal 2:
./client

## Key results

| Test | Expected | Actual |
|------|----------|--------|
| 3 threads concurrent | Finish order differs from creation | Verified |
| Mutex race condition fix | Counter always 300,000 | 300,000 every run |
| Semaphore buffer control | Final buffer count = 0 | 0 confirmed |
| Round Robin scheduler | Avg wait 9.00, turnaround 14.00 | Confirmed |
| FIFO page replacement | 9 faults on 15-access string | 9 faults |
| LRU page replacement | 11 faults on 15-access string | 11 faults |
| File permission denial | Guest blocked from private files | Confirmed |
| Audit log | 20 entries recorded | Confirmed |
| Network authentication | Wrong password blocked | Confirmed |
| Multi-client server | Multiple clients handled | Confirmed |

## Users (Task 3 and Task 4)

| Username | Password | Role |
|----------|----------|------|
| admin | admin123 | Administrator — full access |
| ritika | pass456 | Regular user — owns files |
| guest | guest789 | Guest — read-only shared files |

## Network protocol (Task 4)

| Command | Format | Auth Required |
|---------|--------|---------------|
| LOGIN | LOGIN:username:password | No |
| MSG | MSG:message text | Yes |
| TIME | TIME: | Yes |
| ECHO | ECHO:text | Yes |
| QUIT | QUIT: | No |

## Repository layout

task1_threading/   thread creation, mutex, semaphore, round-robin scheduler
task2_memory/      paging system, FIFO and LRU page replacement
task3_filesystem/  file CRUD, authentication, permissions, encryption, audit log
task4_network/     TCP client-server with protocol and multi-client support

## Scope

This is a coursework-scale implementation built for Ubuntu 24.04 LTS.
XOR encryption is used for demonstration — production systems would use AES-256-GCM.
The audit log is append-only in memory — production systems would write to disk.
The network server uses TCP on port 8080 with a text-based protocol.

## Student

Name:    Ritika Khatiwada
Module:  ST5004CEM Operating Systems and Security
College: Softwarica College of IT and E-Commerce
University: Coventry University
