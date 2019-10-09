/*
 * server.c - a chat server (and monitor) that uses pipes and sockets
 */
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
// constants for pipe FDs
#define WFD 1
#define RFD 0

char pidstr[256];

/*
 * monitor - provides a local chat window
 * @param srfd - server read file descriptor
 * @param swfd - server write file descriptor
 */
void monitor(int srfd, int swfd) {
  // implement me
  char input[1024];
  char go_get[1024];
  //same loop as before but now we have to specify our STDOUT and such
  while(1){
    memset(input,0,1024);
    long file;
    long reader;
    
    if ((file = read(srfd,input,1024))==0){
      exit(1);//checking whether file empty
    } 
    write(STDOUT_FILENO,"> ", 3);
    if((reader=write(STDOUT_FILENO,input, 1024))==-1){
      perror("write");
      exit(1);
    }
    memset(go_get,0,1024);
    if((file=read(STDIN_FILENO,go_get,1024))==0){
      exit(1);
    }
    if((reader=write(swfd,go_get,1024))==-1){
      perror("server write error");
      exit(1);
    }

  }
}



/*
 * server - relays chat messages
 * @param mrfd - monitor read file descriptor
 * @param mwfd - monitor write file descriptor
 * @param portno - TCP port number to use for client connections
 */
void server(int mrfd, int mwfd, int portno) {
  int val = 1;
  char input[1024];
  char go_get[1024];
  int server_fd,new_sock;
  struct sockaddr_in server, client;
  int addrlen = sizeof(server);
  if((server_fd=socket(AF_INET,SOCK_STREAM,0)) == 0){
    perror("socket issue");
    exit(1);
  }
  
  if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &val, sizeof(val))){
    perror("setsockopt\n");
    exit(1);
  }
  server.sin_family=AF_INET;
  server.sin_addr.s_addr=INADDR_ANY;
  server.sin_port=htons(portno);

  if(bind(server_fd, (struct sockaddr *)&server, sizeof(server))<0){  
    perror("Bind issue\n");
    exit(1);
  } 
  //Listen allows for multiple clients to connect so I set it at 3
  if(listen(server_fd,3) < 0){
    perror("Listen issue\n");
    exit(1);
  }
 // addr_size=sizeof(struct sockaddr_in);
  if((new_sock = accept(server_fd, (struct sockaddr *)&server, (socklen_t*)&addrlen))<0){
    perror("accept error\n");
    exit(1);
  }
  //This loop is repeated three other times while only interchanging the
  //various file descriptors
  while(1){
    memset(input,0,1024);
    long file;
    long reader;
    if((file=read(new_sock,input,1024))==-1){
      perror("read error>>>");      //probably could have done better error messages 
      exit(1);
    }
    if((reader=write(mwfd,input,1024))==-1){
      perror("write error<<<>>>");
      exit(1);
    }
    memset(go_get,0,1024);
    if((file=read(mrfd,go_get,1024))==-1){
      perror("read error!!!!");
      exit(1);
    }
    if((reader=write(new_sock,go_get,1024))==-1){
      perror("write error!!@@##");
      exit(1);
    }
  }  
  //add closes for files
  close(server_fd);
  close(new_sock);
  close(mrfd);
  close(mwfd);
}


//This is the main function...obviously
int main(int argc, char **argv) {
  // implement me
  int mfds[2], ret;   //monitor pipes
  int sfds[2];    //server pipes
  int i=1;
  int opt;
  int port_num;
  if(i == argc){
    printf("Error: possible segfault\n");
    printf( "Usage: ./server [-h] [-p port #]\n-h this help message\n-p # specify port number for server\n");
    return -1;
  }
  while((opt=getopt(argc,argv, "hp:")) != -1){
    switch (opt){
      case 'h':
        printf( "Usage: ./server [-h] [-p port #]\n-h this help message\n-p # specify port number for server\n");
        return -1;
      case 'p':
        port_num=atoi(optarg);
        break;
      case '?': 
        printf( "Usage: ./server [-h] [-p port #]\n-h this help message\n-p # specify port number for server\n");
    }
  }

  if(pipe(mfds)==-1){
    perror("Pipe1 error");
    exit(1);
  }
  if(pipe(sfds)==-1){
    perror("Pipe2 error");
    exit(1);
  }

  pid_t p = fork();
  if(p == -1){
    perror("fork error");
    exit(1);
  } else if(p>0){
    //parent process
    close(mfds[RFD]);
    close(sfds[WFD]);
    server(sfds[RFD], mfds[WFD], port_num);    
    close(sfds[RFD]);
    close(mfds[WFD]);
    wait(NULL); 
  } else{
    //child process
    close(sfds[RFD]);
    close(mfds[WFD]);
    monitor(mfds[RFD], sfds[WFD]);
    close(mfds[RFD]);
    close(sfds[WFD]);
    exit(1);
  }

}
