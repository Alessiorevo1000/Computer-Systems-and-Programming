#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 12345
#define MAX_MESSAGE_SIZE 4096

struct ThreadArgs
{
    char *server_address;
    int thread_id;
};

void *run_client(void *arg)
{
    struct ThreadArgs *args = (struct ThreadArgs *)arg;
    char *server_address = args->server_address;
    int thread_id = args->thread_id;

    int client_socket;
    struct sockaddr_in server_addr;
    char message[MAX_MESSAGE_SIZE];
    char response[MAX_MESSAGE_SIZE];
    char local_path[MAX_MESSAGE_SIZE];

    // Create the client socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Configure the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(server_address);
    // server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Assuming server is running locally

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    // Authenticate with username and password
    char username[50];
    char password[50];

    if (thread_id == 1)
    {
        strcpy(username, "root");
        strcpy(password, "password");
    }
    else
    {
        strcpy(username, "user1");
        strcpy(password, "password");
    }

    sprintf(message, "%s:%s", username, password);

    // Send authentication message to the server
    send(client_socket, message, strlen(message), 0);
    printf("\033[0;34m=========================\033[0m\n\n");
    // Receive the token from the server
    recv(client_socket, response, sizeof(response), 0);
    printf("\033[0;35mReceived token: %s \033[0;33m for %d\033[0m\n\n", response, thread_id);
    printf("\033[0;34m=========================\033[0m\n\n");

    // Now you can send messages using the token
    int prove = 0;

    remove("mio1.txt");
    remove("mio2.txt");

    while (1)
    {
        memset(response, 0, sizeof(response));
        memset(message, 0, sizeof(message));

        sleep(2);
        printf("\033[0;35m=========================\033[0m\n\n");
        if (prove == 0)
        {
            strncpy(message, "ls .\n", MAX_MESSAGE_SIZE);
            printf("thread number: %d send ls . to server\n",thread_id);
            prove = prove + 1;
        }
        else if (prove == 1)
        {
            strncpy(message, "ls dir\n", MAX_MESSAGE_SIZE);
             printf("thread number: %d send ls dir to server\n",thread_id);
            prove = prove + 1;
        }
        else if (prove == 2)
        {
            strncpy(message, "ls dir/ultimo\n", MAX_MESSAGE_SIZE);
             printf("thread number: %d send ls dir/ultimo to server\n",thread_id);
            prove = prove + 1;
        }
        else if (prove == 3)
        {
            if (thread_id == 1)
            {
                strncpy(message, "get mio1.txt", MAX_MESSAGE_SIZE);
                 printf("thread number: %d send get mio1.txt to server\n",thread_id);
            }
            else
            {
                strncpy(message, "get mio2.txt", MAX_MESSAGE_SIZE);
                 printf("thread number: %d send get mio2.txt to server\n",thread_id);
            }
            prove = prove + 1;
        }
        else if (prove == 4)
        {
            if (thread_id == 1)
            {
                strncpy(message, "put tuo1.txt", MAX_MESSAGE_SIZE);
                 printf("thread number: %d send put tuo1.txt to server\n",thread_id);
            }
            else
            {
                strncpy(message, "put tuo2.txt", MAX_MESSAGE_SIZE);
                 printf("thread number: %d send put tuo2.txt to server\n",thread_id);
            }
            prove = prove + 1;
        }
        else if (prove == 5)
        {
            strncpy(message, "cd dir", MAX_MESSAGE_SIZE);
             printf("thread number: %d send cd dir to server\n",thread_id);
            prove = prove + 1;
        }
        else if (prove == 6)
        {
            strncpy(message, "cd ..", MAX_MESSAGE_SIZE);
             printf("thread number: %d send cd .. to server\n",thread_id);
            prove = prove + 1;
        }
        else if (prove == 7)
        {
            strncpy(message, "cd ..", MAX_MESSAGE_SIZE);
             printf("thread number: %d send cd .. to server 'try to exit from private directory'\n",thread_id);
            prove = prove + 1;
        }
        else if (prove == 8)
        {
            strncpy(message, "exit", MAX_MESSAGE_SIZE);
             printf("thread number: %d send exit to server\n",thread_id);
            prove = prove + 1;
        }
        else
        {
            // Stop execution
            exit(EXIT_SUCCESS);
        }

        // Remove trailing newline character
        message[strcspn(message, "\n")] = 0;

        if (strncmp(message, "lcd", 3) == 0)
        {
            // Extract the local path
            sscanf(message, "lcd %s", local_path);
            // Change the directory

            char cwd[1024];
            char cwdb[1024];
            // Ottieni il percorso della directory corrente
            getcwd(cwd, sizeof(cwd));
            getcwd(cwdb, sizeof(cwd));

            char folder_path[256];
            sprintf(folder_path, "/home/name/Desktop/new/client_Folder");

            if (strcmp(cwd, folder_path) == 0 && strlen(cwd) <= strlen(folder_path) && strncmp(message, "..", 5) == 0)
            {

                perror("Error changing directory permission denied");
            }
            else
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

                        printf("Changed directory to %s \033[0;33m for %d\n", local_path, thread_id);
                        send(client_socket, "directory changed", 20, 0);
                    }
                    else
                    {
                        chdir(cwdb);
                        printf("Error changing directory permission denied \033[0;33m for %d \n", thread_id);
                        send(client_socket, "Error changing directory permission denied \n", 100, 0);
                    }
                }
            }
        }

        else
        {

            // Send message to the server
            send(client_socket, message, strlen(message), 0);
        }

        if (strcmp(message, "exit") == 0)
        {
            printf("\033[0;31mClient requested to exit. Closing connection \033[0;33m for %d.\033[0m\n", thread_id);
            break;
        }

        if (strncmp(message, "get ", 4) == 0)
        {
            char fileName[MAX_MESSAGE_SIZE];
            sscanf(message, "get %s", fileName);
            FILE *file = fopen(fileName, "w");
            if (file == NULL)
            {
                perror("Error creating file");
                continue;
            }
            // Receive and write data to the file until the server stops sending
            do
            {
                memset(response, 0, sizeof(response));
                ssize_t bytes_received = recv(client_socket, response, sizeof(response), 0);

                if (strstr(response, "finish of the text") != NULL)
                {

                    break;
                }
                else
                {
                    fprintf(file, "%s\n", response);
                }
            } while (1);
            char *substr = "finish of the text";
            char *ptr;

            // Find the first occurrence of the substring
            if ((ptr = strstr(response, substr)) != NULL)
            {
                // Calculate the length of the part before the substring
                size_t len_before = ptr - response;

                // Write the part before the substring to the file
                fwrite(response, sizeof(char), len_before, file);
            }
            else
            {
                // Write the entire response to the file
                fprintf(file, "%s\n", response);
            }
            fclose(file);

            printf("\033[0;32mFile %s received successfully \033[0;33m for %d.\033[0m\n\n", fileName, thread_id);
        }

        else if (strncmp(message, "put ", 4) == 0)
        {

            char filename[MAX_MESSAGE_SIZE];
            sscanf(message, "put %s", filename);

            // Open the file for reading, using a relative path
            FILE *file = fopen(filename, "r");

            if (file == NULL)
            {

                perror("Error opening file");
                exit(EXIT_FAILURE);
            }

            char buffer[MAX_MESSAGE_SIZE];
            size_t bytes_read;

            // Read from the file and send it to the server
            while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
            {
                send(client_socket, buffer, bytes_read, 0);
            }

            // Send a termination message to the server to indicate the end of the file
            send(client_socket, "finish of the text", strlen("finish of the text"), 0);

            fclose(file);

            printf("\033[0;32mFile %s sent to the server \033[0;33m for %d.\n\n\033[0m", filename, thread_id);
        }

        else
        {

            if (strncmp(message, "lcd", 3) == 0)
            {

                printf("\n");
            }
            else
            {
                recv(client_socket, response, sizeof(response), 0);
                printf("\033[0;32mServer response: \033[0;37m\n%s \033[0;33m for %d\n\n\033[0m", response, thread_id);
            }
        }
    }

    // Close the socket
    close(client_socket);

    return NULL;
}

