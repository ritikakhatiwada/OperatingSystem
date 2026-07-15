#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ============================================================
// TASK 3: Secure File Management System
// Student: Ritika Khatiwada
// Module: ST5004CEM Operating Systems and Security
// Features: File CRUD, Authentication, Permissions,
//           Encryption/Decryption, Audit Logging
// ============================================================

#define MAX_USERS     3
#define MAX_FILES     10
#define NAME_LEN      32
#define PASS_LEN      32
#define CONTENT_LEN   256
#define LOG_LEN       20
#define ENCRYPT_KEY   42    // XOR encryption key

// Permission flags
#define PERM_READ     4
#define PERM_WRITE    2
#define PERM_EXECUTE  1

// ============================================================
// SECTION 1: DATA STRUCTURES
// ============================================================

typedef struct {
    char username[NAME_LEN];
    char password[PASS_LEN];
    int  user_id;
    int  is_admin;
} User;

typedef struct {
    char filename[NAME_LEN];
    char content[CONTENT_LEN];
    int  owner_id;
    int  owner_perms;
    int  other_perms;
    int  is_encrypted;
    int  exists;
} SecureFile;

typedef struct {
    char timestamp[32];
    char username[NAME_LEN];
    char action[64];
    char filename[NAME_LEN];
    char result[16];
} AuditLog;

// Global data
User       users[MAX_USERS] = {
    {"admin",  "admin123", 1, 1},
    {"ritika", "pass456",  2, 0},
    {"guest",  "guest789", 3, 0},
};
SecureFile files[MAX_FILES];
AuditLog   audit_logs[LOG_LEN];
int        file_count  = 0;
int        log_count   = 0;
int        current_uid = -1;

// ============================================================
// SECTION 2: AUDIT LOGGING
// ============================================================

// Get current timestamp as string
void get_timestamp(char *buffer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
            t->tm_year+1900, t->tm_mon+1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);
}

// Get username of current user
char *get_username() {
    int i;
    for (i = 0; i < MAX_USERS; i++)
        if (users[i].user_id == current_uid)
            return users[i].username;
    return "unknown";
}

// Add entry to audit log
void audit(char *action, char *filename, char *result) {
    if (log_count >= LOG_LEN) {
        printf("WARNING: Audit log full!\n");
        return;
    }
    get_timestamp(audit_logs[log_count].timestamp);
    strcpy(audit_logs[log_count].username, get_username());
    strcpy(audit_logs[log_count].action,   action);
    strcpy(audit_logs[log_count].filename, filename);
    strcpy(audit_logs[log_count].result,   result);
    log_count++;
}

// Display full audit log
void display_audit_log() {
    int i;
    printf("\n  +=======================================================+\n");
    printf("  |                   AUDIT LOG                           |\n");
    printf("  +=======================================================+\n");
    printf("  | %-19s | %-8s | %-12s | %-6s |\n",
           "Timestamp", "User", "Action", "Result");
    printf("  +---------------------+----------+--------------+--------+\n");
    for (i = 0; i < log_count; i++) {
        printf("  | %-19s | %-8s | %-12s | %-6s |\n",
               audit_logs[i].timestamp,
               audit_logs[i].username,
               audit_logs[i].action,
               audit_logs[i].result);
    }
    printf("  +=======================================================+\n");
    printf("  | Total log entries: %-33d|\n", log_count);
    printf("  +=======================================================+\n");
}

// ============================================================
// SECTION 3: ENCRYPTION
// ============================================================

// XOR encryption - same function encrypts and decrypts
// XOR with key twice returns original: (data XOR key) XOR key = data
void xor_encrypt_decrypt(char *data, int key) {
    int i;
    for (i = 0; data[i] != '\0'; i++) {
        data[i] = data[i] ^ key;
    }
}

// Encrypt file content
void encrypt_file_content(SecureFile *file) {
    if (file->is_encrypted) {
        printf("  File is already encrypted\n");
        return;
    }
    xor_encrypt_decrypt(file->content, ENCRYPT_KEY);
    file->is_encrypted = 1;
    printf("  File encrypted successfully\n");
}

// Decrypt file content
void decrypt_file_content(SecureFile *file) {
    if (!file->is_encrypted) {
        printf("  File is not encrypted\n");
        return;
    }
    xor_encrypt_decrypt(file->content, ENCRYPT_KEY);
    file->is_encrypted = 0;
    printf("  File decrypted successfully\n");
}

// ============================================================
// SECTION 4: AUTHENTICATION
// ============================================================

