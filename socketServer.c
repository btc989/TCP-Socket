#include "socketInclude.h"
#include "socketFunctions.h"

int main(int argc, char * * argv) {
  int socket_fd;
  int new_socket_fd;
  int addr_length;
  pid_t child_pid;
  struct sockaddr_in client_addr;
  struct sockaddr_in server_addr;
  struct hostent * hp;
  char host_name[256];

  if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("socket ERROR in main");
    exit(1);
  }

  memset( & server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  gethostname(host_name, sizeof(host_name));
  hp = gethostbyname(host_name);
  if (hp == (struct hostent * ) NULL) {
    printf("gethostbyname ERROR in main: %s does not exist", host_name);
    exit(1);
  }
  memcpy( & server_addr.sin_addr, hp -> h_addr, hp -> h_length);
  server_addr.sin_port = htons(TCP_PORTNO);

  if (bind(socket_fd, (struct sockaddr * ) & server_addr, sizeof(server_addr)) < 0) {
    printf("bind ERROR in main");
    exit(1);
  }

  printf("Parent: Server on port %d\n", ntohs(server_addr.sin_port));
  if (listen(socket_fd, 5) < 0) {
    printf("listen ERROR in main");
    exit(1);
  }

  for (;;) {
    printf("Parent: Waiting for client\n");
    addr_length = sizeof(client_addr);
    new_socket_fd = accept(socket_fd, (struct sockaddr * ) & client_addr, & addr_length);
    printf("Parent: Client arrived\n");

    if (new_socket_fd < 0) {
      printf("accept ERROR in main");
      exit(1);
    }

    message_echo(new_socket_fd); //call message_echo
    close(new_socket_fd); //close the socket before end of program
  }
}

