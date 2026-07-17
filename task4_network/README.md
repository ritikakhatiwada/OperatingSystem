# Task 4 — Network Programming and IPC

## Description
TCP client-server application in C demonstrating socket programming, custom application protocol, multi-client handling with threads, authentication and error handling.

## Files
| File | Description |
|------|-------------|
| `server.c` | Multi-client TCP server with authentication |
| `client.c` | TCP client with full protocol demonstration |

## Compile and Run
gcc -o server server.c -lpthread
gcc -o client client.c

Terminal 1 — Start server
./server

Terminal 2 — Run client
./client

## Protocol
| Command | Format | Auth Required |
|---------|--------|---------------|
| LOGIN | LOGIN:username:password | No |
| MSG | MSG:message | Yes |
| TIME | TIME: | Yes |
| ECHO | ECHO:text | Yes |
| QUIT | QUIT: | No |

## Requirements
- Ubuntu 24.04 LTS
- GCC compiler
- POSIX threads library
- Port 8080 must be available
