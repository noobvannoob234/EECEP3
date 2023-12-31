// Andrew Roda, Myles Coleman
// 10/13/2023
// EECE 446
// Program 2

/* This code is an updated version of the sample code from "Computer Networks: A
 * Systems Approach," 5th Edition by Larry L. Peterson and Bruce S. Davis. Some
 * code comes from man pages, mostly getaddrinfo(3). */
#include <dirent.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
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

  if (argc != 4) {
    printf("usage: host port peerid\n");
    exit(1);
  }
  char *host = argv[1];
  char *port = argv[2];
  int id = atoi(argv[3]);
  if (atoi(port) <= 2000 || atoi(port) >= 65535) {
    printf("Invalid port number.\n");
    exit(1);
  }

  int s; // fd
  uint32_t peerid = (uint32_t)id;
  /* Lookup IP and connect to server */
  if ((s = lookup_and_connect(host, port)) < 0) {
    exit(1);
  }
  while (strcmp(input, "EXIT") != 0) {
    printf("Enter a command: ");
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
 return 0; 
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
  buf[0] = 1;
  ncount = htonl(ncount);
  memcpy(buf + 1, &ncount, 4);
  if (send(s, buf, length + 5, 0) == -1) {
    perror("p2p peer: send");
    close(s);
    return 1;
  }
  return 0;
}

int search(int s, char *buf) {
  char input[200];
  printf("Enter a file name: ");
  if (fgets(input, sizeof(input), stdin) == NULL) {
    printf("error, return -1\n");
    return -1;
  }
  char *p = strchr(input, '\n');
  if (p) *p = '\0'; 
  int index = (int)(p - input);
  buf[0] = 2;
  memcpy(buf + 1, input, index + 1);
  if (send(s, buf, index + 2, 0) == -1) {
    perror("p2p peer: send");
    close(s);
    return 1;
  }  uint32_t id;
  uint32_t peerip;
  uint32_t peerport;
  char peername[INET_ADDRSTRLEN];
  char peerinfo[10];
  int totalbytes = 0;
  int recbytes = 1;
  while (totalbytes < 10) {
    recbytes = recv(s, buf, sizeof(buf), 0);
    if (recbytes == -1) {
      perror("p2p peer: recv");
      close(s);
      return 1;
    }
    memcpy(peerinfo + totalbytes, buf, recbytes);
    totalbytes = totalbytes + recbytes;
  }
  for (int i = 0; i < 10; i++) {
    if (peerinfo[i] != 0) {
      break;
    } else {
      printf("File not indexed by registry\n");
      return 0;
    }
  }
  memcpy(&id, peerinfo, 4);
  memcpy(&peerip, peerinfo + 4, 4);
  memcpy(&peerport, peerinfo + 8, 2);
  id = htonl(id);
  peerport = ntohs(peerport);
  inet_ntop(AF_INET, &peerip, peername, INET_ADDRSTRLEN);
  printf("File found at\n");
  printf("peer %u\n", id);
  printf("%s:%d\n", peername, peerport);
  return 0;
}