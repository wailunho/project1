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
  int stream_size;
  char* token;
  int token_size;
  int linenum;
  enum token_type last_token;
  enum token_type current_token;
  int (*get_next_byte) (void *);
  void *get_next_byte_argument;
};

void increase_stream_size(command_stream_t s)
{
    s->stream_size += 40;
    s->stream = checked_realloc(s->stream, sizeof(char*) * s->stream_size);
}

void increase_token_size(command_stream_t s)
{
    s->token_size += 20;
    s->token = checked_realloc(s->token, sizeof(char*) * s->token_size);
}

//command_stream_t get_token()

command_stream_t get_token(command_stream_t buff)
{
  int numofchar = 0;
  int ch = buff->get_next_byte(buff->get_next_byte_argument);
  while(ch != EOF)
  {
    //remove whitespaces
    if(ch == ' ' || ch == '\t')
      continue;

    //read a word
    else if(('A' <= ch && ch <= 'Z') || ('a' <= ch && ch <= 'z') ||
       ('0' <= ch && ch <= '9') || ch == '!' ||
       ch == '%' || ch == '+' || ch == ',' || ch == '-' || 
       ch == '.' || ch == '/' || ch == ':' || ch == '@' || ch == '^')
    {
      buff->token[numofchar++] = ch;
      while(ch = buff->get_next_byte(buff->get_next_byte_argument))
      {
          if(('A' <= ch && ch <= 'Z') || ('a' <= ch && ch <= 'z') ||
            ('0' <= ch && ch <= '9') || ch == '!' ||
             ch == '%' || ch == '+' || ch == ',' || ch == '-' || 
             ch == '.' || ch == '/' || ch == ':' || ch == '@' || ch == '^')
          {
            buff->token[numofchar++] = ch;
          }
          else
            break;
          if(buff->token_size <= numofchar)
          {
            increase_token_size(buff);
            buff->token[numofchar] = '\0';
            buff->current_token = WORD_T;
          }
      }

    }
    //remove whitespaces from the beginning of a command
    else if(ch == ' ' || ch == '\t')
      continue;
    //a command is made
    else if (ch == '\n')
    {
      buff->linenum++;
      while(ch = buff->get_next_byte(buff->get_next_byte_argument) == '\n')
      {
         buff->linenum++;
      }
      continue;
    }
    else
      ch = buff->get_next_byte(buff->get_next_byte_argument);  

  }
  return buff;
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  command_stream_t buff = checked_malloc(sizeof(struct command_stream));
  buff->stream_size = 40;
  buff->stream = checked_malloc(sizeof (char*) * buff->stream_size);
  buff->token_size = 20;
  buff->token = checked_malloc(sizeof(char*) * buff->token_size);
  buff->linenum = 0;
  buff->last_token = WORD_T;
  buff->get_next_byte = get_next_byte;
  buff->get_next_byte_argument = get_next_byte_argument;
  
  buff = get_token(buff);
  return buff;
}




command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
      /*  char *result = &(s->stream[0]);
  	while (*(result) != EOF)
 	{
 		printf("%c", *(result));
  		result++;
        }
*/
  int i = 0;
  for(i; s->token[i] != '\0'; i++)
  {
    printf("%c", s->token[i]);
  }

  printf("\n%d\n", s->linenum);
  return 0;
}

