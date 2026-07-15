#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// ============================================================
// TASK 4: Network Client
// Student: Ritika Khatiwada
// Description: TCP client that connects to server and
//              demonstrates the protocol with auto-test
// ============================================================

#define PORT        8080
#define SERVER_IP   "127.0.0.1"
#define BUFFER_SIZE 1024

// Send a command and print the response
void send_command(int sock, char *command) {
    char buffer[BUFFER_SIZE];
    int  bytes;

    printf("CLIENT -> %s", command);

    // Send command to server
    send(sock, command, strlen(command), 0);

    // Receive response
    memset(buffer, 0, BUFFER_SIZE);
    bytes = recv(sock, buffer, BUFFER_SIZE-1, 0);
    if (bytes > 0) {
        printf("SERVER -> %s", buffer);
    }
    printf("\n");
}

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    printf("=============================================\n");
    printf("  ST5004CEM Task 4: Network Client\n");
    printf("  Student: Ritika Khatiwada\n");
    printf("  Connecting to %s:%d\n", SERVER_IP, PORT);
    printf("=============================================\n\n");

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("ERROR: Socket creation failed");
        exit(1);
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0) {
        perror("ERROR: Connection failed");
        exit(1);
    }

    // Receive welcome message
    memset(buffer, 0, BUFFER_SIZE);
    recv(sock, buffer, BUFFER_SIZE-1, 0);
    printf("SERVER -> %s\n", buffer);

    // ── DEMO: AUTO TEST SEQUENCE ──────────────────────────────
    printf("--- Testing wrong login ---\n");
    send_command(sock, "LOGIN:ritika:wrongpass\n");

    printf("--- Testing correct login ---\n");
    send_command(sock, "LOGIN:ritika:pass456\n");

    printf("--- Testing commands without login (should fail) ---\n");

    printf("--- Sending a message ---\n");
    send_command(sock, "MSG:Hello from client!\n");

    printf("--- Requesting server time ---\n");
    send_command(sock, "TIME:\n");

    printf("--- Testing echo ---\n");
    send_command(sock, "ECHO:This is an echo test\n");

    printf("--- Testing unknown command ---\n");
    send_command(sock, "INVALID:test\n");

    printf("--- Disconnecting ---\n");
    send_command(sock, "QUIT:\n");

    close(sock);
    printf("Client disconnected.\n");

    return 0;
}
