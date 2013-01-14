// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <string.h>

//added
#include "alloc.h"

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */
enum token_type
  {
	WORD_T,	//ASCII letters, digits, or any of: ! % + , - . / : @ ^ _
	SEMICOLON_T, // ;
	PIPE_T, // |
	AND_T, 	//&&
	OR_T, 	// ||
	OPEN_PAREN_T, // (
	CLOSE_PAREN_T, // )
	INPUT_T, // <
	OUTPUT_T, // >
	NEWLINE_T // \n	
  };

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */
struct command_stream
{
	int (*next_byte)(void *);
	void * next_byte_argument;
	
};

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
  int breakcount=0;
  char *buff = checked_malloc(sizeof(get_next_byte_argument)+1);
  while(ch != EOF) {
    if( ch =='\n' || ch == ';'){
    breakcount++;
    }
    buff[stream_size]=ch;
    stream_size++;
    ch = get_next_byte(get_next_byte_argument); 
  }
  //printf("Size of the bytestream is %d\n", sizeof(get_next_byte_argument)); 
  //printf("the buffer contains:\n%s", buff);
  //printf("the number of line breaks are: %d\n", breakcount); 
  
  
  char delims[] = "\n;";
  char *result = NULL;
  size_t result_size = sizeof(result);
  char *lines[breakcount+1];
  int numlines=0;
  result = strtok(buff, delims);
  while( result != NULL) {
    lines[numlines] = result;
    numlines++;
    result = strtok(NULL, delims);
  }
  
  //int i=0;
  //for( ; i<numlines; i++){
  //printf("%s\n", lines[i]);
  //}
  
  //lines is an array of Cpointers to strings 
  

  //doesnt work when i free memory
  //free(buff);

  //error (1, 0, "command reading not yet implemented");
  return 0;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
}