void message_echo(int socket_fd) {
    int i;
    int n;
    int j = 0;
    int n_char = 0;
    int size;
    int inFile;
    int outFile;
    char line[MAX_LINE_SIZE];
    char fileName[MAX_LINE_SIZE];
    char data[MAX_LINE_SIZE];
    char command[6];
    bzero(line, sizeof(line));
    bzero(fileName, sizeof(fileName));
    bzero(data, sizeof(data));
    size_t len = 512;
    int count = 1;

    for (;;) {
      n = read_line(socket_fd, line, MAX_LINE_SIZE);
      if (n == 0) {
        //break;
      }
      if (n < 0) {
        printf("read_line ERROR in message_echo");
        exit(1);
      }

      printf("\n The line recieved : %s \n", line);
      strncpy(command, line, 5);
      //get fileName
      //copy data into file
      int t = 0;
      for (j = 5; j < strlen(line) - 1; j++) {
        fileName[t] = line[j];
        t++;
      }
      printf("The filename %s\n", fileName);

      //if read request
      if (strncmp(command, "rrq", 3) == 0) //if the command is rrq
      {
        inFile = open(fileName, O_RDONLY);
        if (inFile == -1) {
          printf("ERROR: Could Not Open File Name \n");
          return;
        }

        printf("%s was opened successfully\n", fileName);
        strcpy(line, "data ");

        while ((n_char = read_line(inFile, data, MAX_LINE_SIZE)) > 0) {
          if (count == 11) {
            printf("%s has over 10 lines of data, sending for client approval\n", fileName);
            //send message to see if client wants to continue to recieve messages
            strcpy(line, "fse   \n\0");
            n = strlen(line);
            write_n(socket_fd, line, n);
            do {
              n = read_line(socket_fd, line, MAX_LINE_SIZE);
              if (n == 0)
                break;

              if (n < 0) {
                printf("ERROR: reading from client\n");
                exit(1);
              }

              strncpy(command, line, 5);
              printf("Client decision: %s\n", line);
              //if about then close connection with client
              if (strncmp(command, "abort", 5) == 0) {
                strcpy(line, "ack \n\0");
                n = strlen(line);
                write_n(socket_fd, line, n);
                printf("Sent abort acknowledgment\n");
                close(inFile);
                return;
              }
            } while (strncmp(command, "cont", 4) != 0);
            bzero(line, sizeof(line));
            strcpy(line, "data ");
            //otherwise continue with file transfer
          }

          strcat(line, data);
          strcat(line, " \n\0");
         printf("TEST:: %s",line);
          write_n(socket_fd, line, n);
          count++;
          //wait for ack message
          do {
            printf("Waitng for acknowledgement for data \n");
            n = read_line(socket_fd, line, MAX_LINE_SIZE);
            if (n == 0)
              break;
            if (n < 0) {
              printf("ERROR: reading from client\n");
              exit(1);
            }
            strncpy(command, line, 5);
          } while (strncmp(command, "ack", 3) != 0); //end of for loop

          //clear everything ready for next data set
          bzero(line, sizeof(line));
          strcpy(line, "data ");
        } //end ofwhile statement
        //end of file send to client
        strcpy(line, "eof  \n\0");
        write_n(socket_fd, line, n);
        printf("End of File %s\n");
        close(inFile);
        return;
      } //end if
      //write request
      else if (strncmp(command, "wrq", 3) == 0) //if the command from the client begins with wrq
      {
        FILE * fd;
        fd = fopen(fileName, "w");
        if (fd == NULL) {
          printf("ERROR: failed to open or create file\n");
          return;
        }
        int t = 0; //temp counter for getting file name

        //GOT wrq SENDING ack
        printf("Sending Client acknowledgment to begin write\n");
        strcpy(line, "ack \n\0");
        n = strlen(line);
        if ((i = write_n(socket_fd, line, n)) != n) {
          printf("Error: ack return error\n");
          exit(1);
        }

        do {
          n = read_line(socket_fd, line, MAX_LINE_SIZE);
          //If n == 0 there was no data recieved
          //If n > 0 there was data recieved could be file data or eof
          //If n < 0 there was an error
          if (n <= 0) {
            printf("ERROR: could not read from server");
            return;
          }
          //Get the first 5 characters which is the command
          strncpy(command, line, 5);

          //If data is being sent
          if (strncmp(command, "data", 4) == 0) //THE REPLY WILL NOT ME "DATA", it will be the first line of the file
          {
            //if over 11 lines
            if (count >= 11) {
              printf("File over 10 lines, sending fse message\n");
              //send fse message
              strcpy(line, "fse   \n\0");
              n = strlen(line);
              write_n(socket_fd, line, n);
              //wait for abort

              n = read_line(socket_fd, line, MAX_LINE_SIZE);
              if (n == 0)
                break;

              if (n < 0) {
                printf("ERROR: reading from client \n");
                exit(1);
              }

              strncpy(command, line, 5);
              printf("TEST:: %s", command);
              //if about then close connection with client
              if (strncmp(command, "abort", 5) == 0) {
                strcpy(line, "ack \n\0");
                n = strlen(line);
                write_n(socket_fd, line, n);
                printf("Sent abort acknowledgment\n");
              } else {
                printf("ERROR: Client did not abort\n");
              }

              //delete file
              close(fd);
              int status = remove(fileName);

              if (status == 0) {
                printf("The file %s was deleted successfully.\n", fileName);
              } else {
                printf("ERROR:Unable to delete the %s\n", fileName);
              }
              strcpy(line, "ack \n\0");
              n = strlen(line);
              write_n(socket_fd, line, n);
              printf("Sent abort acknowledgment\n");
              //ack abort
              return;
            }
            //copy data into file char by char to avoid the server command
            printf("Data Sent From Client: ");
            for (j = 5; j < strlen(line); j++) {
              fputc(line[j], fd);
              printf("%c", line[j]);
            }
            //send server ack
            strcpy(line, "ack  \n\0");
            n = strlen(line);
            printf("Sending data ackknowlegment %s\n", line);
            if ((i = write_n(socket_fd, line, n)) != n) {
              printf("ERROR could not write to server");
              return;
            }
            count++;
          }
        } while (strncmp(command, "eof", 3) != 0);
        fclose(fd);
        printf("End of File Recieved. Sending acknowlegdment.\n");
        strcpy(line, "ack \n\0");
        n = strlen(line);
        if ((i = write_n(socket_fd, line, n)) != n) {
          printf("Error: ack return error\n");
          exit(1);
        }
        return;
      }
      //Client sent a bad command
      else {
        printf("Unknown Command \n");
        exit(1);
      }
    } //end for
    return;
  } //end function