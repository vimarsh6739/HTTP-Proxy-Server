#include "proxy_parse.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

//Function to parse incoming message from accept(socket())
void parse(int sock){
  //In child process for server

  int n; //ret value for read and write calls
  char buffer[8192]; //8 KB buffer for messages
  
  bzero(buffer,8192);
  n = read(sock,buffer,8191);
  if(n<0) error("[-]Couldnt read from socket");
  
  printf("Received query:\n %s",buffer);
  //Parse message in buffer : TODO
  
  //Return result of GET query :TODO
  
}

int main(int argc, char * argv[]) {

  int sockfd; //fd of original socket
  int newsockfd;  //unique fd to client
  int portno; //server port
  int clilen; //length of client process
  int pid; //id of child process

  struct sockaddr_in serv_addr; //server address 
  struct sockaddr_in cli_addr;  //client address 

  if (argc < 2) {
    error("ERROR, no port provided\n");
  }

  //init socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) error("ERROR opening socket");
  //set server addr to 0 - like memset
  bzero((char *) &serv_addr, sizeof(serv_addr));
  //init server_addr
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  //Bind socket
  if (bind(sockfd, (struct sockaddr *) &serv_addr,
      sizeof(serv_addr)) < 0) 
      error("ERROR on binding");
  //listen with wait queue of 5 clients max
  listen(sockfd,5);
  clilen = sizeof(cli_addr);
  
  while(1){
    //Accept-blocking fifo semantics
    newsockfd = accept(sockfd,(struct sockaddr* ) &cli_addr, (socklen_t*)&clilen);
    if(newsockfd < 0){
      error("[-]Couldnt accept incoming connection");
    }

    pid = fork();
    
    if(pid < 0){
      error("[-]error on forking new process");
    }

    if(pid==0){
      //in child
      //close the original socket connection. Communicate only using newsockfd
      close(sockfd);  
      parse(newsockfd);
      exit(0);
    }
    else {
      //in parent
      close(newsockfd);
    }
  }

  //unreachable statement
  return 0;

}