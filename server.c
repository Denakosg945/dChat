#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <string.h>
#include <stdint.h>


#define MAX_CONNECTED_USERS 10
#define POLL_WAITING_TIME 1000  // in ms 

int thread_counter = 0;
//Create a pthread mutex so we can have exclusive control over the thread_counter
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void * thread_worker(void *connfd){

  
  int fd = (int)(intptr_t)connfd;

  printf("Connection established with = %d",fd);
  
  
  close(fd);

  printf("Closed connection!\n");
  
  //Critical region - can be edited by multiple threads/main at the same time causing race condition 
  pthread_mutex_lock(&mutex);
  thread_counter--;
  pthread_mutex_unlock(&mutex);

  return NULL; 
}

int main(int argc,char **argv){

  pthread_t pid[MAX_CONNECTED_USERS];
  //Create a return value variable to store return values to check validity
  int ret;
  
  //Set pthread attributes
  pthread_attr_t thread_attr;
  
  
  ret = pthread_attr_init(&thread_attr);
    
  if(ret != 0){
    fprintf(stderr,"pthread_attr_init: %s (%d)\n",strerror(ret),ret);
    exit(100);
  }

  ret = pthread_attr_setdetachstate(&thread_attr,PTHREAD_CREATE_DETACHED);

  if(ret != 0){
    fprintf(stderr,"pthread_attr_setdetachstate: %s (%d)\n",strerror(ret),ret);
    exit(101);
  }
  
  

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

  printf("Listening for incoming connections...\n");

  //Create a poll struct so we can monitor when a connection arrives
  //

  struct pollfd fds;
  fds.fd = fd;
  fds.events = POLLIN;
  //Make variable to store and transfer connection file descriptor to thread_worker 
  int connfd; 
  int thr_ret;

  while(1){
    
    if(poll(&fds,1,POLL_WAITING_TIME) == POLLIN){
      printf("Data incoming...\n");
      connfd = accept(fd,NULL,NULL);
      if(connfd == -1){
        perror("Failed to accept connection...\n");
        continue;
      }
      

      thr_ret = pthread_create(&pid[thread_counter],&thread_attr,thread_worker,(void*)(intptr_t)connfd);
      if(thr_ret != 0){
        fprintf(stderr,"pthread_create: %s (%d)",strerror(thr_ret),thr_ret);
        continue;
      }  
      
      pthread_mutex_lock(&mutex);
      thread_counter++;
      pthread_mutex_unlock(&mutex);
      printf("Thread created\n");


    }
    


  }  

  pthread_mutex_destroy(&mutex);

  return 0;

}
