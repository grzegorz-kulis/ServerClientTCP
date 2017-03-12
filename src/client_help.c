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
  
  do
  {
    next_option = getopt_long(argc, argv, short_options, long_options, NULL);

    switch(next_option)
    {
      case 'h':
        print_usage(stdout,0, program_name);
	 
      case '?':
	print_usage(stderr,1, program_name);

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

