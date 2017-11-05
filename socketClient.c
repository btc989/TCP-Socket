#include "socketInclude.h"
#include "socketFunctions.h"

int main(int argc, char * * argv) {
  int i;
  int socket_fd;
  struct sockaddr_in server_addr;
  struct hostent * hp;

  /*ADDED */
  char ch[MAXLINE]; /* character for i/o */
  int num_char = MAXLINE; /* number of characters */
  char command[20];
  char fileName[20];
  bzero(command, sizeof(command));
  bzero(command, sizeof(fileName));

  /*END*/

  if (argc < 2) {
    printf("argc ERROR in main: Not enough arguments");
    exit(1);
  } else if (argc > 2) {
    printf("argc ERROR in main: Too many arguments");
    exit(1);
  }

  /*ADDED TO GET WHAT USER WANTS */
  printf("\n Enter File Name: ");
  scanf("%s", fileName);

  printf("\n Would you like to read or write from file?: ");
  scanf("%s", command);
  //Error Check if command is recongized
  if ((strcmp("read", command) != 0) && (strcmp("write", command) != 0)) {

    do {
      bzero(command, sizeof(command));
      printf("\n Please enter read, write, or quit?: ");
      scanf("%s", command);
    } while ((strcmp("read", command) != 0) && (strcmp("write", command) != 0) && (strcmp("quit", command) != 0));
    //user wants to quit program

    if (strcmp("quit", command) == 0) {
      exit(1);
    }
  }

  /*END OF ADDED */

  if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("socket ERROR in main");
    exit(1);
  }

  memset( & server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  hp = gethostbyname(argv[1]);
  if (hp == (struct hostent * ) NULL) {
    printf("gethostbyname ERROR in main: %s does not exist", argv[1]);
    exit(1);
  }
  memcpy( & server_addr.sin_addr, hp -> h_addr, hp -> h_length);
  server_addr.sin_port = htons(TCP_PORTNO);

  if (connect(socket_fd, (struct sockaddr * ) & server_addr, sizeof(server_addr)) < 0) {
    printf("connect ERROR in main");
    exit(1);
  }

  //WHERE MOST OF THE WORK WILL HAPPEN

  if (strcmp("write", command) == 0) { //if the user said WRITE
    write_file(stdin, socket_fd, fileName); //call write_file
  } else { //if the user said READ
    read_file(stdin, socket_fd, fileName); //call read_file
  }

  //send_message (stdin, socket_fd);  		//call send_message function

  close(socket_fd);

  return;
}

/*ADDED FROM THIS POINT ONWARD */
void read_file(FILE * fp, int socket_fd, char * fileName) {
  int i;
  int n;
  int j = 4;
  int n_char = 0;
  char send_line[MAX_LINE_SIZE];
  char recv_line[MAX_LINE_SIZE];
  char test[MAX_LINE_SIZE];
  char command[MAX_LINE_SIZE];
  int output;
  FILE * fd;
  fd = fopen(fileName, "w");
  if (fd == NULL) {
    printf("ERROR: failed to open or create file");
    return;
  }

  //Create message to send to server
  strcpy(send_line, "rrq  ");
  strcat(send_line, fileName);
  strcat(send_line, "\n\0");
  n = strlen(send_line);

  if ((i = write_n(socket_fd, send_line, n)) != n) {
    printf("ERROR: could not send to server");
    exit(1);
  }

    
  do {
      
    n = read_line(socket_fd, recv_line, MAX_LINE_SIZE);
    
    //If n == 0 there was no data recieved
    //If n > 0 there was data recieved could be file data or eof
    //If n < 0 there was an error
    if (n <= 0) {
      printf("ERROR: could not read from server");
      return;
    }
    //Get the first 5 characters which is the command
    strncpy(command, recv_line, 5);

    //If data is being sent
    if (strncmp(command, "data", 4) == 0) //THE REPLY WILL NOT ME "DATA", it will be the first line of the file
    {
      //copy data into file char by char to avoid the server command
      printf("Data Sent From Server: ");
      for (j = 5; j < strlen(recv_line); j++) {
        fputc(recv_line[j], fd);
        printf("%c", recv_line[j]);
      }
      //send server ack
      strcpy(send_line, "ack  \n\0");
      n = strlen(send_line);
      if ((i = write_n(socket_fd, send_line, n)) != n) {
        printf("ERROR could not write to server");
        return;
      }
    }
    //if over ten lines of data
    else if (strncmp(command, "fse", 3) == 0) {
      bzero(command, sizeof(command));
      printf("\n File over 10 Lines of data. Continue Y or N?");
      scanf("%s", command);
      //Error Check if command is recongized
      if ((strcmp(command, "Y") != 0) && (strcmp(command, "N") != 0) && (strcmp(command, "y") != 0) && (strcmp(command, "n") != 0)) {
        do {
          bzero(command, sizeof(command));
          printf("\n Please enter Y or N?");
          scanf("%s", command);
        } while ((strcmp("Y", command) != 0) && (strcmp("N", command) != 0) && (strcmp("y", command) != 0) && (strcmp("n", command) != 0));
      }
      //send server cont message
      if ((strcmp("Y", command) == 0) || (strcmp("y", command) == 0)) {
        //send server ack
        strcpy(send_line, "cont \n\0");
        n = strlen(send_line);
        if ((i = write_n(socket_fd, send_line, n)) != n) {
          printf("ERROR: failed to read from server");
          return;
        }
      }
      //send abort message
      else {
        //send server ack
        printf("Server sent abort \n");
        strcpy(send_line, "abort\n\0");
        n = strlen(send_line);
        if ((i = write_n(socket_fd, send_line, n)) != n) {
          printf("ERROR: failed to write to server");
          return;
        }
        //wait for ack
        //then return
        do {
          n = read_line(socket_fd, recv_line, MAX_LINE_SIZE);
          if (n < 0) {
            printf("ERROR: failed to read from server");
            return;
          }
          strncpy(command, recv_line, 5);
        } while (strncmp(command, recv_line, 3) != 0);

        if (strncmp(command, "ack", 3) == 0) {
          printf("Server acknowlegment recieved\n");
          fclose(fd);
          //remove any data sent from server
          int status = remove(fileName);

          if (status == 0) {
            printf("The file %s was deleted successfully.\n", fileName);
          } else {
            printf("ERROR:Unable to delete the %s\n", fileName);
          }
          return;
        }
      }
    }
  } while (strncmp(command, "eof", 3) != 0);
  printf("End of File recieved. Closing Connection");
  fclose(fd);
  return;
}

