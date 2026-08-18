#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>


#define MAX_CONNECTED_USERS 10



int main(int argc,char **argv){

  pthread_t pid[MAX_CONNECTED_USERS];

  //Create a return value variable to store return values to check validity
  int ret;
  ret = socket(AF_INET,SOCK_STREAM,0);

  if(ret == -1){
    perror("Failed to open socket...Try again later!\n");
    exit(2);
  }
  
  int fd = ret;

  printf("IPv4 Socket created!\n");
  
  //Create a struct to bind socket to IP Address and port 
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  if(argc == 2){
    printf("Manual address = %s\n",argv[1]);
    address.sin_addr.s_addr = atoi(argv[1]);
  }else {
    address.sin_addr.s_addr = INADDR_ANY;
  }

  if(argc == 3){
    printf("Manual port = %s\n",argv[2]);
    address.sin_port = htons(atoi(argv[2]));
  }else{
    address.sin_port = htons(8080);
  }

  ret = bind(fd,(struct sockaddr *)&address,sizeof(address));

  if(ret == -1){
    perror("Failed to bind socket to IP Address\n");
    close(fd);
    exit(3);
  }

  printf("Address binded successfully!\n");
 
  ret = listen(fd,MAX_CONNECTED_USERS);

  if(ret == -1){
    perror("Could not start listening...Exiting!\n");
    close(fd);
    exit(4);
  }

  


  return 0;

}
