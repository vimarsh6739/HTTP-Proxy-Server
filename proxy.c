#include "proxy_parse.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h>
#include <sys/wait.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

//Function to parse incoming message from accept(socket())
int parse(int cli_sock){

  int n; //ret value for read() and write()
  char buffer[65536]; //request buffer
  char buf_temp[65536];
  int req_len=0;
  memset(&buffer,0,65536);
  memset(&buf_temp,0,65536);

  n = read(cli_sock,buf_temp,65535);
  if(n<0) error("Couldn't read from socket");
  req_len = strlen(buf_temp);	
  
  sprintf(buffer+strlen(buffer),buf_temp);

  //split request- client has to send more information
  if(req_len > 0 && strcmp(&buf_temp[req_len-4],"\r\n\r\n")!=0){
  	//read the remaining request
	memset(&buf_temp,0,65536);
	n = read(cli_sock,buf_temp,65535);
	if(n<0) error("Couldn't read from socket");
	req_len += strlen(buf_temp);
	sprintf(buffer+strlen(buffer),buf_temp);
  }
  else {
  	//entire get request is received
  }
  
  //Parse request
  struct ParsedRequest*  req = ParsedRequest_create();
  if (ParsedRequest_parse(req, buffer, req_len) < 0) {
    memset(&buffer,0,8192);
    sprintf(buffer,"HTTP/1.0 400 Bad Request\r\n\r\n");
    n = write(cli_sock,buffer,strlen(buffer));
    if(n<0)error("Couldn't write to socket");
    return -1;
  }

  //Send GET request to actual server-proxy acts as client now
  int sockfd; 
  int portno; 
  struct sockaddr_in serv_addr_actual;  //address of server
  struct hostent *server_actual;  //host computer of server

  if(req->port == NULL){
    portno = 80;
  }
  else{
    portno = strtol(req->port,(char **)NULL,10);
  }

  sockfd = socket(AF_INET,SOCK_STREAM,0);
  if(sockfd < 0)error("Couldn't open socket in child proc");
  
  //gethostbyname() queries databases around the country
  server_actual = gethostbyname(req->host);

  if(server_actual==NULL){
    //404!
    memset(&buffer,0,65536);
    sprintf(buffer,"HTTP/1.0 404 Not Found\r\n\r\n");
    n = write(cli_sock,buffer,strlen(buffer));
    if(n<0)error("[-]Couldn't write to socket");
    return -1;
  }
  //set fields of address
  memset(&serv_addr_actual, 0,sizeof(serv_addr_actual));
  serv_addr_actual.sin_family = AF_INET;
  memcpy(&serv_addr_actual.sin_addr.s_addr,server_actual->h_addr,server_actual->h_length);
  // bcopy((char *)server_actual->h_addr,(char *)&serv_addr_actual.sin_addr.s_addr,server_actual->h_length);
  serv_addr_actual.sin_port = htons(portno);

  //establish connection to og server
  if(connect(sockfd,(struct sockaddr *)&serv_addr_actual
  ,sizeof(serv_addr_actual)) < 0){
    error("Couldn't establish connection to server");
  }

  //Modify request to server 
  
  n = ParsedHeader_set(req,"Host",req->host);
  if(n < 0){error("parse set host failed");}
  
  n = ParsedHeader_set(req,"Connection","close");
  if(n < 0){error("parse_set connection failed");}

  //length of modified request
  int mod_len = ParsedRequest_totalLen(req);
  char* mod_buf = (char*)malloc(mod_len+1);
  if(ParsedRequest_unparse(req,mod_buf,mod_len) < 0){
    error("unparse failed");
  }
  mod_buf[mod_len] = '\0';

  //Write to og server
  n = write(sockfd,mod_buf,mod_len);
  if (n < 0){
    error("couldn't write to original server");
  }
	
  //Read result from og server-enclose it in a loop until the entire message is read
 	//char buffer2[65535];
	//int length_buf = 0;
  //Simaltaneously read and write.
	while(1){
	memset(&buffer,0,65536);
  	n = read(sockfd,buffer,65535);
  	if(n<0){
    	error("couldn't read from original server");
			break;
  	}
	if(n==0){
		break;
	}
    n = write(cli_sock,buffer,strlen(buffer));
    if(n<0){
      error("couldn't write to client");
    }
		// length_buf += sprintf(buffer + length_buf,buffer2);
	} 
 
  //Close og server side socket
  // close(sockfd);
  //Return result of GET query to client
  /*n = write(cli_sock,buffer,strlen(buffer));
  if(n<0){
    error("couldn't write to client");
  }*/
  //Close client side socket
  // close(cli_sock);
  return 0;
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
    error("error, no port provided\n");
  }

  //init socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) error("error opening socket");
  //set server addr to 0 - like memset
  // bzero((char *) &serv_addr, sizeof(serv_addr));
  memset(&serv_addr,0,sizeof(serv_addr));
  //init server_addr
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  //Bind socket
  if (bind(sockfd, (struct sockaddr *) &serv_addr,
      sizeof(serv_addr)) < 0) 
      error("error on binding");
  //listen with wait queue of 5 clients max
  listen(sockfd,5);
  clilen = sizeof(cli_addr);
  
  while(1){
    //Accept-blocking fifo semantics
    newsockfd = accept(sockfd,(struct sockaddr* ) &cli_addr, (socklen_t*)&clilen);
    if(newsockfd < 0){
      error("Couldn't accept incoming connection");
    }

    pid = fork();
    
    if(pid < 0){
      error("error on forking new process");
    }

    if(pid==0){
      //in child
      //close the original socket connection. Communicate only using newsockfd
      close(sockfd);  
      int rv = parse(newsockfd);
      if(rv==0);
      exit(0);
    }
    else {
      //in parent
      //sleep(1);
      //Might cause problems in retrieval
      close(newsockfd);
    }
  }

  //unreachable statement
  return 0;

}
