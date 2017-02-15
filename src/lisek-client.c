#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>

#define MAX_SIZE 10000

const char* program_name;

void print_usage(FILE* stream, int exit_code)
{
  fprintf(stream, "Usage: %s options [ inputfile ]\n", program_name);
  fprintf(stream,
          "  -h  --help       Display this usage information\n"
	  "  Run `make` command to build both the server and client applications\n"
	  "  Run `makefile -f makefile.nix` to build only client application\n"
	  "  Run the program with `./client localhost` after running server application\n"
	  "  There is limited pool of avaiable commands that can be casted on the  server:\n"
	  "    `ls` - lists all files in the directory;\n"
	  "           it will list files from directory in which server app was started\n"
	  "    `cd ..` - goes one directory up\n"
	  "    `cd dir` - enters directory `dir`; user has to make sure it exists\n"
	  "    `cp file /dir/file` - copies a file;\n"
	  "        file - name of the file in the currect directory (must exist)\n"
	  "        /dir/file - destination path for the file including its new name\n"
	  "    `mv file /dir/file` - moves a file;\n"
	  "        file - name of the file in the current directory (must exist)\n"
	  "        /dir/file - destination path for the file including its new name\n"
	  "    `scp /dir1/src_file /dir2/dst_file` - sends a file to the server\n"
	  "         /dir1/src_file - full path to file which should be sent\n"
	  "         /dir2/dst_path - full path where uploaded file should be saved\n");
  exit(exit_code);
}

static void send_message(int socket_fd, const char *msg) {

  int rc;
  int message_size = strlen(msg);

  do {
    rc = send(socket_fd, msg, message_size, 0);
    if(rc < 0) {
      fprintf(stderr, "send() failed: %s\n", strerror(errno));
      abort();
    }
  } while(rc < message_size);

}

int main(int argc, char* argv[])
{
  const char* const short_options = "h";
  const struct option long_options[] = {
    { "help", 0, NULL, 'h'}
  };
  int socket_fd, port_number;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[MAX_SIZE];
  int rc, next_option;

  const char space[] = " \n\0";

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

  if(argc<2) {
    fprintf(stderr,"Incorrect arguments input\n");
    exit(0);
  }

  port_number = 12345;

  server = gethostbyname(argv[1]);

  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(socket_fd < 0) {
    perror("Error opening socket");
    exit(1);
  }

  bzero((char*) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port_number);
  bcopy((char*) server->h_addr,(char *) &serv_addr.sin_addr.s_addr,sizeof(server->h_length));

  if(connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("Error connecting");
      exit(1);
  }

  enum {
    INIT_STATE,
    READY_STATE
  } serv_state = INIT_STATE;

  #define check_serv_state(str, st) \
   (strncmp(str, #st, sizeof(#st) - 1) == 0)

  for(;;) {

    memset(buffer, 0, sizeof(buffer));
    rc = recv(socket_fd, buffer, sizeof(buffer), 0);
    if(rc<0) {
      perror("Error reading back from server");
      exit(1);
    }
    else if(rc == 0) {
      printf("The server has been disconnected. Quitting.\n");
      exit(0);
    }

    if(serv_state == INIT_STATE) {
      char *part = strtok(buffer,"\n");
      do {
	if(check_serv_state(part, READY)) {
	  printf("Server is ready\n");
	  serv_state = READY_STATE;
	}
	else 
	  printf("Server: [[[%s]]]\n", part);
      } while((part = strtok(NULL, "\n")));

      if(serv_state != READY_STATE)
	 continue;
      } else
	 printf("#server: %s", buffer);

      memset(buffer, 0, sizeof(buffer));

      /* Get command from client */
      printf("\n#client: ");
      fgets(buffer, sizeof(buffer)/sizeof(buffer[0]) - 1, stdin);

      /* Tokens */
      char *temp = (char*) malloc(strlen(buffer)+1);
      strcpy(temp, buffer);
      char *fix = strtok(temp,space);
      char *part1;
      char *part2;
      char *part3;
      char *part4;
      int flag = 0;
      while(fix != NULL) {
        if(flag == 0) {
          part1 = fix;
          fix = strtok(NULL,space);
        }
	if(flag == 1) {
          part2 = fix;
          fix = strtok(NULL,space);
        }
	if(flag == 2) {
          part3 = fix;
          fix = strtok(NULL,space);
        }
	if(flag == 3) {
          part4 = fix;
          fix = strtok(NULL,space);
        }
	flag++;
      }
      flag = 0;
 
      //Send a file
      if(strcmp(part1, "scp") == 0) {

        FILE *fp;
        size_t nbytes;
        char file_buf[MAX_SIZE];
	char scp_buffer[MAX_SIZE];

        fp = fopen(part2, "r");

	if(fp != NULL) {
          nbytes = fread(file_buf, 1, MAX_SIZE, fp);
        }
	else
	  perror("Error reading file");

        //Build a proper info to the server 
	if(nbytes > 0) {
          strcpy(scp_buffer, part1);
          strcat(scp_buffer, " ");
          strcat(scp_buffer, part3);
          strcat(scp_buffer, " ");
          strcat(scp_buffer, file_buf);
          send_message(socket_fd, scp_buffer);
        }
	fclose(fp);	
	
      }
      //Download a file
      else if(strcmp(part1, "wget") == 0) {

        send_message(socket_fd, buffer);
	
	FILE *fd = fopen(part3, "w");

        memset(buffer, 0, sizeof(buffer));
        rc = recv(socket_fd, buffer, sizeof(buffer), 0);
        if(rc<0) {
          perror("Error reading back from server");
          exit(1);
        }
        else if(rc == 0) {
          printf("The server has been disconnected. Quitting.\n");
          exit(0);
        }

	fwrite(buffer, sizeof(char), strlen(buffer), fd);
	fclose(fd);
	printf("Done downloading\n");
	
	
      }
      else {
        int len = strlen(buffer);
        if(buffer[len-1] == '\n') {
	  buffer[len-1] = '\0';
        }
	send_message(socket_fd, buffer);
	  
      }
  }
   
  close(socket_fd);
  return 0;
}
