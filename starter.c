#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define MAX_IP_LEN 15
#define MAX_PORT_LEN 5



void startServer(int argc, char** argv){
  while(getchar() != '\n');
  printf(ANSI_COLOR_RED "Select:\n" ANSI_COLOR_RESET);

  printf(ANSI_COLOR_CYAN "a)" ANSI_COLOR_RESET " Auto IP\n" );
  printf(ANSI_COLOR_MAGENTA "b)" ANSI_COLOR_RESET " Select IP and Port\n");
  printf(ANSI_COLOR_RED "c)" ANSI_COLOR_RESET " Select IP, Port and password\n");  
  char a = '\0';
  while(a == '\0'){
    printf(">");
    a = getchar();
    if(a < 'a' || a > 'c'){
      printf("Enter a valid character from a-b\n");
      a = '\0';
      while(getchar() != '\n');
    }
  
  }
  int ret;
  if(a == 'a'){
      ret = execvp("./server",argv);
      if(ret == -1){
        perror("execvp");
        exit(1);
      }
  }

  
  char **custom_argv = (char **)calloc(5,sizeof(char *));
  
  while(getchar() != '\n');
  char temp_IP[MAX_IP_LEN+1];
  char temp_port[MAX_PORT_LEN+1];
  unsigned char temp_key[4]; // 0-255 + '\0'
  temp_key[3] = '\0';
  printf("Enter IP Address!\n>");
  fgets(temp_IP,MAX_IP_LEN,stdin);
  temp_IP[MAX_IP_LEN] = '\0';
  printf("Enter port!\n>");
  fgets(temp_port,MAX_PORT_LEN,stdin);
  temp_port[MAX_PORT_LEN] = '\0';
  
  while(getchar() != '\n');
  if(a == 'c'){
    printf("Enter password!\n>");
    fgets(temp_key,3,stdin);
    temp_key[strlen(temp_key)] = '\0';
    custom_argv[3] = strdup(temp_key);
  }else{
    custom_argv[3] = "0";
  }
  

  custom_argv[0] = "./server";
  custom_argv[1] = strdup(temp_IP);
  custom_argv[2] = strdup(temp_port);
  custom_argv[4] = NULL;
  
  *(custom_argv[1]+strlen(temp_IP) - 1) = '\0';


  execvp("./server",custom_argv);

  
  perror("execvp");

  for(int i=0 ;i<4; i++){
    free(custom_argv[i]);
  }
  free(custom_argv);


  exit(1);


}

void connectServer(){

  char **custom_argv = (char **)calloc(5,sizeof(char *));
  
  printf("Connect to server\n");
  printf(ANSI_COLOR_RED "Select:\n" ANSI_COLOR_RESET);

  printf(ANSI_COLOR_CYAN "a)" ANSI_COLOR_RESET " No password\n" );
  printf(ANSI_COLOR_MAGENTA "b)" ANSI_COLOR_RESET " with password\n");
  
  while(getchar() != '\n');
  char a = '\0';
  while(a == '\0'){
    printf(">");
    a = getchar();
    if(a < 'a' || a > 'c'){
      printf("Enter a valid character from a-b\n");
      a = '\0';
      while(getchar() != '\n');
    }
  }
  



  while(getchar() != '\n');
  char temp_IP[MAX_IP_LEN+1];
  char temp_port[MAX_PORT_LEN+1];
  unsigned char temp_key[4]; // 0-255 + '\0'
  temp_key[3] = '\0';
  printf("Enter IP Address!\n>");
  fgets(temp_IP,MAX_IP_LEN,stdin);
  temp_IP[MAX_IP_LEN] = '\0';
  printf("Enter port!\n>");
  fgets(temp_port,MAX_PORT_LEN,stdin);
  temp_port[MAX_PORT_LEN] = '\0';


  custom_argv[0] = "./client";
  custom_argv[1] = strdup(temp_IP);
  custom_argv[2] = strdup(temp_port);
  if(a == 'b'){
    printf("Enter password!\n>");
    fgets(temp_key,3,stdin);
    temp_key[strlen(temp_key)] = '\0';
    custom_argv[3] = strdup(temp_key);
    while(getchar() != '\n');
  }else{
    custom_argv[3] = "0";
  }
  custom_argv[4] = NULL;

  *(custom_argv[1]+strlen(temp_IP) - 1) = '\0';

  execvp("./client",custom_argv);

  perror("execvp");
  

  for(int i=0 ;i<4; i++){
    free(custom_argv[i]);
  }
  free(custom_argv);
  
  exit(1);

}


int main(int argc,char **argv){
  printf("Welcome to " ANSI_COLOR_RED "DChat\n" ANSI_COLOR_RESET);
  printf(ANSI_COLOR_GREEN "MENU:\n" ANSI_COLOR_RESET);
  printf(ANSI_COLOR_CYAN "a)" ANSI_COLOR_RESET " Start server\n" );
  printf(ANSI_COLOR_MAGENTA "b)" ANSI_COLOR_RESET " Join server\n");
  printf(ANSI_COLOR_BLUE "c)" ANSI_COLOR_RESET " Information\n");


  char *information_msg = "This is a experimental app developed to learn about sockets/multithreading.\n"
  "You can use the first option (a) to create a server with default settings or with a IP and a Port.\n"
  "The second option (b) allows you join a server created with the option (a) using an IP and a Port.\n";


  char a = '\0';
  while(a == '\0'){
    printf(">");
    a = getchar();
    if(a < 'a' || a > 'c'){
      printf("Enter a valid character from a-c\n");
      a = '\0';
      while(getchar() != '\n');
    }


  }
  
  switch(a){
    case 'a':
      startServer(argc,argv);
      break;
    case 'b': 
      connectServer();
      break;
    case 'c':
      printf("%s",information_msg);
      break;
  }

}
