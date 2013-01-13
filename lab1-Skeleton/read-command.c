// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <string.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */


command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  //printf("%s", (char *)(get_next_byte_argument)); 
  int ch = get_next_byte(get_next_byte_argument);
  int stream_size=0; 
  char *buff = malloc(sizeof(get_next_byte_argument)+1);
  while(ch != EOF) {
    buff[stream_size]=ch;
    stream_size++;
    ch = get_next_byte(get_next_byte_argument); 
  }
  //printf("Size of the bytestream is %d\n", sizeof(get_next_byte_argument)); 
  //printf("the buffer contains:\n%s", buff);
  
  char delims[] = "\n;";
  char *result = NULL;
  //int inst_count = 0;
  result = strtok(buff, delims);
  while( result != NULL) {
    printf("%s\n", result);
    //inst_count++;
    result = strtok(NULL, delims);
  }
  //doesnt work when i free memory
  //free(buff);

  error (1, 0, "command reading not yet implemented");
  return 0;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
}
