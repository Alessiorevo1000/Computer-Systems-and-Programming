# Computer-Systems-and-Programming

# Simple FTP Server

## Description

This project implements a basic FTP server-client model with two main components:  

1. **Server Folder (`server_Folder`)**:  
   - Contains `server.c` (server-side code) and its executable file.  
   - Includes directories for users (`root` and `user1`), each with private spaces inaccessible to others.  

2. **Client Folder (`client_Folder`)**:  
   - Contains `client.c` (client-side code), `test.c` (test client), their respective executables, and other files for testing file transmission.  

### Key Features

- **Server**:  
  - Listens on port `12345` and waits for client connections.  

- **Client**:  
  - Connects to the server using IP/DNS via `./client <IP address>` (default: `127.0.0.1`).  
  - Authenticates with credentials (currently supports: `root/password` and `user1/password`).  
  - Upon successful authentication, users receive a token to identify subsequent requests.  

- **Supported Commands**:  
  1. `ls` - List files in the user's private server directory.  
  2. `get` - Download a file from the server to the client.  
  3. `put` - Upload a file from the client to the server.  
  4. `cd` - Change directory on the server.  
  5. `lcd` - Change directory locally on the client.  
  6. `exit` - Close the connection.  

### Notes  
- Users cannot access directories of other users.  
- A test execution can be observed by running `./server` and then `./test`.  

---

## Installation and Usage  

1. **Compile the code**:  
   - In `client_Folder`:  
     ```bash
     gcc client.c -o client
     gcc test.c -o test
     ```  
   - In `server_Folder`:  
     ```bash
     gcc server.c -o server
     ```  

2. **Run the server**:  
   ```bash
   ./server
   ```  

3. **Run the client**:  
   ```bash
   ./client
   ```  
   - Use the available commands in the client prompt to interact with the server.  

4. **Test the system**:  
   ```bash
   ./test
   ```  

---

## Example Execution  

Running `./test` demonstrates typical operations:  

```plaintext
=========================
Received token: Vvvxp6QwCW for 1
=========================
Server response:
dir  
mio1.txt  
nascosto.txt  
nuovo.txt  
=========================
File mio1.txt received successfully for 1.
=========================
Client requested to exit. Closing connection for 1.
=========================
```  

This example showcases file transfers, directory changes, and permission handling.  

---  

Enjoy experimenting with this Simple FTP Server! 🎉
