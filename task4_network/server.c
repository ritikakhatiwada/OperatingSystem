#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

// ============================================================
// TASK 4: Network Server
// Student: Ritika Khatiwada
// Module: ST5004CEM Operating Systems and Security
// Description: Multi-client TCP server with authentication,
//              data validation and proper error handling
// ============================================================

#define PORT         8080
#define MAX_CLIENTS  5
#define BUFFER_SIZE  1024
#define MAX_USERS    3

// Simple protocol message format:
// CMD:PAYLOAD
// Commands: LOGIN, MSG, TIME, ECHO, QUIT

// User database for authentication
typedef struct {
    char username[32];
    char password[32];
} User;

User valid_users[MAX_USERS] = {
    {"admin",  "admin123"},
    {"ritika", "pass456"},
    {"guest",  "guest789"},
};

// Client connection info
typedef struct {
    int  socket_fd;
    int  client_id;
    char username[32];
    int  is_authenticated;
    char ip_address[16];
} ClientInfo;

// Active client count
int active_clients = 0;
pthread_mutex_t client_mutex;

// ── LOGGING ──────────────────────────────────────────────────
void server_log(char *level, char *message) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf("[%02d:%02d:%02d] [%s] %s\n",
           t->tm_hour, t->tm_min, t->tm_sec, level, message);
}

// ── AUTHENTICATION ────────────────────────────────────────────
int authenticate(char *username, char *password) {
    int i;
    for (i = 0; i < MAX_USERS; i++) {
        if (strcmp(valid_users[i].username, username) == 0 &&
            strcmp(valid_users[i].password, password) == 0) {
            return 1;
        }
    }
    return 0;
}

// ── DATA VALIDATION ───────────────────────────────────────────
// Check message only contains printable characters
int validate_input(char *input) {
    int i;
    for (i = 0; input[i] != '\0'; i++) {
        // Allow printable ASCII only (32-126)
       if ((input[i] < 32 || input[i] > 126) && input[i] != '\n' && input[i] != '\r') {
            return 0;
        }
    }
    return 1;
}

// ── PROTOCOL HANDLER ─────────────────────────────────────────
// Parse command from message (format: CMD:PAYLOAD)
void parse_command(char *message, char *cmd, char *payload) {
    char *colon = strchr(message, ':');
    if (colon == NULL) {
        strcpy(cmd, message);
        strcpy(payload, "");
    } else {
        int cmd_len = colon - message;
        strncpy(cmd, message, cmd_len);
        cmd[cmd_len] = '\0';
        strcpy(payload, colon + 1);
    }
    // Remove newline from payload
    payload[strcspn(payload, "\n")] = '\0';
    cmd[strcspn(cmd, "\n")]         = '\0';
}