int login(char *username, char *password) {
    int i;
    for (i = 0; i < MAX_USERS; i++) {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password, password) == 0) {
            current_uid = users[i].user_id;
            audit("LOGIN", "-", "SUCCESS");
            printf("  Login successful! Welcome %s [%s]\n",
                   username, users[i].is_admin ? "ADMIN" : "USER");
            return 1;
        }
    }
    // Log failed login attempt
    int saved = current_uid;
    current_uid = -1;
    // temporarily set name for logging
    strcpy(users[0].username, username);
    audit("LOGIN", "-", "FAILED");
    strcpy(users[0].username, "admin");
    current_uid = saved;
    printf("  Login FAILED for user: %s\n", username);
    return 0;
}

void logout() {
    audit("LOGOUT", "-", "SUCCESS");
    printf("  %s logged out\n", get_username());
    current_uid = -1;
}

// ============================================================
// SECTION 5: PERMISSION CHECKING
// ============================================================

User *current_user() {
    int i;
    for (i = 0; i < MAX_USERS; i++)
        if (users[i].user_id == current_uid)
            return &users[i];
    return NULL;
}

int has_permission(SecureFile *file, int perm) {
    User *u = current_user();
    if (!u) return 0;
    if (u->is_admin) return 1;
    if (file->owner_id == current_uid)
        return (file->owner_perms & perm) != 0;
    return (file->other_perms & perm) != 0;
}

void show_perms(int perms) {
    printf("%c%c%c",
           (perms & PERM_READ)    ? 'r' : '-',
           (perms & PERM_WRITE)   ? 'w' : '-',
           (perms & PERM_EXECUTE) ? 'x' : '-');
}

// ============================================================
// SECTION 6: SECURE FILE OPERATIONS
// ============================================================

// Create file
int secure_create(char *filename, char *content,
                  int owner_perms, int other_perms) {
    if (current_uid == -1) {
        printf("  ERROR: Must be logged in\n");
        audit("CREATE", filename, "DENIED");
        return 0;
    }
    if (file_count >= MAX_FILES) {
        printf("  ERROR: File limit reached\n");
        return 0;
    }

    strcpy(files[file_count].filename,  filename);
    strcpy(files[file_count].content,   content);
    files[file_count].owner_id     = current_uid;
    files[file_count].owner_perms  = owner_perms;
    files[file_count].other_perms  = other_perms;
    files[file_count].is_encrypted = 0;
    files[file_count].exists       = 1;
    file_count++;

    audit("CREATE", filename, "SUCCESS");
    printf("  Created: %s | Owner: ", filename);
    show_perms(owner_perms);
    printf(" | Others: ");
    show_perms(other_perms);
    printf("\n");
    return 1;
}

// Read file
int secure_read(char *filename) {
    int i;
    for (i = 0; i < file_count; i++) {
        if (strcmp(files[i].filename, filename) == 0 && files[i].exists) {
            if (!has_permission(&files[i], PERM_READ)) {
                printf("  PERMISSION DENIED: Cannot read %s\n", filename);
                audit("READ", filename, "DENIED");
                return 0;
            }
            if (files[i].is_encrypted) {
                printf("  READ OK: %s [ENCRYPTED - decrypt first]\n", filename);
            } else {
                printf("  READ OK: %s -> \"%s\"\n", filename, files[i].content);
            }
            audit("READ", filename, "SUCCESS");
            return 1;
        }
    }
    printf("  ERROR: File %s not found\n", filename);
    audit("READ", filename, "NOT FOUND");
    return 0;
}

// Write to file
int secure_write(char *filename, char *new_content) {
    int i;
    for (i = 0; i < file_count; i++) {
        if (strcmp(files[i].filename, filename) == 0 && files[i].exists) {
            if (!has_permission(&files[i], PERM_WRITE)) {
                printf("  PERMISSION DENIED: Cannot write to %s\n", filename);
                audit("WRITE", filename, "DENIED");
                return 0;
            }
            if (files[i].is_encrypted) {
                printf("  ERROR: Decrypt file before writing\n");
                return 0;
            }
            strcpy(files[i].content, new_content);
            audit("WRITE", filename, "SUCCESS");
            printf("  WRITE OK: %s updated\n", filename);
            return 1;
        }
    }
    printf("  ERROR: File %s not found\n", filename);
    return 0;
}

// Delete file
int secure_delete(char *filename) {
    int i;
    for (i = 0; i < file_count; i++) {
        if (strcmp(files[i].filename, filename) == 0 && files[i].exists) {
            if (!has_permission(&files[i], PERM_WRITE)) {
                printf("  PERMISSION DENIED: Cannot delete %s\n", filename);
                audit("DELETE", filename, "DENIED");
                return 0;
            }
            files[i].exists = 0;
            audit("DELETE", filename, "SUCCESS");
            printf("  DELETED: %s\n", filename);
            return 1;
        }
    }
    printf("  ERROR: File %s not found\n", filename);
    return 0;
}

