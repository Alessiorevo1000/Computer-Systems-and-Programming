#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h> //functions and data structures necessary for manipulating IP addresses and network addresses in binary format
#include <sys/socket.h>
#include <netinet/in.h> //declarations and definitions for Internet Protocol (IP) network operations
#include <sys/wait.h>
#include <time.h>

#define STRING_LENGTH 10 // size of random string for token
#define PORT 12345
#define MAX_CLIENTS 10                            // number of client in queue
#define MAX_MESSAGE_SIZE 8192                     // size of buffer
#define MAX_COMMAND_SIZE (MAX_MESSAGE_SIZE + 6)   // size of ls command
#define MAX_FILE_PATH_SIZE (MAX_MESSAGE_SIZE + 6) // size of path for put command

char local_path[MAX_MESSAGE_SIZE];


void generate_random_string(char *str, size_t length)
{
    
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    // Seed the random number generator
    srand((unsigned int)time(NULL));

    
    for (size_t i = 0; i < length; ++i)
    {
        int index = rand() % (int)(sizeof(charset) - 1);
        str[i] = charset[index];
    }

    // Null-terminate the string
    str[length] = '\0';
}

// Structure to manage user credentials
struct User
{
    char username[50];
    char password[50];
    char path[200];
    char token[50];
    bool authenticated;
};

// Registered users
struct User users[] = {
    {"root", "password", "./root", "", false}, // Initial token is empty
    {"user1", "password", "./user1", "", false},
};

int num_users = sizeof(users) / sizeof(users[0]);

// Function to authenticate the user and generate token
bool authenticate_user(const char *username, const char *password, char *token)
{
    for (int i = 0; i < num_users; ++i)
    {

        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0)
        {

           
            chdir(users[i].path); // set private directory

            char random_string[STRING_LENGTH + 1]; // +1 for the null terminator

          
            generate_random_string(random_string, STRING_LENGTH);

           

            strcpy(token, random_string);  // Generate token
            strcpy(users[i].token, token); // Save the token for this user
            users[i].authenticated = true; // Mark user as authenticated
            return true;
        }
    }
    return false; 
}

