#include <stdio.h>
#include <stdlib.h>

#define PORT_NUMBER 12345

static int thread_number = 0;

void print_usage(FILE* , int);
static void sigterm_handler(int);





