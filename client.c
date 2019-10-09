/* Works cited
 *Beej's Guide to network Programming
 https://www.geeksforgeeks.org/socket-programming-cc/
 https://www.geeksforgeeks.org/c-program-demonstrate-fork-and-pipe/
 https://stackoverflow.com/questions/15883568/reading-from-stdin
 https://www.youtube.com/watch?v=BIJGSQEipEE
  mostly these provided help with understanding sockets,pipes,forks
  While similar to our program they were just different enough
 * */
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/file.h>
#include <arpa/inet.h>
#include <netdb.h>
#define PORT 8080
//Main and only function
int main(int argc, char *argv[]){
  int i=1;
  int opt;
  char *port_num="8000";//here we arbitrarily set the port
  if(i == argc){
    printf("Error: possible segfault\n");
    printf( "Usage: ./client [-h] [-p port #]\n-h this help message\n-p # specify port number for server\n");
    return -1;
  }
  while((opt=getopt(argc,argv, "hp:")) != -1){
    switch (opt){
      case 'h':
        printf( "Usage: ./client [-h] [-p port #]\n-h this help message\n-p # specify port number for server\n");
        return -1;
      case 'p':
        port_num=optarg;
        break;
      case '?': 
        printf( "Usage: ./client [-h] [-p port #]\n-h this help message\n-p # specify port number for server\n");
    }
  }
  //various data structures that help with socket connections
  char buf[1024];
  char input[1024];
  char go_get[1024];
  int rbytes;
  struct sockaddr_in server;
  int sockfd;
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family= AF_INET;
  hints.ai_socktype= SOCK_STREAM;
  //in get addrinfo we can leave our port number as a string!
  getaddrinfo("senna.rhodes.edu", port_num, &hints, &res);
  if((sockfd=socket(AF_INET, SOCK_STREAM, 0))<0){
    perror("socket");
    exit(1);
  }
  server.sin_family = AF_INET;
  server.sin_port   = htons(8000); 
  server.sin_addr.s_addr= INADDR_ANY; //encountered probelms when I tried to specify an address

  //this will not connect if that is no server to connect to... 
  int connection_status=connect(sockfd, res->ai_addr, res->ai_addrlen);
  if(connection_status<0){
    perror("This is a failed connection");
    exit(1);
  }

  //same as all the other loops but I'll describe it better here
  //set two character strings to be able to fit most messages sent
  printf("connected to server...\n");
  while(1){
    memset(input,0,1024);
    long file;    //i don't know why this is long as opposed to int
    long reader;  //I saw it on stack overflow somewhere
    write(STDIN_FILENO,"> ", 3);
    if((file=read(STDIN_FILENO,input,1024))==-1){ //our first read, we take in the message from STDIN
      perror("read error");
      exit(1);
    }
    if ((reader=write(sockfd,input,1024))==-1){ //then we write it to our socket
      perror("write error");
      exit(1);
    }

    memset(go_get,0,1024);//got to reset our memory so we aren't writing garbage
    if((file=read(sockfd,go_get,1024))==-1){  //this reads from what the socket brings back
      perror("read from socket"); //so this will be what our server texts back
      exit(1);
    }
    if((reader=write(STDOUT_FILENO,go_get,1024))==-1){
      perror("writing error");
      exit(1);
    }
  }
  close(sockfd);
  return 0;
}
