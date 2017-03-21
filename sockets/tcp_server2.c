#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAXDATASIZE 100

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10     // how many pending connections queue will hold

void *get_in_addr(struct sockaddr *sa)					//get ipv4 or ipv6 address
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);	//ipv4 addr
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);	//ipv6 addr
}

int main(int argc, char const *argv[])
{
	int sockfd,clifd;						//socket fd's
	struct addrinfo hints,*servinfo,*p;		//type of addr, server addr, iterator
	struct sockaddr_storage cliaddr;		//client socket
	socklen_t cli_size;						//clint size
	//struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int ret_val;
	int num_bytes_recv;
	char buffer[250];						//buffer to store data

	memset(&hints,0,sizeof(hints));			//initialize to 0
	hints.ai_family = AF_UNSPEC;			//any address family
	hints.ai_socktype = SOCK_STREAM;		//tcp
	hints.ai_flags = AI_PASSIVE;

	if((ret_val = getaddrinfo(NULL,PORT,&hints,&servinfo))!=0)		//error in getting addr
	{
		fprintf(stderr, "getaddrinfo:%s\n", gai_strerror(ret_val));
		return 1;
	}

	for(p=servinfo; p!=NULL ; p= p->ai_next) //find free address among given addresses
	{
		if((sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1)	//create socket
		{
			perror("server: socket");
			continue;
		}

		if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))==-1)		//set socket options
		{
			perror("setsockopt");
			exit(1);
		}
		//printf("socket:%d\n",sockfd );
		if(bind(sockfd,p->ai_addr,p->ai_addrlen)==-1)							//vind socket
		{
			close(sockfd);
			perror("server:bind");
			continue;
		}
		break;
	}

	if(p==NULL)	//if no address free
	{
		fprintf(stderr, "server:failed to bind\n");
		exit(1);
	}

	if(listen(sockfd,BACKLOG)==-1)
	{
		perror("listen");
		exit(1);
	}
	printf("waiting for connections\n");

	while(1)
	{
		cli_size = sizeof(cliaddr);
		if((clifd = accept(sockfd,(struct sockaddr*)&cliaddr,&cli_size))==-1)	//accept connection
		{
			perror("accept");
			continue;
		}
		
		inet_ntop(cliaddr.ss_family,
            get_in_addr((struct sockaddr *)&cliaddr),
            s, sizeof s);									//change to char string. store in "s"
		printf("server got connection from %s\n", s);

		if(!fork())											//child
		{
			close(sockfd);									//sockfd no longer necessary
			while(1)
			{
				printf("server:\t");
				fgets(buffer,250,stdin);
				if(send(clifd,buffer,strlen(buffer),0)==-1)   //send value
			    {
			    	perror("server:send");
			    	exit(1);
			    }
			    printf("message sent\n");
			}
			/*while(1)
			{
				if((num_bytes_recv = recv(clifd,buffer,MAXDATASIZE-1,0))==-1)	//receive data
				{
					perror("recv");
					exit(1);
				}
				buffer[num_bytes_recv]='\0';				//terminate buffer
				printf("client:\t%s\n", buffer);			
				if(!strcmp(buffer,"EXIT"))					//exit if message is "EXIT"
				{
					close(clifd);
					printf("killing child\n");
					exit(0);printf("child killed.\n");
				}
			}*/
		}


	}
	return 0;
}