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
#include <signal.h>


#define MAX_CONNECTED_USERS 10
#define POLL_WAITING_TIME 1000  // in ms 
#define NAME_SIZE 256
#define MAXIMUM_MSG_SIZE 2048 //2048 chars per message 

int thread_counter = 0;
//Create a pthread mutex so we can have exclusive control over the thread_counter
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//Create a pthread mutex to be used with the condition variable for reading msg buffer 
pthread_mutex_t cond_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_variable = PTHREAD_COND_INITIALIZER;
int condition = true;
int fd_pool[MAX_CONNECTED_USERS];

void handle_sigint(int sig){

  printf("SIGINT Detected... Skipping\n");
  printf("Type :exit to exit\n");

  char temp_msg[6];
  fgets(temp_msg,6,stdin);
  if(strcmp(temp_msg,":exit") == 0){
    for(int i=0; i<MAX_CONNECTED_USERS; i++){
      if(fd_pool[i] != -1){
          close(fd_pool[i]);
      }
    }
    exit(0);
  }
}

void sendMessage(int fd,char *name,char* str){
        send(fd,"\n",strlen("\n"),0);
        send(fd,str,strlen(str),0);
        //Add error checking to send syscalls
}

void * thread_worker(void *connfd){

  
  int fd = (int)(intptr_t)connfd;

  printf("Connection established with = %d\nWaiting for name...\n",fd);
  //+1 for null termination
  char name[NAME_SIZE+1];
  name[NAME_SIZE] = '\0';


  int bytes = recv(fd,name,NAME_SIZE,0);
  
  //if the server didn't receive a name send a message a new name 
  while(bytes <= 2){
    send(fd,"Please provide a correct username\n",strlen("Please provide a correct username\n"),0);
    //Clear previous name 
    memset(name,0,NAME_SIZE+1);
    bytes = recv(fd,name,NAME_SIZE,0);
  }


  //Format the string to remove \n character 
  char *delimiter = "\r";
  char *token = strtok(name,delimiter);
 
  //Ensure null termination on the name 
  *(token+strlen(name)) = '\0';
  
  printf("Name received = %s(%d bytes)\n",name,bytes);
  
  printf("Beginning chat...");

  char temporary_msg[MAXIMUM_MSG_SIZE+1];
  memset(temporary_msg,0,MAXIMUM_MSG_SIZE);
  
  

  int msgBytes = 0;
  while(1){
    // send(fd,name,strlen(name),0);
    // send(fd,">",strlen(">"),0);
    memset(temporary_msg,0,MAX_CONNECTED_USERS);
    msgBytes = recv(fd,temporary_msg,MAXIMUM_MSG_SIZE,0);
    if(msgBytes <= 2){
      continue;
    }

    printf("Message received = %s (%d bytes)",temporary_msg,msgBytes);
   
    

    
    for(int i=0; i<MAX_CONNECTED_USERS; i++){
      if(fd_pool[i] != -1 && fd != fd_pool[i]){
        sendMessage(fd_pool[i],name,temporary_msg);
      }
    }  
    
    memset(temporary_msg,0,MAXIMUM_MSG_SIZE+1);
  }

  for(int i=0; i<MAX_CONNECTED_USERS; i++){
    if(fd == fd_pool[i]){
      fd_pool[i] = -1;
      break;
    }
  }
  close(fd);

  printf("Closed connection!\n");

  //Critical region - can be edited by multiple threads/main at the same time causing race condition 
  pthread_mutex_lock(&mutex);
  thread_counter--;
  pthread_mutex_unlock(&mutex);

  return NULL; 
}

int main(int argc,char **argv){
  signal(SIGINT,handle_sigint);
  
  signal(SIGPIPE,SIG_IGN);
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
  int current_idx; 
  int thr_ret;

  for(int i=0; i<MAX_CONNECTED_USERS; i++){
    fd_pool[i] = -1;
  }

  while(1){
    
    if(poll(&fds,1,POLL_WAITING_TIME) == POLLIN){
      printf("Data incoming...\n");
      for(int i=0; i<MAX_CONNECTED_USERS; i++){
        //Search for a freed slot
        if(fd_pool[i] == -1){
          fd_pool[i] = accept(fd,NULL,NULL);
          current_idx = i;
          break;
        }
      }
      
      

      thr_ret = pthread_create(&pid[thread_counter],&thread_attr,thread_worker,(void*)(intptr_t)fd_pool[current_idx]);
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
