// Andrew Roda, Myles Coleman
// 10/13/2023
// EECE 446
// Program 2

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
#include <dirent.h>
#define PUBLISH_DIRECTORY "SharedFiles"
#define BUFFER_SIZE 200
/*
 * Lookup a host IP address and connect to it using service. Arguments match the
 * first two arguments to getaddrinfo(3).
 *
 * Returns a connected socket descriptor or -1 on error. Caller is responsible
 * for closing the returned socket.
 */
int lookup_and_connect(const char *host, const char *service);

int join(uint32_t id, int s, char *buf);
int search(int s, char *buf);
int publish(int s, char *buf);

int main(int argc, char *argv[]) {
  char input[10], buf[BUFFER_SIZE];

  if (argc != 2) {
    printf("usage: port#\n");
    exit(1);
  }
  char *port = argv[1];
  if (atoi(port) <= 2000 || atoi(port) >= 65535) {
    printf("Invalid port number.\n");
    exit(1);
  }

  char *host = "www.ecst.csuchico.edu";
  int s; // fd
  uint32_t peerid = 3789;
  /* Lookup IP and connect to server */
  if ((s = lookup_and_connect(host, port)) < 0) {
    exit(1);
  }
  while (strcmp(input, "EXIT") != 0) {
    if (fgets(input, sizeof(input), stdin) == NULL) {
      printf("Fgets error\n");
      break;
    }
    char *p = strchr(input, '\n');
    if (p)
      *p = '\0';
    if (strcmp(input, "JOIN") == 0) {
      if (join(peerid, s, buf) == 1) {
        perror("Invalid\n");
        return 1;
      }
    } else if (strcmp(input, "PUBLISH") == 0) {
      if (publish(s, buf) == 1) {
        perror("Invalid\n");
        return 1;
      }
    } else if (strcmp(input, "SEARCH") == 0) {
      if (search(s, buf) == 1) {
        perror("Invalid\n");
        return 1;
      }
    } else if (strcmp(input, "EXIT") == 0) {
      printf("exiting\n");
    } else {
      printf("Invalid input\n");
    }
  }
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

/* send join request to server */
int join(uint32_t id, int s, char *buf) {
  id = htonl(id);
  buf[0] = 0;
  memcpy(buf + 1, &id, 4);
  if (send(s, buf, 5, 0) == -1) {
    perror("p2p peer: send");
    close(s);
    return 1;
  }
}
int publish(int s, char *buf) {
  int length = 0; 
  DIR *d;
  struct dirent *dir;
  uint32_t ncount = 0;

  d = opendir(PUBLISH_DIRECTORY);
  if (d) {
    while ((dir = readdir(d))) {
      if (dir->d_type != DT_DIR) {
        if (length + strlen(dir->d_name) + 1 > BUFFER_SIZE) {
          printf("Packet exceeds max permitted size\n");
          return -1;
        }
        for (int i = 0; i < strlen(dir->d_name) + 1; i++) {
          buf[length + 5 + i] = dir->d_name[i];
        }
        length = length + strlen(dir->d_name) + 1;
        ncount++;
      }
    }
    closedir(d);
  }
  
  return 0; 
}

int search(int s, char *buf) {

  char input[20];
  if (fgets(input, sizeof(input), stdin))
    return -1;
  char *p = strchr(input, '\n');
  int index = (int)(p - input);
  buf[0] = 2;
  memcpy(buf + 1, input, index); 
  if (send(s, buf, sizeof(buf), 0) == -1) {
    perror("p2p peer: send");
    close(s);
    return 1;
  }

  uint32_t id;
  uint32_t peerip;
  uint32_t peerport;
  char *peername;

  if (recv(s, buf, sizeof(buf), 0) == -1) {
    perror("p2p peer: recv");
    close(s);
    return 1;
  }
  
  memcpy(id, buf, 4); 
  memcpy(peerip, buf + 4, 4); 
  memcpy(peerport, buf + 8, 2);

  id = ntohl(id);
  peerip = ntohl(peerip);
  peerport = ntohs(peerport);

  inet_ntop(AF_INET, &peerip, peername, 4);

  return 0;
}

