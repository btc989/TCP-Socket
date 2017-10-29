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
	if (new_socket_fd < 0)
	{
	    printf ("accept ERROR in main");
	    exit (1);
        }

        if ((child_pid = fork ()) < 0)
        {
            printf ("fork ERROR in main");
            exit (1);
        }
        else if (child_pid == 0)
        {
            printf ("Child: Fork OK\n");
            close (socket_fd);
            message_echo (new_socket_fd);
            printf ("Child: Done\n");
            exit (0);
        }
        else
        {
            close (new_socket_fd);
        }
    }
}

void message_echo (int socket_fd)
{
    int i;
    int n;
    char line [MAX_LINE_SIZE];

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
    }
}
