#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 12345
#define MAX_MESSAGE_SIZE 4096

int main(int argc, char *argv[])
{

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


    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    // Authenticate with username and password
    char username[50];
    char password[50];

    // Prompt the user for username and password
    printf("Enter your username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0; 

    printf("Enter your password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;

    sprintf(message, "%s:%s", username, password);

    // Send authentication message to the server
    send(client_socket, message, strlen(message), 0);

    // Receive the token from the server
    recv(client_socket, response, sizeof(response), 0);
    printf("\033[0;35mReceived token: %s\n\n\033[0m", response);

    // Now you can send messages using the token

    while (1)
    {
        memset(response, 0, sizeof(response));
        memset(message, 0, sizeof(message));
        printf("\033[0;33mEnter your message:\033[0m \n");
        fgets(message, sizeof(message), stdin);

        // Remove trailing newline character
        message[strcspn(message, "\n")] = 0;

        if (strncmp(message, "lcd", 3) == 0)
        {
            // Extract the local path
            sscanf(message, "lcd %s", local_path);
            

            char cwd[1024];
            char cwdb[1024];                  // used to back the last directory if access is denied
            // Ottieni il percorso della directory corrente
            getcwd(cwd, sizeof(cwd));
            getcwd(cwdb, sizeof(cwd));

            char folder_path[256];
            sprintf(folder_path, "/client_Folder");

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

                        printf("Changed directory to %s\n", local_path);
                        send(client_socket, "directory changed", 20, 0);
                    }
                    else
                    {
                        chdir(cwdb);
                        printf("Error changing directory permission denied \n");
                        send(client_socket, "Error changing directory permission denied \n", 100, 0);
                    }
                }
            }
        }

        else
        {

        
            send(client_socket, message, strlen(message), 0);
        }

        if (strcmp(message, "exit") == 0)
        {
            printf("\033[0;31mClient requested to exit. Closing connection.\033[0m\n");
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
              
                // Controlla se la stringa "finish of the text" è stata trovata
                if (strstr(response, "finish of the text") != NULL)
                {
                   //if i find finish of the text
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

            printf("\033[0;32mFile %s received successfully.\033[0m\n\n", fileName);
        }

        else if (strncmp(message, "put ", 4) == 0)
        {

            char filename[MAX_MESSAGE_SIZE];
            sscanf(message, "put %s", filename);

      
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
                // printf("messaggio inviato:%s\n",buffer);
            }

            // Send a termination message to the server to indicate the end of the file
            send(client_socket, "finish of the text", strlen("finish of the text"), 0);

            fclose(file);

            printf("\033[0;32mFile %s sent to the server.\n\n\033[0m", filename);
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
                printf("\033[0;32mServer response: \n%s\n\n\033[0m", response);
            }
        }
    }


    close(client_socket);

    return 0;
}
