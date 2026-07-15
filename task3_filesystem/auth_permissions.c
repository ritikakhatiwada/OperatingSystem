#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================
// TASK 3 LAYER 2: User Authentication and File Permissions
// Demonstrates: login system, permission checking (rwx)
// ============================================================

#define MAX_USERS    3
#define MAX_FILES    5
#define NAME_LEN     32
#define PASS_LEN     32

// Permission flags (like Unix rwx)
#define PERM_READ    4   // 100 in binary
#define PERM_WRITE   2   // 010 in binary
#define PERM_EXECUTE 1   // 001 in binary

// Structure representing a user
typedef struct {
    char username[NAME_LEN];
    char password[PASS_LEN];  // in real system this would be hashed
    int  user_id;
    int  is_admin;            // 1 = admin, 0 = regular user
} User;

// Structure representing a file with permissions
typedef struct {
    char filename[NAME_LEN];
    int  owner_id;            // user_id of owner
    int  owner_perms;         // permissions for owner
    int  other_perms;         // permissions for everyone else
    int  exists;              // 1 = file exists
} SecureFile;

// Global user database
User users[MAX_USERS] = {
    {"admin",   "admin123",  1, 1},   // admin user
    {"ritika",  "pass456",   2, 0},   // regular user
    {"guest",   "guest789",  3, 0},   // guest user
};

// Global file database
SecureFile files[MAX_FILES];
int file_count = 0;

// Currently logged in user (-1 = nobody)
int current_user_id = -1;

// ── AUTHENTICATION ────────────────────────────────────────────

// Simple password hash (XOR based - for demo purposes)
// In real systems use bcrypt or SHA-256
int simple_hash(char *password) {
    int hash = 0, i;
    for (i = 0; password[i] != '\0'; i++) {
        hash ^= (password[i] * (i + 1));
    }
    return hash;
}

// Authenticate user - returns user_id or -1 if failed
int login(char *username, char *password) {
    int i;
    for (i = 0; i < MAX_USERS; i++) {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password, password) == 0) {
            current_user_id = users[i].user_id;
            printf("Login successful! Welcome %s %s\n",
                   users[i].is_admin ? "[ADMIN]" : "[USER]",
                   username);
            return users[i].user_id;
        }
    }
    printf("Login FAILED: Invalid username or password\n");
    return -1;
}

// Logout current user
void logout() {
    if (current_user_id == -1) {
        printf("No user is logged in\n");
        return;
    }
    printf("User logged out successfully\n");
    current_user_id = -1;
}

// Get current user info
User *get_current_user() {
    int i;
    for (i = 0; i < MAX_USERS; i++) {
        if (users[i].user_id == current_user_id)
            return &users[i];
    }
    return NULL;
}

// ── PERMISSION CHECKING ───────────────────────────────────────

// Check if current user has permission for a file
int check_permission(SecureFile *file, int perm_needed) {
    User *user = get_current_user();
    if (user == NULL) {
        printf("ERROR: No user logged in\n");
        return 0;
    }

    // Admin has all permissions
    if (user->is_admin) return 1;

    // Owner gets owner permissions
    if (file->owner_id == current_user_id) {
        return (file->owner_perms & perm_needed) != 0;
    }

    // Everyone else gets other permissions
    return (file->other_perms & perm_needed) != 0;
}

// Display permissions in rwx format
void display_perms(int perms) {
    printf("%c%c%c",
           (perms & PERM_READ)    ? 'r' : '-',
           (perms & PERM_WRITE)   ? 'w' : '-',
           (perms & PERM_EXECUTE) ? 'x' : '-');
}

// ── FILE OPERATIONS WITH PERMISSIONS ─────────────────────────

