#include "socketInclude.h"
#include "socketFunctions.h"

int main (int argc, char **argv)
{
    int socket_fd;
    int new_socket_fd;
    int addr_length;
    pid_t child_pid;
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;
    struct hostent *hp;
    char host_name [256];
 

    if ((socket_fd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf ("socket ERROR in main");
        exit (1);
    }

    memset (&server_addr, 0, sizeof (server_addr));
    server_addr.sin_family = AF_INET;
    gethostname (host_name, sizeof (host_name));
    hp = gethostbyname (host_name);
    if (hp == (struct hostent *) NULL)
    {
        printf ("gethostbyname ERROR in main: %s does not exist", host_name);
	exit (1);
    }
    memcpy (&server_addr.sin_addr, hp -> h_addr, hp -> h_length);
    server_addr.sin_port = htons (TCP_PORTNO);

    if (bind (socket_fd, (struct sockaddr *) &server_addr, sizeof (server_addr)) < 0)
    {
        printf ("bind ERROR in main");
	exit (1);
    }

    printf ("Parent: Server on port %d\n", ntohs (server_addr.sin_port));
    if (listen (socket_fd, 5) < 0)
    {
        printf ("listen ERROR in main");
        exit (1);
    }

    for ( ; ; )
    {
        printf ("Parent: Waiting for client\n");
        addr_length = sizeof (client_addr);
	new_socket_fd = accept (socket_fd, (struct sockaddr *) &client_addr, &addr_length);
	printf ("Parent: Client arrived\n");
        printf("Just before function call %d\n",new_socket_fd);
	if (new_socket_fd < 0)
	{
	    printf ("accept ERROR in main");
	    exit (1);
    }

        
        
       

        
        
        
        
        
        
        printf("Just before function call\n");
        
        message_echo (new_socket_fd);
        
        printf("after function call\n");
        
        
        
        
        
        
        
        close (new_socket_fd);
        
        
        
        
    }
}

void message_echo (int socket_fd)
{
    
    printf("at very beginning \n");
    int i;
    int n;
    int j=0;
    int n_char=0;
    int size;
    int inFile;
    char line [MAX_LINE_SIZE];
    char fileName [MAX_LINE_SIZE];
    char data [MAX_LINE_SIZE];
    char command [6];
    bzero(line, sizeof(line));
    bzero(fileName, sizeof(fileName));
    size_t len = 0;
    int count =1;
    
    for(;;){
        printf("Just before read line\n");
        n = read_line (socket_fd, line, MAX_LINE_SIZE);
        if (n == 0)
        {  
            break;
        }
        if (n < 0)
        {
           printf ("read_line ERROR in message_echo");
           exit (1);
        }
        for (i = 0; i < n; i ++)
        {
           printf ("%c", line [i]);
        }

        printf("The line %s %d",line, n);
    
    strncpy(command,line[0] ,5);
  
    
    printf("The command %s",command);
    //if read request
   /* if(strcmp(command, "rrq  ")==0){
        //copy data into file
        int t =0;
        printf("made it into rrq");
        for(j=4; j<strlen(line); j++)
        {       
            fileName[t]=line[j];
            t++;
        }
        printf("The filename %s",fileName);
        inFile=open(fileName,O_RDONLY);
        if (inFile==-1)
        {       
            printf("Error: Could Not Open File Name /n");
            return;
        }
        
        printf("Opened file /n" );
        
       /* fseek (inFile, 0, SEEK_END);
        size = ftell(inFile);

        if (0 == size) {
            printf("file is empty\n");
            //if file is empty return eof mesage
            strcpy(line, "eof   \0");
            write_n (socket_fd, line, n);
        }
        else{
            strcpy(line, "data ");
             while( (n_char=read(getline(&data, &len, inFile)) != -1))
            {
                 if(count == 11){
                     //send message to see if client wants to continue to recieve messages
                     strcpy(line, "fse   \0");
                     write_n (socket_fd, line, n);
                     n = read_line (socket_fd, line, MAX_LINE_SIZE);
                     if (n == 0)
                     {  
                        break;
                     }
                     if (n < 0)
                     {
                        printf ("read_line ERROR in message_echo");
                        exit (1);
                     }
                     
                     strcpy(command,line[0]);
                     strcat (command,line[1]);
                     strcat (command,line[2]);
                     strcat (command,line[3]);
                     strcat (command,line[4]);
                     
                     //if about then close connection with client
                     if(strcmp(command, "abort "))
                         return;
                     //otherwise continue with file transfer

                 }
                 
                strcat(line, data);
                n_char=write(socket_fd,line,n_char+5);
                count++;
                //wait for ack message
                for(;;){
                    n = read_line (socket_fd, line, MAX_LINE_SIZE);
                     if (n == 0)
                     {  
                        break;
                     }
                     if (n < 0)
                     {
                        printf ("read_line ERROR in message_echo");
                        exit (1);
                     }
                     
                     strcpy(command,line[0]);
                     strcat (command,line[1]);
                     strcat (command,line[2]);
                     strcat (command,line[3]);
                     strcat (command,line[4]);
                     if(strcmp(command, "ack "))
                         break;
                }
                 
                 
                
            }
        }
        
      
        
        

        
        
        
    }
    //write request
    else if(strcmp(command, "wrq  ")==0){
        
    }
    else{
        printf("Unknown Command");
        exit(1);
    }
    
      */
    
    
    
    
    
    }
    
    
    
    
    
    
    
    
    
    
    /*Orginal stuff
    int i;
    int n;
    char line [MAX_LINE_SIZE];
    bzero(line, sizeof(line));
    
    for ( ; ; )
    {
        n = read_line (socket_fd, line, MAX_LINE_SIZE);
        if (n == 0)
        {  
	    break;
        }
        if (n < 0)
        {
	    printf ("read_line ERROR in message_echo");
	    exit (1);
        }
        for (i = 0; i < n; i ++)
        {
	    printf ("%c", line [i]);
        }
        if (write_n (socket_fd, line, n) != n)
        {
            printf ("write_n ERROR in message_echo");
	    exit (1);
        }
    }*/
}