// ── CLIENT HANDLER ────────────────────────────────────────────
// This function runs in a separate thread for each client
void *handle_client(void *arg) {
    ClientInfo *client = (ClientInfo *)arg;
    char buffer[BUFFER_SIZE];
    char cmd[32];
    char payload[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    char log_msg[256];
    int  bytes_received;

    sprintf(log_msg, "Client %d connected from %s",
            client->client_id, client->ip_address);
    server_log("INFO", log_msg);

    // Send welcome message
    send(client->socket_fd,
         "WELCOME: Please login with LOGIN:username:password\n",
         51, 0);

    // Main message loop
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        bytes_received = recv(client->socket_fd, buffer, BUFFER_SIZE-1, 0);

        if (bytes_received <= 0) {
            // Client disconnected
            sprintf(log_msg, "Client %d disconnected", client->client_id);
            server_log("INFO", log_msg);
            break;
        }

        // Validate input
        if (!validate_input(buffer)) {
            send(client->socket_fd,
                 "ERROR: Invalid characters in message\n", 37, 0);
            server_log("WARN", "Invalid input received — rejected");
            continue;
        }

        // Parse the command
        parse_command(buffer, cmd, payload);

        sprintf(log_msg, "Client %d sent: %s", client->client_id, cmd);
        server_log("INFO", log_msg);

        // ── COMMAND: LOGIN ──
        if (strcmp(cmd, "LOGIN") == 0) {
            char uname[32], pass[32];
            // Payload format: username:password
            char *colon = strchr(payload, ':');
            if (colon == NULL) {
                send(client->socket_fd,
                     "ERROR: Format is LOGIN:username:password\n", 41, 0);
                continue;
            }
            int ulen = colon - payload;
            strncpy(uname, payload, ulen);
            uname[ulen] = '\0';
            strcpy(pass, colon + 1);

            if (authenticate(uname, pass)) {
                client->is_authenticated = 1;
                strcpy(client->username, uname);
                sprintf(response, "SUCCESS: Welcome %s!\n", uname);
                send(client->socket_fd, response, strlen(response), 0);
                sprintf(log_msg, "Client %d authenticated as %s",
                        client->client_id, uname);
                server_log("AUTH", log_msg);
            } else {
                send(client->socket_fd,
                     "ERROR: Invalid credentials\n", 27, 0);
                server_log("WARN", "Failed authentication attempt");
            }

        // ── COMMAND: MSG ──
        } else if (strcmp(cmd, "MSG") == 0) {
            if (!client->is_authenticated) {
                send(client->socket_fd,
                     "ERROR: Please login first\n", 26, 0);
                continue;
            }
            sprintf(response, "SERVER RECEIVED: %s\n", payload);
            send(client->socket_fd, response, strlen(response), 0);

        // ── COMMAND: TIME ──
        } else if (strcmp(cmd, "TIME") == 0) {
            if (!client->is_authenticated) {
                send(client->socket_fd,
                     "ERROR: Please login first\n", 26, 0);
                continue;
            }
            time_t now = time(NULL);
            sprintf(response, "TIME: %s", ctime(&now));
            send(client->socket_fd, response, strlen(response), 0);

        // ── COMMAND: ECHO ──
        } else if (strcmp(cmd, "ECHO") == 0) {
            if (!client->is_authenticated) {
                send(client->socket_fd,
                     "ERROR: Please login first\n", 26, 0);
                continue;
            }
            sprintf(response, "ECHO: %s\n", payload);
            send(client->socket_fd, response, strlen(response), 0);

        // ── COMMAND: QUIT ──
        } else if (strcmp(cmd, "QUIT") == 0) {
            send(client->socket_fd, "GOODBYE\n", 8, 0);
            sprintf(log_msg, "Client %d sent QUIT", client->client_id);
            server_log("INFO", log_msg);
            break;

        // ── UNKNOWN COMMAND ──
        } else {
            send(client->socket_fd,
                 "ERROR: Unknown command. Use LOGIN/MSG/TIME/ECHO/QUIT\n",
                 53, 0);
        }
    }

    // Cleanup
    close(client->socket_fd);
    pthread_mutex_lock(&client_mutex);
    active_clients--;
    pthread_mutex_unlock(&client_mutex);
    free(client);
    return NULL;
}

// ── MAIN SERVER ───────────────────────────────────────────────
int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_counter   = 0;
    int opt = 1;

    printf("=============================================\n");
    printf("  ST5004CEM Task 4: Network Server\n");
    printf("  Student: Ritika Khatiwada\n");
    printf("  Port: %d | Max Clients: %d\n", PORT, MAX_CLIENTS);
    printf("=============================================\n\n");

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("ERROR: Socket creation failed");
        exit(1);
    }

    // Allow port reuse
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(PORT);

    // Bind socket to port
    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("ERROR: Bind failed");
        exit(1);
    }

    // Listen for connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("ERROR: Listen failed");
        exit(1);
    }

    pthread_mutex_init(&client_mutex, NULL);
    server_log("INFO", "Server started — waiting for connections...");

    // Accept loop - runs forever
    while (1) {
        // Accept new client connection
        client_fd = accept(server_fd,
                           (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            server_log("ERROR", "Accept failed");
            continue;
        }

        // Check max clients
        pthread_mutex_lock(&client_mutex);
        if (active_clients >= MAX_CLIENTS) {
            pthread_mutex_unlock(&client_mutex);
            send(client_fd,
                 "ERROR: Server full. Try again later\n", 36, 0);
            close(client_fd);
            server_log("WARN", "Connection rejected — server full");
            continue;
        }
        active_clients++;
        pthread_mutex_unlock(&client_mutex);

        // Create client info
        ClientInfo *client = malloc(sizeof(ClientInfo));
        client->socket_fd        = client_fd;
        client->client_id        = ++client_counter;
        client->is_authenticated = 0;
        strcpy(client->ip_address,
               inet_ntoa(client_addr.sin_addr));

        // Create thread for this client
        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, client);
        pthread_detach(thread);
    }

    close(server_fd);
    pthread_mutex_destroy(&client_mutex);
    return 0;
}
