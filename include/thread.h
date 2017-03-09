#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>

#include <netdb.h>
#include <netinet/in.h>

#define MAX_THREADS 512
#define MAX_SIZE 9999

char cwd[1024];
struct Thread* head_thread;
volatile sig_atomic_t do_shutdown;

typedef struct MsgParts {
  char* part1;
  char* part2;
  char* part3;
  char* part4;
  char* part5;
} MsgParts;

struct Thread {
  int connection_id;
  struct Thread *next_thread;
};

void prepare_tokens(char*, MsgParts*);
void handle_ls();
void handle_cd();
void handle_cp();
void handle_mv();
void handle_scp();
void handle_wget();
void* handle_connection(void*);
short insert_thread();
void remove_thread(short);
static void sigterm_handler(int);