void handle_client(int client_socket)
{
    char buffer[MAX_MESSAGE_SIZE];
    ssize_t received;

    char username[50];
    char password[50];
    char message[MAX_MESSAGE_SIZE];

    // Receive username and password from the client
    received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (received > 0)
    {
        buffer[received] = '\0'; // Null-terminate the received data

        // Extract username and password from the received buffer
        sscanf(buffer, "%s %s", username, password);


        char *splitter = strtok(buffer, ":");
        if (splitter != NULL)
        {
            // Copy the first part (username) 
            strcpy(username, splitter);

            // Get the next splitter (password)
            splitter = strtok(NULL, ":");
            if (splitter != NULL)
            {
                // Copy the second part (password) 
                strcpy(password, splitter);

                // Print the username and password
                printf("Username: %s\n", username);
                printf("Password: %s\n", password);
            }
        }

        // Authenticate the user and generate token
        char token[50];
        if (authenticate_user(username, password, token))
        {
            
            send(client_socket, token, strlen(token), 0);

            // Continue receiving messages from the authenticated client
            while ((received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0)
            {
                buffer[received] = '\0'; // Null-terminate the received data

            
                if (strcmp(buffer, "exit") == 0)
                {
                    printf("\033[0;31mClient %s requested to exit. Closing connection.\n \033[0m", username);
                    break; // Exit the loop and close the connection
                }

             

                else if (strncmp(buffer, "ls ", 3) == 0)
                {
                    // Extract the directory path from the "ls" command
                    char directory[MAX_MESSAGE_SIZE];
                    strcpy(directory, buffer + 3); // Skip the "ls " part

                    // Execute the ls command in the specified directory
                    char command[MAX_COMMAND_SIZE];
                    snprintf(command, sizeof(command), "ls %s", directory);
                    FILE *ls_output = popen(command, "r");

                    if (ls_output == NULL)
                    {
                        perror("Error executing ls command");
                        const char *error_message = "Error executing ls command.";
                        send(client_socket, error_message, strlen(error_message), 0);
                    }
                    else
                    {
                        // Read the output of ls command and send it back to the client
                        while (fgets(buffer, sizeof(buffer), ls_output) != NULL)
                        {
                            send(client_socket, buffer, strlen(buffer), 0);
                        }
                        pclose(ls_output);
                    }
                }

                    // check if cd in buffer 

                else if (strncmp(buffer, "cd", 2) == 0)
                {
                    // Extract the local path
                    sscanf(buffer, "cd %s", local_path);

                    // Change the directory
                    char cwd[1024];
                    char cwdb[1024];

                    // Get the current directory path
                    getcwd(cwd, sizeof(cwd));
                    getcwd(cwdb, sizeof(cwd)); // Used to return to the last directory if access is denied

                    char folder_path[256];
                    sprintf(folder_path, "/server_Folder/%s", username);

                    if (strcmp(cwd, folder_path) == 0 && strlen(cwd) <= strlen(folder_path) && strncmp(buffer, "cd ..", 5) == 0)
                    {
                        perror("Error changing directory permission denied");
                        send(client_socket, "Error changing directory permission denied \n", 100, 0);
                    }
                    else
                    {
                        if (strstr(cwd, folder_path) != NULL)
                        {
                            if (chdir(local_path) != 0)
                            {
                                perror("Error changing directory");
                            }
                            else
                            {
                                getcwd(cwd, sizeof(cwd));
                                if (strlen(cwd) >= strlen(folder_path) && strstr(cwd, folder_path) != NULL)
                                {
                                    send(client_socket, "directory changed", 20, 0);
                                }
                                else
                                {
                                    chdir(cwdb);
                                    send(client_socket, "Error changing directory permission denied \n", 100, 0);
                                }
                            }
                        }
                        else
                        {
                            send(client_socket, "Error changing directory permission denied \n", 100, 0);
                        }
                    }
                }


                else if (strncmp(buffer, "get ", 4) == 0)
                {
                    // Extract the filename from the "get" command
                    char filename[MAX_MESSAGE_SIZE];
                    strcpy(filename, buffer + 4); // Skip the "get " part

                    FILE *file = fopen(filename, "r");
                    if (file == NULL)
                    {
                        perror("Error opening file");
                        const char *error_message = "Error opening file.";
                        send(client_socket, error_message, strlen(error_message), 0);
                    }
                    else
                    {
                        // Read the content of the file and send it back to the client
                        char file_buffer[MAX_MESSAGE_SIZE];
                        size_t bytes_read;
                        while ((bytes_read = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0)
                        {
                            send(client_socket, file_buffer, bytes_read, 0);
                        }

                        // Check if there was an error reading the file
                        if (ferror(file))
                        {
                            perror("Error reading file");
                            const char *error_message = "Error reading file.";
                            send(client_socket, error_message, strlen(error_message), 0);
                        }
                        else
                        {
                            // Send a message indicating the end of the file content
                            const char *end_message = "finish of the text";
                            send(client_socket, end_message, strlen(end_message), 0);
                        }

                        fclose(file); 
                    }
                }

                else if (strncmp(buffer, "put ", 4) == 0)
                {
                    char fileName[MAX_MESSAGE_SIZE];
                    char filePath[MAX_FILE_PATH_SIZE]; 

                    sscanf(buffer, "put %s", fileName);

                    
                    char directory[] = "./";

                    // Combine the directory path and file name to create the full file path
                    snprintf(filePath, sizeof(filePath), "%s%s", directory, fileName);

                    FILE *file = fopen(filePath, "w");
                    if (file == NULL)
                    {
                        perror("Error creating file");
                        continue;
                    }

                    // Receive data from the client and write it to the file
                    do
                    {
                        memset(buffer, 0, sizeof(buffer));
                        ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
                        // Check if the string "finish of the text" is found
                        if (strstr(buffer, "finish of the text") != NULL)
                        {

                            char *substr = "finish of the text";
                            char *ptr;

                           
                            if ((ptr = strstr(buffer, substr)) != NULL)
                            {
                                // Calculate the length of the part before the substring
                                size_t len_before = ptr - buffer;

                                // Write the part before the substring to the file
                                fwrite(buffer, sizeof(char), len_before, file);
                                
                            }

                            break; 
                        }
                        else
                        {

                            fprintf(file, "%s\n", buffer);
                            printf("%s\n", buffer);
                        }
                    } while (1);

                    // Close the file after receiving data
                    fclose(file);
                    printf("File %s received and saved successfully.\n", filePath);
                }

                else
                {

                    
                    printf("Message from %s: %s\n", username, buffer);
                  
                    send(client_socket, buffer, strlen(buffer), 0);
                }
                memset(buffer, 0, sizeof(buffer)); // Clear the buffer
            }
        }
        else
        {
            // Authentication failed
            const char *error_message = "Authentication failed. Invalid username or password.";
            send(client_socket, error_message, strlen(error_message), 0);
        }
    }

    // Close the client socket
    close(client_socket);
}

int main()
{

    chdir("../server_Folder");
    remove("root/tuo1.txt");
    remove("user1/tuo2.txt");                   //these variables are used to test

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);

    // Create the server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Configure the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the address and port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Socket binding error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) == -1)
    {
        perror("Error in listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port: %d...\n", PORT);

    while (1)
    {
        // Accept incoming connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
        if (client_socket == -1)
        {
            perror("Error accepting connection");
            continue;
        }

        // Create a child process to handle client communication
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("Error forking");
            close(client_socket);
            continue;
        }
        else if (pid == 0)
        {
            // Child process handles client communication
            close(server_socket); // Close the server socket in the child process
            handle_client(client_socket);
            exit(EXIT_SUCCESS);
        }
        else
        {
            // Parent process continues to accept new connections
            close(client_socket); // Close the client socket in the parent process
            while (waitpid(-1, NULL, WNOHANG) > 0)
                ; // Clean up zombie processes
        }
    }

    // Close the server socket (unreachable in the current implementation)
    close(server_socket);

    return 0;
}