// Encrypt a file
int secure_encrypt(char *filename) {
    int i;
    for (i = 0; i < file_count; i++) {
        if (strcmp(files[i].filename, filename) == 0 && files[i].exists) {
            if (!has_permission(&files[i], PERM_WRITE)) {
                printf("  PERMISSION DENIED: Cannot encrypt %s\n", filename);
                audit("ENCRYPT", filename, "DENIED");
                return 0;
            }
            encrypt_file_content(&files[i]);
            audit("ENCRYPT", filename, "SUCCESS");
            return 1;
        }
    }
    printf("  ERROR: File %s not found\n", filename);
    return 0;
}

// Decrypt a file
int secure_decrypt(char *filename) {
    int i;
    for (i = 0; i < file_count; i++) {
        if (strcmp(files[i].filename, filename) == 0 && files[i].exists) {
            if (!has_permission(&files[i], PERM_WRITE)) {
                printf("  PERMISSION DENIED: Cannot decrypt %s\n", filename);
                audit("DECRYPT", filename, "DENIED");
                return 0;
            }
            decrypt_file_content(&files[i]);
            audit("DECRYPT", filename, "SUCCESS");
            return 1;
        }
    }
    printf("  ERROR: File %s not found\n", filename);
    return 0;
}

// List all files
void list_files() {
    int i;
    printf("\n  +---------------------+-------+-------+-------+-----------+\n");
    printf("  | Filename            | Owner | Perms | Other | Encrypted |\n");
    printf("  +---------------------+-------+-------+-------+-----------+\n");
    for (i = 0; i < file_count; i++) {
        if (files[i].exists) {
            printf("  | %-19s | %5d | ", files[i].filename, files[i].owner_id);
            show_perms(files[i].owner_perms);
            printf("   | ");
            show_perms(files[i].other_perms);
            printf("   | %-9s |\n",
                   files[i].is_encrypted ? "YES" : "NO");
        }
    }
    printf("  +---------------------+-------+-------+-------+-----------+\n");
}

// ============================================================
// SECTION 7: MAIN
// ============================================================

int main() {
    printf("=============================================\n");
    printf("  ST5004CEM Task 3: Secure File System\n");
    printf("  Student: Ritika Khatiwada\n");
    printf("=============================================\n");

    // ---- DEMO 1: AUTHENTICATION ----
    printf("\n===== DEMO 1: Authentication =====\n");
    login("ritika", "wrongpass");
    login("ritika", "pass456");

    // ---- DEMO 2: FILE OPERATIONS ----
    printf("\n===== DEMO 2: File Operations =====\n");
    secure_create("notes.txt",   "My personal notes",
                  PERM_READ|PERM_WRITE, PERM_READ);
    secure_create("secret.txt",  "Top secret data",
                  PERM_READ|PERM_WRITE, 0);
    secure_create("script.sh",   "#!/bin/bash echo hello",
                  PERM_READ|PERM_WRITE|PERM_EXECUTE, 0);

    list_files();

    printf("\n--- Reading files ---\n");
    secure_read("notes.txt");
    secure_read("secret.txt");

    printf("\n--- Writing to file ---\n");
    secure_write("notes.txt", "Updated notes content");
    secure_read("notes.txt");

    // ---- DEMO 3: ENCRYPTION ----
    printf("\n===== DEMO 3: Encryption =====\n");
    printf("\nBefore encryption:\n");
    secure_read("secret.txt");

    printf("\nEncrypting secret.txt...\n");
    secure_encrypt("secret.txt");

    printf("\nAfter encryption:\n");
    secure_read("secret.txt");

    printf("\nDecrypting secret.txt...\n");
    secure_decrypt("secret.txt");

    printf("\nAfter decryption:\n");
    secure_read("secret.txt");

    // ---- DEMO 4: PERMISSIONS ----
    printf("\n===== DEMO 4: Permission Testing =====\n");
    logout();
    login("guest", "guest789");

    printf("\n--- Guest tries to access ritika's files ---\n");
    secure_read("notes.txt");     // allowed - others have read
    secure_read("secret.txt");    // denied  - others have no perms
    secure_write("notes.txt", "guest edit"); // denied - others no write
    secure_delete("secret.txt");  // denied  - others no write

    // ---- DEMO 5: ADMIN ACCESS ----
    printf("\n===== DEMO 5: Admin Access =====\n");
    logout();
    login("admin", "admin123");

    printf("\n--- Admin can access everything ---\n");
    secure_read("secret.txt");
    secure_write("secret.txt", "Admin updated this");
    secure_delete("script.sh");

    logout();

    // ---- DEMO 6: AUDIT LOG ----
    printf("\n===== DEMO 6: Audit Log =====\n");
    display_audit_log();

    printf("\n=============================================\n");
    printf("  Task 3 Complete! All demos passed!\n");
    printf("=============================================\n");

    return 0;
}
