// Andrew Roda, Myles Coleman
// 9/23/2023
// EECE 446
// Progra// Andrew Roda, Myles Coleman
// 9/23/2023
// EECE 446
// Program 1
// uses code from stream-talk-client.c

/* This code is an updated version of the sample code from "Computer Networks: A
 * Systems Approach," 5th Edition by Larry L. Peterson and Bruce S. Davis. Some
 * code comes from man pages, mostly getaddrinfo(3). */
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define JOIN_ACTION 0
#define PUBLISH_ACTION 1
#define SEARCH_ACTION 2
#define FETCH_ACTION 3
#define PUBLISH_DIRECTORY "SharedFiles"
/*
 * Lookup a host IP address and connect to it using service. Arguments match the
 * first two arguments to getaddrinfo(3).
 *
 * Returns a connected socket descriptor or -1 on error. Caller is responsible
 * for closing the returned socket.
 */
int lookup_and_connect(const char *host, const char *service);

int join(uint32_t id); 
int search(); 
int publish(); 

int main(int argc, char *argv[]) {
  char input[10]; 
  if(argc == 2){
    char* SERVER_PORT = argv[1];   
    if(atoi(SERVER_PORT) <= 2000 || atoi(SERVER_PORT) >= 65535){
      printf("Invalid port number.\n");
      exit(1);
  }
  else{
      printf("usage: %s port#\n");
      exit(1); 
  }
  char *host = "www.ecst.csuchico.edu";
  int s; // fd
  uint32_t peerid = 3789; 
  /* Lookup IP and connect to server */
  if ((s = lookup_and_connect(host, SERVER_PORT)) < 0) {
    exit(1);
  }
  while(strcmp(input,"EXIT") != 0){
    fgets(input, sizeof(input), stdin);
    char * p = strchr(input, '\n' );
    if (p) *p = '\0';
    if(strcmp(input, "JOIN") == 0){
      if(join(peerid) == 1){
        perror("Invalid\n"); 
        return 1; 
      }
    }
    elseif(strcmp(input,"SEARCH")){
      if(search() == 1){
        perror("Invalid\n"); 
        return 1; 
      }
    }
    elseif(strcmp(intput,"PUBLISH")){
      if(publish() == 1){
        perror("Invalid\n"); 
        return 1; 
      }
    }
    elseif(strcmp(input,"EXIT")){
    }
    else(){

    }     
  }
    

    
  /* Main loop: get and send lines of text */
  close(s);

  return 0;
  }  
  
  int lookup_and_connect(const char *host, const char *service) {
  struct addrinfo hints;
  struct addrinfo *rp, *result;
  int s;

  /* Translate host name into peer's IP address */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  if ((s = getaddrinfo(host, service, &hints, &result)) != 0) {
    fprintf(stderr, "stream-talk-client: getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }

  /* Iterate through the address list and try to connect */
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    if ((s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
      continue;
    }

    if (connect(s, rp->ai_addr, rp->ai_addrlen) != -1) {
      break;
    }

    close(s);
  }
  if (rp == NULL) {
    perror("stream-talk-client: connect");
    return -1;
  }
  
  freeaddrinfo(result);

  return s;
}
m 1
// uses code from stream-talk-client.c

/* This code is an updated version of the sample code from "Computer Networks: A
 * Systems Approach," 5th Edition by Larry L. Peterson and Bruce S. Davis. Some
 * code comes from man pages, mostly getaddrinfo(3). */
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_PORT "80"

/*
 * Lookup a host IP address and connect to it using service. Arguments match the
 * first two arguments to getaddrinfo(3).
 *
 * Returns a connected socket descriptor or -1 on error. Caller is responsible
 * for closing the returned socket.
 */
int lookup_and_connect(const char *host, const char *service);

int main(int argc, char *argv[]) {
  char *host = "www.ecst.csuchico.edu";
  int s;
  int len;
  int size;
  int pcounter = 0;
  char str[] = "GET /~kkredo/file.html HTTP/1.0\r\n\r\n";
  if (argc == 2) {
    size = atoi(argv[1]);

    if (size <= 3 || size > 1000) {
      exit(1);
    }
  } else {
    fprintf(stderr, "usage: %s host\n", argv[0]);
    exit(1);
  }
  char buf[size];
  /* Lookup IP and connect to server */
  if ((s = lookup_and_connect(host, SERVER_PORT)) < 0) {
    exit(1);
  }
  /* Main loop: get and send lines of text */
  if (send(s, str, strlen(str) + 1, 0) == -1) {
    perror("stream-talk-client: send");
    close(s);
    exit(1);
  }
  buf[size - 1] = '\0';
  char tag[] = "<p>";
  int recbytes = 1;
  int totalbytes = 0;
  while (recbytes != 0) {
    recbytes = recv(s, buf, size + 1, 0);
    if (recbytes == -1) {
      exit(1);
    }
    totalbytes = totalbytes + recbytes;
    int j = 0;
    for (int i = 0; i < recbytes; i++) {
      // printf("%c", buf[i]); //debug print text
      //  tag counter
      if (buf[i] == tag[j]) {
        j++;
      } else if (buf[i] != tag[j]) {
        if (j != 0) {
          if (buf[i] == tag[0]) {
            j = 1;
          } else {
            j = 0;
          }
        } else {
          j = 0;
        }
      }
      if (j == 3) {
        pcounter++;
        j = 0;
      }
    }
  } // while
  printf("Number of <p> tags: %d\n", pcounter);
  printf("Number of bytes: %d", totalbytes);
  close(s);

  return 0;
}

int lookup_and_connect(const char *host, const char *service) {
  struct addrinfo hints;
  struct addrinfo *rp, *result;
  int s;

  /* Translate host name into peer's IP address */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  if ((s = getaddrinfo(host, service, &hints, &result)) != 0) {
    fprintf(stderr, "stream-talk-client: getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }

  /* Iterate through the address list and try to connect */
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    if ((s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
      continue;
    }

    if (connect(s, rp->ai_addr, rp->ai_addrlen) != -1) {
      break;
    }

    close(s);
  }
  if (rp == NULL) {
    perror("stream-talk-client: connect");
    return -1;
  }
  freeaddrinfo(result);

  return s;
}
