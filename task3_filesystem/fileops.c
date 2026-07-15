#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================
// TASK 3 LAYER 1: Basic File Operations
// Student: Ritika Khatiwada
// Demonstrates: create, write, read, delete files
// ============================================================

// Create a new file and write content to it
int create_file(char *filename, char *content) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("ERROR: Could not create file %s\n", filename);
        return 0;
    }
    fprintf(fp, "%s", content);
    fclose(fp);
    printf("File created: %s\n", filename);
    return 1;
}

// Read and display contents of a file
int read_file(char *filename) {
    char buffer[1024];
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("ERROR: Could not open file %s\n", filename);
        return 0;
    }
    printf("Contents of %s:\n", filename);
    printf("--- START ---\n");
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
    }
    printf("--- END ---\n");
    fclose(fp);
    return 1;
}

// Append content to existing file
int write_file(char *filename, char *content) {
    FILE *fp = fopen(filename, "a");
    if (fp == NULL) {
        printf("ERROR: Could not open file %s\n", filename);
        return 0;
    }
    fprintf(fp, "%s", content);
    fclose(fp);
    printf("Content appended to: %s\n", filename);
    return 1;
}

// Delete a file
int delete_file(char *filename) {
    if (remove(filename) == 0) {
        printf("File deleted: %s\n", filename);
        return 1;
    } else {
        printf("ERROR: Could not delete file %s\n", filename);
        return 0;
    }
}

int main() {
    printf("========================================\n");
    printf("  Task 3 Layer 1: File Operations\n");
    printf("========================================\n\n");

    // Create a file
    printf("--- Creating file ---\n");
    create_file("test.txt", "Hello, this is line 1\n");

    // Read the file
    printf("\n--- Reading file ---\n");
    read_file("test.txt");

    // Append to the file
    printf("\n--- Appending to file ---\n");
    write_file("test.txt", "This is line 2\n");
    write_file("test.txt", "This is line 3\n");

    // Read again to see changes
    printf("\n--- Reading updated file ---\n");
    read_file("test.txt");

    // Delete the file
    printf("\n--- Deleting file ---\n");
    delete_file("test.txt");

    // Try to read deleted file
    printf("\n--- Trying to read deleted file ---\n");
    read_file("test.txt");

    printf("\n========================================\n");
    printf("  Layer 1 Complete!\n");
    printf("========================================\n");

    return 0;
}