void *execute_command(void *arg)
{
    char *command = (char *)arg;
    int result = system(command);
}

int main(int argc, char *argv[])
{

      sleep(2); 
    printf("\n\n\033[0;31mTUO1.TXT AND TUO2.TXT ARE CURRENTLY IN THE SAME FOLDER BUT ARE MANAGED BY DIFFERENT USERS, I THOUGHT THAT IN REALITY THE TWO USERS ARE ON DIFFERENT MACHINES.\n\n\n\033[0m");
  
   sleep(2); 
    
    
    printf("\033[0;33mInitial situation:\n\n");
    printf("root contains:            tuo1.txt  ------|    // same directory\n");
    printf("user1 contains:           tuo2.txt  ------|    // same directory\n");
    printf("server/root contains:     mio1.txt\n");
    printf("server/user1 contains:    mio2.txt\n\n");
    printf("------------------------------------------------------------------------\n\n");
    sleep(2);
    
    printf("Final situation:\n\n");
    printf("root contains:            mio1.txt tuo1.txt ---|  // same directory\n");
    printf("user1 contains:           mio2.txt tuo2.txt ---|  // same directory\n");
    printf("server/root contains:     mio1.txt tuo1.txt\n");
    printf("server/user1 contains:    mio2.txt tuo2.txt\n\033[0m");
    printf("------------------------------------------------------------------------\n\n");
     
    pthread_t tid;
    char command[100];

    // Construct the command to execute the executable file
    sprintf(command, "../server_Folder/server");

    // Create a thread to execute the command
    if (pthread_create(&tid, NULL, execute_command, (void *)command) != 0)
    {
        perror("pthread_create");
        return EXIT_FAILURE;
    }

    sleep(2);
    char *server_address;

    // If no server address is provided, default to 127.0.0.1
    if (argc == 1)
    {
        server_address = "127.0.0.1";
    }
    else if (argc == 2)
    {
        server_address = argv[1];
    }
    else
    {
        fprintf(stderr, "Usage: %s [server_ip_or_dns]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pthread_t thread1, thread2;
    struct ThreadArgs args1 = {server_address, 1};
    struct ThreadArgs args2 = {server_address, 2};

    // Create thread 1
    if (pthread_create(&thread1, NULL, run_client, (void *)&args1) != 0)
    {
        perror("Error creating thread 1");
        exit(EXIT_FAILURE);
    }
    sleep(6);
    // Create thread 2
    if (pthread_create(&thread2, NULL, run_client, (void *)&args2) != 0)
    {
        perror("Error creating thread 2");
        exit(EXIT_FAILURE);
    }

    // Wait for both threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(tid, NULL);
    return EXIT_FAILURE;

    return 0;
}
