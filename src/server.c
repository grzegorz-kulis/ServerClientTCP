#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <getopt.h>

#include <netdb.h>
#include <netinet/in.h>

#include "../include/thread.h"

#define PORT_NUMBER 12345

int thread_number = 0;
extern struct Thread *head_thread; //list of threads
pthread_mutex_t list_thread = PTHREAD_MUTEX_INITIALIZER; //access list of threads
const char *program_name;

volatile sig_atomic_t do_shutdown = 0;

void print_usage(FILE* stream, int exit_code)
{
  fprintf(stream, "Usage: %s options [ inputfile ]\n", program_name);
  fprintf(stream,
      "  -h  --help       Display this usage information\n"
      "  Run `make` command to build both the server and client applications\n"
      "  Run `makefile -f makefile.win` to build only server application\n"
      "  Type `./server` into terminal to start the server and wait for the client to connect\n\n");
  exit(exit_code);
}

static void sigterm_handler(int sig)
{
  printf("Caught signal %d. Shutting down.\n", sig);
  do_shutdown = 1;
}

int main(int argc, char* argv[])
{
  const char* const short_options = "h";
  const struct option long_options[] = {
    { "help", 0, NULL, 'h'}
  };
  struct sockaddr_in serv_addr;
  struct sockaddr_in cli_addr;
  int socket_fd;
  int client_socket_fd;
  int client_length;
  int *new_sock;
  int next_option;
  char buffer[256];

  program_name = argv[0];

  do {
    next_option = getopt_long(argc, argv, short_options, long_options, NULL);

    switch(next_option)
    {
      case 'h':
	print_usage(stdout,0);

      case '?':
	print_usage(stderr,1);

      case -1:
	break;

      default:
	abort();

    }

  } while(next_option != -1);

  if(getcwd(cwd, sizeof(cwd)) !=  NULL)
    fprintf(stdout, "Current working dir: %s\n", cwd);
  else
    perror("getcwd() error");

  socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  if(socket_fd < 0) {
    perror("Error opening socket!");
    exit(1);
  }

  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
    error("setsockopt(SO_REUSEADDR) failed");

  /* Initialize socket structure */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr= INADDR_ANY;
  serv_addr.sin_port = htons(PORT_NUMBER);

  /* Binding */
  if(bind(socket_fd, (struct sockaddr *) &serv_addr,  sizeof(serv_addr)) < 0) {
    perror("Error on binding!");
    exit(1);
  }

  const int thread_pool_step = 10;

  /* Listen for incoming connections */
  listen(socket_fd, thread_pool_step);
  client_length = sizeof(cli_addr);

  /* Thread array */
  int thread_pool_size = thread_pool_step;
  pthread_t *thread_ids = malloc(thread_pool_size *sizeof(pthread_t));
  if(thread_ids == NULL) {
    perror("Failed to allocate memory for threads ids");
    abort();
  }
  memset(thread_ids, 0, thread_pool_size*sizeof(pthread_t));
  int thread_index = -1;

  struct sigaction a;
  a.sa_handler = sigterm_handler;
  a.sa_flags = 0;
  sigemptyset(&a.sa_mask);
  sigaction(SIGINT, &a, NULL);
  sigaction(SIGQUIT, &a, NULL);
  sigaction(SIGTERM, &a, NULL);

  while(!do_shutdown) {

    client_socket_fd = 
      accept(socket_fd, (struct sockaddr*) (&cli_addr), &client_length);

    if(client_socket_fd < 0) {
      fprintf(stderr, "accept() failed: %s\n", strerror(errno));
      break;
    }

    /* Arguments for thread */
    new_sock = malloc(sizeof(client_socket_fd));
    *new_sock = client_socket_fd;

    if(++thread_index >= thread_pool_size) {
      thread_pool_size += thread_pool_step;
      thread_ids = realloc(thread_ids, thread_pool_size*sizeof(pthread_t));
      if(thread_ids == NULL) {
	fprintf(stderr, "failed to realloc thread pool: %s\n", strerror(errno));
	abort();
      }
    }

    /* Create new thread for a newly connected client */
    if(pthread_create(&thread_ids[thread_index++], NULL, handle_connection,
	  (void*) new_sock)) 
    {
      fprintf(stderr, "pthread_create() failed: %s\n", strerror(errno));
      abort;
    }
  }

  puts("Waiting for threads to finish");
  while(--thread_index >= 0) {
    if(thread_ids[thread_index])
      pthread_join(thread_ids[thread_index], NULL);
  }

  if(thread_ids != NULL)
    free(thread_ids);


  return 0;
}