// Create a file with permissions
int secure_create(char *filename, int owner_perms, int other_perms) {
    if (current_user_id == -1) {
        printf("ERROR: Must be logged in to create files\n");
        return 0;
    }
    if (file_count >= MAX_FILES) {
        printf("ERROR: Maximum file limit reached\n");
        return 0;
    }

    // Add to file database
    strcpy(files[file_count].filename,  filename);
    files[file_count].owner_id     = current_user_id;
    files[file_count].owner_perms  = owner_perms;
    files[file_count].other_perms  = other_perms;
    files[file_count].exists       = 1;
    file_count++;

    printf("File created: %s | Owner perms: ", filename);
    display_perms(owner_perms);
    printf(" | Other perms: ");
    display_perms(other_perms);
    printf("\n");
    return 1;
}

// Read a file - checks read permission
int secure_read(char *filename) {
    int i;
    for (i = 0; i < file_count; i++) {
        if (strcmp(files[i].filename, filename) == 0 && files[i].exists) {
            if (check_permission(&files[i], PERM_READ)) {
                printf("READ OK: Accessing %s\n", filename);
                return 1;
            } else {
                printf("PERMISSION DENIED: Cannot read %s\n", filename);
                return 0;
            }
        }
    }
    printf("ERROR: File %s not found\n", filename);
    return 0;
}

// Write to a file - checks write permission
int secure_write(char *filename) {
    int i;
    for (i = 0; i < file_count; i++) {
        if (strcmp(files[i].filename, filename) == 0 && files[i].exists) {
            if (check_permission(&files[i], PERM_WRITE)) {
                printf("WRITE OK: Writing to %s\n", filename);
                return 1;
            } else {
                printf("PERMISSION DENIED: Cannot write to %s\n", filename);
                return 0;
            }
        }
    }
    printf("ERROR: File %s not found\n", filename);
    return 0;
}

// Display all files and their permissions
void list_files() {
    int i;
    printf("\n  Files in system:\n");
    printf("  +-----------------------+-------+-------+--------+\n");
    printf("  | Filename              | Owner | Perms | Other  |\n");
    printf("  +-----------------------+-------+-------+--------+\n");
    for (i = 0; i < file_count; i++) {
        if (files[i].exists) {
            printf("  | %-21s | %5d | ", files[i].filename, files[i].owner_id);
            display_perms(files[i].owner_perms);
            printf("   | ");
            display_perms(files[i].other_perms);
            printf("    |\n");
        }
    }
    printf("  +-----------------------+-------+-------+--------+\n");
}

// ── MAIN ──────────────────────────────────────────────────────

int main() {
    printf("========================================\n");
    printf("  Task 3 Layer 2: Auth and Permissions\n");
    printf("========================================\n\n");

    // Test wrong login
    printf("--- Testing wrong login ---\n");
    login("ritika", "wrongpass");

    // Login as ritika
    printf("\n--- Login as ritika ---\n");
    login("ritika", "pass456");

    // Create files with different permissions
    printf("\n--- Creating files ---\n");
    // Owner: rwx, Others: r--
    secure_create("myfile.txt",   PERM_READ|PERM_WRITE|PERM_EXECUTE, PERM_READ);
    // Owner: rw-, Others: --- (private)
    secure_create("private.txt",  PERM_READ|PERM_WRITE, 0);

    list_files();

    // Test reading own file
    printf("\n--- Ritika reads her own files ---\n");
    secure_read("myfile.txt");
    secure_read("private.txt");

    // Logout and login as guest
    printf("\n--- Switching to guest user ---\n");
    logout();
    login("guest", "guest789");

    // Guest tries to access ritika's files
    printf("\n--- Guest tries to access ritika's files ---\n");
    secure_read("myfile.txt");    // should work (other has read)
    secure_read("private.txt");   // should fail (other has no perms)
    secure_write("myfile.txt");   // should fail (other has no write)

    // Login as admin
    printf("\n--- Admin login ---\n");
    logout();
    login("admin", "admin123");

    // Admin can access everything
    printf("\n--- Admin accesses all files ---\n");
    secure_read("private.txt");   // admin bypasses all permissions
    secure_write("private.txt");  // admin can write too

    logout();

    printf("\n========================================\n");
    printf("  Layer 2 Complete!\n");
    printf("========================================\n");

    return 0;
}
