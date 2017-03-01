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

#define MAX_SIZE 9999

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
  const char* program_name = argv[0];
  const char space[] = " \n\0";
  const int port_number = 12345; 
  int socket_fd, rc;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[MAX_SIZE];
  
  handle_options(argc, argv, program_name);

  if(argc<2) {
    fprintf(stderr,"Incorrect arguments input\n");
    exit(0);
  }

  server = gethostbyname(argv[1]);

  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(socket_fd < 0) {
    perror("Error opening socket\n");
    exit(1);
  }

  bzero((char*) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port_number);
  bcopy((char*) server->h_addr,
	(char *) &serv_addr.sin_addr.s_addr,
	sizeof(server->h_length));

  if(connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("Error connecting\n");
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
      perror("Error reading back from server.\n");
      exit(1);
    }
    else if(rc == 0) {
      printf("The server has been disconnected. Quitting.\n");
      exit(0);
    }

    /**
    * Check if we get proper message from the server so we can start communicating. While booting, we assume that server is in the INIT_STATE.
    */
    if(serv_state == INIT_STATE) {
      char *msg = strtok(buffer,"\n");
      /* Gather information from the buffer */
      do {
	if(check_serv_state(msg, READY)) {
	  printf("Server is ready\n");
	  serv_state = READY_STATE;
	}
	else 
	  printf("Server: [[[%s]]]\n", msg);
      } while((msg = strtok(NULL, "\n")));

      if(serv_state != READY_STATE)
	 continue;
      } else
	 printf("#server: %s", buffer);

      memset(buffer, 0, sizeof(buffer));

      /* Get command from client terminal */
      printf("\n#client: ");
      fgets(buffer, sizeof(buffer)/sizeof(buffer[0]) - 1, stdin);

      /** 
      * Gather tokens from the message which the user typed in so we can handle partial work of the commands
       */
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
