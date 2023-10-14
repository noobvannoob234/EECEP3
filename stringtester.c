#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(){
  char test[15]; 
  fgets(test, sizeof(test), stdin);
  char * p = strchr(test, '\n' );
  if (p) *p = '\0'; 
  if(strcmp(test,"TEST") == 0){
    printf("valid\n"); 
  }
  fgets(test, sizeof(test), stdin);
  p = strchr( test, '\n' );
  if (p) *p = '\0'; 
  char buf[8];
  int index = (int)(p - test); 
  buf[0] = ' '; 
  memcpy(buf+ 1, test, index);
  if(strcmp(buf," TEST2") == 0){
    printf("valid2\n"); 
  }

  return 0;
}