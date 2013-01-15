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
	char* stream;
	char* token;
	int size;
	enum token_type last_token;
	enum token_type current_token;
};

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  command_stream_t buff = checked_malloc(sizeof(struct command_stream));
  buff->stream = checked_malloc(sizeof (get_next_byte_argument) +1);
  buff->token = checked_malloc(30 * sizeof (char));
  int ch = get_next_byte(get_next_byte_argument);
  int stream_size = 0;
  while(ch != EOF) 
  {
        buff->stream[stream_size]=ch;
        stream_size++;
        ch = get_next_byte(get_next_byte_argument); 
  }
  buff->stream[stream_size]=ch;
  stream_size++;

  return buff;
}




command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
        char *result = &(s->stream[0]);
  	while (*(result) != EOF)
 	{
 		printf("%c", *(result));
  		result++;
        }
	
	return 0;
}


	

