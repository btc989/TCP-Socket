#include "socketInclude.h"
#include "socketFunctions.h"

int main (int argc, char **argv)
{
    int i;
    int socket_fd;
    struct sockaddr_in server_addr;
    struct hostent *hp;
    
    /*ADDED */
    char ch[MAXLINE];       /* character for i/o */
    int num_char=MAXLINE;   /* number of characters */
    char command[20];
    char fileName[20];
    bzero(command, sizeof(command)); 
    bzero(command, sizeof(fileName)); 
    
    /*END*/

    if (argc < 2)
    {
        printf ("argc ERROR in main: Not enough arguments");
        exit (1);
    }
    else if (argc > 2)
    {
        printf ("argc ERROR in main: Too many arguments");
        exit (1);
    }
    
    /*ADDED TO GET WHAT USER WANTS */
    printf("\n Enter File Name: ");
    scanf("%s", fileName);
    
    printf("\n Would you like to read or write from file?: ");
    scanf("%s", command);
    //Error Check if command is recongized
    if((strcmp ("read",command) != 0) &&(strcmp ("write",command) != 0)){
        
        do{
            bzero(command, sizeof(command)); 
            printf("\n Please enter read, write, or quit?: ");
            scanf("%s", command);
        }while((strcmp ("read",command) != 0) &&(strcmp ("write",command) != 0) &&(strcmp ("quit",command) != 0)); 
        //user wants to quit program
        
        if(strcmp ("quit",command) == 0){
            exit (1);
        }
    }
    
    /*END OF ADDED */
    

    if ((socket_fd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf ("socket ERROR in main");
        exit (1);
    }

    memset (&server_addr, 0, sizeof (server_addr));
    server_addr.sin_family = AF_INET;
    hp = gethostbyname (argv [1]);
    if (hp == (struct hostent *) NULL)
    {
        printf ("gethostbyname ERROR in main: %s does not exist", argv [1]);
        exit (1);
    }
    memcpy (&server_addr.sin_addr, hp -> h_addr, hp -> h_length);
    server_addr.sin_port = htons (TCP_PORTNO);

    if (connect (socket_fd, (struct sockaddr *) &server_addr, sizeof (server_addr)) < 0)
    {
        printf ("connect ERROR in main");
        exit (1);
    }



    //WHERE MOST OF THE WORK WILL HAPPEN

    if(strcmp ("write",command) == 0){ 			//if the user said WRITE
        write_file (stdin, socket_fd,fileName);   	//call write_file
    }
    else{                    				//if the user said READ
        read_file(stdin, socket_fd,fileName);   	//call read_file
    }
    
    //send_message (stdin, socket_fd);  		//call send_message function

    close (socket_fd);

    return;
}

void send_message (FILE *fp, int socket_fd)
{
    int i;
    int n;
    char send_line [MAX_LINE_SIZE];
    char recv_line [MAX_LINE_SIZE];

    while (fgets (send_line, MAX_LINE_SIZE, fp) != (char *) NULL)
    {
        n = strlen (send_line);
        if ((i = write_n (socket_fd, send_line, n)) != n)
        {
            printf ("write_n ERROR in send_message");
	    exit (1);
        }
        n = read_line (socket_fd, recv_line, MAX_LINE_SIZE);
        if (n < 0)
        {
            printf ("read_line ERROR in send_message");
            exit (1);
        }
        fputs (recv_line, stdout);
    }
    if (ferror (fp))
    {
        printf ("message ERROR in send_message");
        exit (1);
    }

    return;
}

/*ADDED FROM THIS POINT ONWARD */
void read_file (FILE *fp, int socket_fd, char * fileName)
{
    printf("TEST::Beginning of read_file function\n");

    int i;
    int n;
    int j=4;
    int n_char=0;
    char send_line [MAX_LINE_SIZE];
    char recv_line [MAX_LINE_SIZE];
    char command [MAX_LINE_SIZE];
    int output;
    
    printf("TEST::This is the filename: %s\n",fileName);

    //open or create file
    output=open(fileName, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if(output == -1)
    {
        printf("Error could not open file\n");
        return;
    }
    //Create message to send to server
    strcpy(send_line,"rrq  ");
    strcat (send_line,fileName);
    strcat (send_line,"\n\0");
    
    printf("TEST::The command being sent to server is: %s\n", send_line);

    n = strlen (send_line);

    printf("TEST::The size of command is: %d\n",n);

    if ((i = write_n (socket_fd, send_line, n)) != n)
    {
        printf ("write_n ERROR in send_message");
	    exit (1);
    }    
    //CODE WILL WORK UP TO HE IF THE REST IS COMMENTED OUT
    
  do
  {
        printf("TEST::After write, i: %d\n", i);

    	n = read_line (socket_fd, recv_line, MAX_LINE_SIZE);
        
        //If n == 0 there was no data recieved
        //If n > 0 there was data recieved could be file data or eof
        //If n < 0 there was an error
        
        printf("TEST::Command from server is: %s\n", recv_line);
        printf("TEST::Command lenght is: %d\n", n);

        if (n <= 0)
        {
        	printf ("read_line ERROR in send_message");
            exit (1);
        }
        fputs (recv_line, stdout);

        if (n > 0)
        {
            printf("TEST::%d\n", strcmp(recv_line,"eof"));  //should be 0 if they are the same
        }        

    
        //Get the first 5 characters which is the command
        strncpy(command,recv_line,5);

            
        //If data is being sent
        if(strncmp(command, "data",4)==0)  //THE REPLY WILL NOT ME "DATA", it will be the first line of the file
	    {
      		//copy data into file
            for(j=4; j<strlen(recv_line); j++)
            {       
               printf("TEST::in writing to file %c\n",recv_line[j]); n_char=write(output,recv_line[j],n_char);
                if(n_char<0 ){
                    printf ("ERROR: Failed to write to file \n");
                }
            }

            printf("TEST::After writing to file\n");
            //send server ack
            strcpy(send_line,"ack  \n\0");
            n = strlen (send_line);
            if ((i = write_n (socket_fd, send_line, n)) != n)
            {
                printf ("write_n ERROR in send_message");
                exit (1);
            }  
            printf("TEST::After ack sent\n");
        }
        //if over ten lines of data
        else if(strncmp(command, "fse",3)==0)
        {
            bzero(command, sizeof(command));
            printf("\n File over 10 Lines of data. Continue Y or N?");
            scanf("%s", command);
            //Error Check if command is recongized
            if((strcmp(command,"Y") != 0) &&(strcmp(command,"N") != 0)&&(strcmp(command,"y") != 0) &&(strcmp(command,"n") != 0))
	        {
	 	        do
		        {
                    bzero(command, sizeof(command)); 
                    printf("\n Please enter Y or N?");
                    scanf("%s", command);
                }while((strcmp ("Y",command) != 0) &&(strcmp ("N",command) != 0)&&(strcmp ("y",command) != 0) &&(strcmp ("n",command) != 0)); 
            }
            //send server cont message
            if((strcmp ("Y",command) == 0) ||(strcmp ("y",command) == 0))
            {
                //send server ack
                strcpy(send_line,"cont \n\0");
                n = strlen (send_line);
                if ((i = write_n (socket_fd, send_line, n)) != n)
                {
                    printf ("write_n ERROR in send_message");
                    exit (1);
                }  
            }   

            //send abort message
            else
            {
                //send server ack
                strcpy(send_line,"abort\n\0");
                n = strlen (send_line);
                if ((i = write_n (socket_fd, send_line, n)) != n)
                {
                    printf ("write_n ERROR in send_message");
                    exit (1);
                }  
                //wait for ack
                //then return
                n = read_line (socket_fd, recv_line, MAX_LINE_SIZE);
                if (n < 0)
                {
                    printf ("read_line ERROR in send_message");
                    exit (1);
                }
                fputs (recv_line, stdout);

                //Probaby a better way to get first five characters
                strncpy(command,recv_line,5);
                
                    
                if(strcmp(command, "ack  "))
                {
                    close(output);
                    //remove any data sent from server
                    int status = remove(fileName);
 
                    if( status == 0 )
                    {
                        printf("%s file deleted successfully.\n",fileName);
                    }
                    else
                    {
                        printf("Unable to delete the file\n");
                        perror("Error");
                        exit(1);
                    }
                    return;
                }
            }
        }

 }while(strncmp(command,"eof",3)!=0);
    
    if (ferror (fp))
    {
        printf ("message ERROR in send_message");
        exit (1);
    }
    close(output);
    return;
}

void write_file (FILE *fp, int socket_fd, char * fileName)
{
    
    printf("TEST::Beginning of write_file function\n"); ///TEST STATEMENT

    char send_line [MAX_LINE_SIZE];
	char recv_line [MAX_LINE_SIZE];
	int n = 0;
	int i = 0;
	int inFile = 0;
	
	
	//SEND wrq
    strcpy(send_line,"wrq\n\0"); //Create message to send to server
    n = strlen (send_line);  //lenght of the command
	
    printf("TEST::The command being sent to server is: '%s'\n", send_line);  ///TEST STATEMENT
	printf("TEST::The command size is %d\n", n); ///TEST STATEMENT
	printf("TEST::The socketFD is: %d\n", socket_fd);  //TEST STATEMENT
    
    if ((i = write_n(socket_fd, send_line, n)) != n) //send the wrq to the server
    {
		printf("Error: sending wrq to server\n");
		exit(1);
	}
	
	//READ for ack
	n = read_line(socket_fd, recv_line, MAX_LINE_SIZE); //get the response from the server
	
	printf("TEST::return size read_line is: '%d'\n", n);		    ///TEST STATEMENT
	printf("TEST::return message from read_line is: '%s'\n", recv_line); ///TEST STATEMENT

	if (n < 0) //we had an error
    {
		printf("Error: File return from server\n");
		exit (1);
    }
    else if (n == 0) //nothing in message
    {  
		printf("Server responded with nothing\n");
    }
	else //we got values back
	{
		printf("TEST::Server responeded with data\n");  ///TEST STATEMENT
		
		if(strncmp(recv_line, "ack", 3)==0) //the command was ack
		{
			printf("TEST::Server responeded with ack\n"); ///TEST STATEMENT
			
			inFile=open(fileName,O_RDONLY);  //open the file to send to server
        	if (inFile==-1)
        	{       
            		printf("Error: Could Not Open File /n");
            		exit(1);
        	}
			
			printf("TEST::File was opened, FD: %d\n", inFile);  ///TEST STATEMENT
			
			n = read_line(inFile, send_line, MAX_LINE_SIZE); //get the first line of the file
			if (n == 0) //the file is empty
			{
				if ((i = write_n(socket_fd, "eof\n\0", 5)) != 5) //send the wrq to the server
				{
					printf("Error: sending wrq to server\n");
					exit(1);
				}
				printf("eof sent\n");
				
				bzero(recv_line, MAX_LINE_SIZE);
				//get ack from server that they got eof
				n = read_line(socket_fd, recv_line, MAX_LINE_SIZE);
				
				printf("TEST::eof, read_line is: '%s'\n", recv_line);  ///TEST STATEMENT
				printf("TEST::eof, read_line size is: %d\n", n);    ///TEST STATEMENT
				
				if (n <= 0) //there was a read error
				{
					printf("Error: ack problem with eof send\n");
					exit(1);
				}
				else
				{
					if(strncmp(recv_line,"ack",3)==0)//we got ack from the server
					{
						printf("Server sent ack\n");
						exit(1);	
					}
					else
					{
						printf("Server sent a message that is not ack\n");
						exit(1);
					}
				}
			}
			else //the file has data
			{
				printf("TEST::Sending data to the server\n"); ///TEST STATEMENT
				printf("TEST::Sending data: '%s'\n", send_line); ///TEST STATEMENT
				
				strcat(send_line, "\0"); //add null to the end of the string
				if ((i = write_n(socket_fd, send_line, n+1)) != n+1) //send the wrq to the server
				{
					printf("Error: sending wrq to server\n");
					exit(1);
				}
				
				printf("TEST::Data was sent\n");
			}
		}
		else //the response was not ack
		{
			printf("Error server responded with wrong message\n");  
			exit(1);
		}
	}
	
    
    
    
    
    /*int i;
    int n;
    char send_line [MAX_LINE_SIZE];
    char recv_line [MAX_LINE_SIZE];

    while (fgets (send_line, MAX_LINE_SIZE, fp) != (char *) NULL)
    {
        n = strlen (send_line);
        if ((i = write_n (socket_fd, send_line, n)) != n)
        {
            printf ("write_n ERROR in send_message");
	    exit (1);
        }
        n = read_line (socket_fd, recv_line, MAX_LINE_SIZE);
        if (n < 0)
        {
            printf ("read_line ERROR in send_message");
            exit (1);
        }
        fputs (recv_line, stdout);
    }
    if (ferror (fp))
    {
        printf ("message ERROR in send_message");
        exit (1);
    }
*/
    return;
}
