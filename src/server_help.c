#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

void handle_options(int , char* [] , const char*);
void print_usage(FILE* , int , const char* );

/**
* Handles options given at the start-up of the program
*/
void handle_options(int argc, char* argv[], const char* pn)
{
  const char* const short_options = "h";
  const struct option long_options[] = {
    { "help", 0, NULL, 'h'}
  };
  int next_option;
  const char* program_name = pn;
 
  do {
    next_option = getopt_long(argc, argv, short_options, long_options, NULL);

    switch(next_option)
    {
      case 'h':
	print_usage(stdout,0,program_name);

      case '?':
	print_usage(stderr,1,program_name);

      case -1:
	break;

      default:
	abort();

    }
  } while(next_option != -1);
}

/**
* Prints necessary information related to the help section to the screen
*/
void print_usage(FILE* stream, int exit_code, const char* pn)
{
  fprintf(stream, "Usage: %s options [ inputfile ]\n", pn);
  fprintf(stream,
      "  -h  --help       Display this usage information\n"
      "  Run `make` command to build both the server and client applications\n"
      "  Run `makefile -f makefile.win` to build only server application\n"
      "  Type `./server` into terminal to start the server and wait for the client to connect\n\n");
  exit(exit_code);
}
