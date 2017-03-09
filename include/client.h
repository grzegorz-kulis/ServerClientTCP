#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 9999

typedef struct MsgParts {
  char* part1;
  char* part2;
  char* part3;
  char* part4;
  char* part5;
} MsgParts;

const char token_IgnoreSigns[] = " \n\0"; //used to extract proper message on client side
const int port_number = 12345; 


static void send_message(const int , const char*);
void prepare_tokens(char* , MsgParts*);
void send_file(const int* , const MsgParts*);
void download_file(const int*, const MsgParts*, char*);


