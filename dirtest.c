#include <dirent.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define PUBLISH_DIRECTORY "SharedFiles"
#define BUFFER_SIZE 200

int main() {
  char buf[BUFFER_SIZE];
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
  for (int i = 0; i < length + 5; i++) {
    if (buf[i] == '\0') {
      printf("0");
    }
    printf("%c", buf[i]);
  }

  printf("%s\n", buf);
  return 0;
}