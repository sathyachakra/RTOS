#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <arpa/inet.h>

#define PORT "3490"
#define MAXDATASIZE 100
int sockfd,sock_set_flag=0; //make socket global to access from signal handler

void sig_handler()          //signal handler
{
	char c;
	printf("Do you really want to exit?[y/n]\n");
	scanf("%c",&c);
	if(c=='y' || c=='Y')
	{
		if(sock_set_flag)
			send(sockfd,"EXIT",4,0);     //send exit message to server
		exit(0);
	}
}
void *get_in_addr(struct sockaddr *sa)   //ipv4 or v6
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc,char *argv[])
{
	int numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;  //type of addr, server addr, iterator
	int ret_val;
	char s[INET6_ADDRSTRLEN],buffer[250];

	signal(SIGINT,sig_handler);            //bind signal handler to Ctrl-C

	if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }
    bzero(&hints,sizeof(hints));           //initialize to zero
    hints.ai_family = AF_UNSPEC;           //any address fam
    hints.ai_socktype = SOCK_STREAM;       //tcp

    if((ret_val = getaddrinfo(argv[1],PORT,&hints,&servinfo))!=0)   //error in getting address
    {
    	fprintf(stderr, "getaddrinfo:%s\n", gai_strerror(ret_val));
    	return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next)    //find suitable address
    {
    	if((sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1)
    	{
    		perror("client:socket");
    		continue;
    	}

    	if(connect(sockfd,p->ai_addr,p->ai_addrlen) == -1) //error in connection
    	{
    		close(sockfd);
    		perror("client:connect");
    		continue;
    	}

    	break;         //if connection made successfully, break
    }

    if(p==NULL)        //if no connection
    {
    	fprintf(stderr, "client:failed to connect\n");
    	return 2;
    }
    sock_set_flag = 1;
    inet_ntop(p->ai_family,get_in_addr((struct sockaddr *)p->ai_addr),s,sizeof(s)); //addr to string
    printf("client:connecting to %s\n", s);

    freeaddrinfo(servinfo); //server info no longer reqd

    while(1)
    {
    	printf("\nyou:\t");
	    //scanf("%s",buffer);
        fgets(buffer,250,stdin);    //get from keyboard the value to send
        //fflush(stdin);
        printf("%s:%ld",buffer,strlen(buffer));
	    if(send(sockfd,buffer,strlen(buffer),0)==-1)   //send value
	    {
	    	perror("client:send");
	    	exit(1);
	    }
	    printf("message sent\n");

    }

    close(sockfd);
    return 0;
}