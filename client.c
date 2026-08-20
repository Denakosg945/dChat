#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>


#define FALSE 0
#define TRUE 1
#define MAXIMUM_MESSAGE_SIZE 2048
#define NAME_SIZE 256

char name[NAME_SIZE+1];

int exit_var = FALSE; 

int isCiphered = 0;

void cipher(char *msg,int key){
  int length = strlen(msg);
  for(int i=0; i<length; i++){
    msg[i] = msg[i] ^ key;
  }
}


void *receiver_thread(void *connfd){
  int fd = (intptr_t) connfd;
  char msg[MAXIMUM_MESSAGE_SIZE+1];
  msg[MAXIMUM_MESSAGE_SIZE] = '\0';
  
  while(!exit_var){

    memset(msg,0,MAXIMUM_MESSAGE_SIZE);
    int receivedBytes = recv(fd,msg,MAXIMUM_MESSAGE_SIZE,0);
    

    if(receivedBytes >= 2){
      if(isCiphered != 0){
        cipher(msg,isCiphered);
      }
      printf("%s",msg);
    }
  }

}


int main(int argc,char **argv){
  
  if(argc < 3 ){
    fprintf(stderr,"Please enter a valid IP Address!\n");
    fprintf(stderr,"./client [IP Address] [Port] [Password](Optional)");
    exit(1);
  }

  if(argc == 4){
    if(atoi(argv[3]) >= 1 && atoi(argv[3]) <= 255){
      isCiphered = atoi(argv[3]);
    }
  }

  int ret;
  int fd;
  //Try to create a socket 
  ret = socket(AF_INET,SOCK_STREAM,0);

  if(ret == -1){
    perror("Failed to open socket...Try again later!\n");
    exit(2);
  }

  fd = ret;
  //Assign IP to the struct so we can use it 
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  ret = inet_pton(AF_INET,argv[1],&address.sin_addr);
  if(ret == 0){
    fprintf(stderr,"Not a valid IP Address\n");
    close(fd);
    exit(3);
  }else if(ret == -1){
    perror("inet_pton:");
    close(fd);
    exit(4);
  }
  address.sin_port = htons(atoi(argv[2]));
  

  //Attempt to connect to the server 
  ret = connect(fd,(struct sockaddr*)&address,sizeof(address));
  sleep(2);


  if(ret == -1){
    perror("connect:");
  }
  
  
  printf("Please enter your name: ");
  fgets(name,NAME_SIZE,stdin);
  name[NAME_SIZE] = '\0';
  *(name+strlen(name)-1) = '\0'; 
  
  char *temp_name = (char *)malloc(sizeof(char)*(strlen(name)+1));
  strcat(temp_name,name);
  strcat(temp_name,">\0");
  send(fd,temp_name,strlen(temp_name),0);

  free(temp_name);


  char msg[MAXIMUM_MESSAGE_SIZE+1];
  ssize_t msgBytes;
  //Chatting section
  //
  //Thread that listens to the socket and broadcasts any message received 
  pthread_t receiver;
  pthread_attr_t receiver_attr;
  pthread_attr_init(&receiver_attr);

  pthread_attr_setdetachstate(&receiver_attr,PTHREAD_CREATE_DETACHED);

  ret = pthread_create(&receiver,&receiver_attr,receiver_thread,(void *)(intptr_t)fd);
  if(ret != 0){
    fprintf(stderr,"Could not create thread...Exiting!\n");
    close(fd);
    exit(5);
  }

  char *str;
  while(!exit_var){
          memset(msg,0,MAXIMUM_MESSAGE_SIZE);
          fgets(msg,MAXIMUM_MESSAGE_SIZE,stdin);
     
          str = (char*)malloc(sizeof(char)*(strlen(name)+strlen(msg)+strlen(">"))-1);
          memset(str,0,sizeof(char)*(strlen(name)+strlen(msg)+strlen(">"))-1);
          strncat(str,name,strlen(name));
          strcat(str,">");
          strncat(str,msg,strlen(msg));
          send(fd,str,strlen(str),0);
          
          free(str);
        
  }
  

  return 0;
}



