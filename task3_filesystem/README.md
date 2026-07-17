# Task 3 — File System Operations and Security

## Description
Secure file management system in C with user authentication, Unix-style file permissions, XOR encryption/decryption, and audit logging.

## Files
| File | Description |
|------|-------------|
| `fileops.c` | Layer 1 — Basic file create, read, write, delete |
| `auth_permissions.c` | Layer 2 — User authentication and file permissions |
| `task3_complete.c` | Final — Complete secure file system |

## Compile and Run
gcc -o task3_complete task3_complete.c
./task3_complete

## Users
| Username | Password | Role |
|----------|----------|------|
| admin | admin123 | Administrator |
| ritika | pass456 | Regular User |
| guest | guest789 | Guest |

## Requirements
- Ubuntu 24.04 LTS
- GCC compiler