void write_file(FILE * fp, int socket_fd, char * fileName) {
  char send_line[MAX_LINE_SIZE];
  char recv_line[MAX_LINE_SIZE];
  char command[MAX_LINE_SIZE];
  int n = 0;
  int i = 0;
  int inFile = 0;
  int n_char = 0;
  char data[MAX_LINE_SIZE];
  bzero(data, sizeof(data));
  //SEND wrq
  strcpy(send_line, "wrq  "); //Create message to send to server
  strcat(send_line, fileName);
  strcat(send_line, "\n\0");
  n = strlen(send_line); //lenght of the command

  if ((i = write_n(socket_fd, send_line, n)) != n) //send the wrq to the server
  {
    printf("Error: sending wrq to server\n");
    exit(1);
  }

  //READ for ack
  n = read_line(socket_fd, recv_line, MAX_LINE_SIZE); //get the response from the server

  if (n < 0) //we had an error
  {
    printf("Error: File return from server\n");
    exit(1);
  } else if (n == 0) //nothing in message
  {
    printf("Server responded with nothing\n");
  } else //we got values back
  {
    if (strncmp(recv_line, "ack", 3) == 0) //the command was ack
    {
      printf("Server responeded with acknowledgment\n"); ///TEST STATEMENT

      inFile = open(fileName, O_RDONLY); //open the file to send to server
      if (inFile == -1) {
        printf("Error: Could Not Open File /n");
        exit(1);
      }

      printf("File was opened\n"); ///TEST STATEMENT

      strcpy(send_line, "data ");
      while ((n_char = read_line(inFile, data, MAX_LINE_SIZE)) > 0) {
        strcat(send_line, data);
        strcat(send_line, "\n\0");
        n = strlen(send_line); //lenght of the command

        printf("The command being sent to server is: %s", send_line); ///TEST STATEMENT

        if ((i = write_n(socket_fd, send_line, n)) != n) //send the wrq to the server
        {
          printf("Error: sending wrq to server\n");
          exit(1);
        }

        n = read_line(socket_fd, recv_line, MAX_LINE_SIZE);
        if (n <= 0) //there was a read error
        {
          printf("Error: ack problem with eof send\n");
          exit(1);
        } else {
          if (strncmp(recv_line, "fse", 3) == 0) //we got ack from the server
          {
            printf("File Size exceeed message recieved. Sending abort \n");
            //abort
            strcpy(send_line, "abort\n\0"); //Create message to send to server
            n = strlen(send_line); //lenght of the command
            if ((i = write_n(socket_fd, send_line, n)) != n) //send the wrq to the server
            {
              printf("Error: sending wrq to server\n");
              exit(1);
            }
            //wait for ack message
            do {

              n = read_line(socket_fd, recv_line, MAX_LINE_SIZE);
              if (n == 0)
                break;
              if (n < 0) {
                printf("ERROR: reading from client\n");
                exit(1);
              }
              strncpy(command, recv_line, 5);
            } while (strncmp(command, "ack", 3) != 0); //end of for loop
            //wait for ack

            //then return
            printf("Server sent acknowledgment\n");
            close(inFile);
            exit(1);
          } else if (strncmp(recv_line, "ack", 3) != 0) {
            //pe teserver sent junk or something not exc
            printf("Server sent junk %s\n", recv_line);
            return;
          }
        }
        bzero(send_line, sizeof(send_line));
        bzero(recv_line, sizeof(send_line));
        strcpy(send_line, "data ");
      }

      printf("Sending EOF message");
      strcpy(send_line, "eof  \n\0"); //Create message to send to server
      n = strlen(send_line); //lenght of the command
      if ((i = write_n(socket_fd, send_line, n)) != n) //send the wrq to the server
      {
        printf("Error: sending wrq to server\n");
        exit(1);
      }

      bzero(recv_line, MAX_LINE_SIZE);
      //get ack from server that they got eof
      n = read_line(socket_fd, recv_line, MAX_LINE_SIZE);

      if (n <= 0) //there was a read error
      {
        printf("Error: ack problem with eof send\n");
        exit(1);
      } else {
        if (strncmp(recv_line, "ack", 3) == 0) //we got ack from the server
        {
          printf("Server sent ack\n");
          exit(1);
        } else {
          printf("Server sent a message that is not ack\n");
          exit(1);
        }
      }
    } else //the response was not ack
    {
      printf("Error server responded with wrong message\n");
      exit(1);
    }
  }
  return;
}