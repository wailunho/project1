// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <string.h>
#include <ctype.h>

//added
#include "alloc.h"
int count = 0;

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
	NEWLINE_T, // \n	
  EOF_T //end of file
  };

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */
struct command_stream
{
  int stream_loc;
	char** stream;
  int stream_size;
  char* last_string;
  char* current_string;
  int token_size;
  int linenum;
  enum token_type last_token;
  enum token_type current_token;
  int (*get_next_byte) (void *);
  void *get_next_byte_argument;
};

void increase_stream_size(command_stream_t s)
{
    s->stream_size += 20;
    s->stream = checked_realloc(s->stream, sizeof(char*) * s->stream_size);
}

void increase_token_size(command_stream_t s)
{
    s->token_size += 20;
    s->current_string = checked_realloc(s->current_string, sizeof(char*) * s->token_size);
}

command_stream_t get_token(command_stream_t buff)
{
  int token_finished = 0;
  int numofchar = 0;
  int ch = buff->get_next_byte(buff->get_next_byte_argument);
  while(token_finished == 0)
  {
    //remove whitespaces
    if(ch == ' ' || ch == '\t')
    {
      ch = buff->get_next_byte(buff->get_next_byte_argument);
    }

    //read a word
    else if(isalnum(ch) || ch == '!' ||
       ch == '%' || ch == '+' || ch == ',' || ch == '-' || 
       ch == '.' || ch == '/' || ch == ':' || ch == '@' || ch == '^')
    {
      buff->current_string[numofchar++] = ch;
      while(ch = buff->get_next_byte(buff->get_next_byte_argument))
      {
          if(isalnum(ch)  || ch == '!' ||
             ch == '%' || ch == '+' || ch == ',' || ch == '-' || 
             ch == '.' || ch == '/' || ch == ':' || ch == '@' || ch == '^')
          {
            buff->current_string[numofchar++] = ch;
          }
          else
          {
            ungetc(ch, buff->get_next_byte_argument);
            break;
          }     
          if(buff->token_size <= numofchar)
          {
            increase_token_size(buff);
          }
      }
      buff->current_string[numofchar] = '\0';
      buff->current_token = WORD_T;
      buff->stream[buff->stream_loc] = checked_malloc(sizeof (char*) * numofchar + 1);

      strcpy(buff->stream[buff->stream_loc++], buff->current_string);
      buff->stream[buff->stream_loc] = NULL;
      token_finished = 1;
    }
    //a command is made
    else if (ch == '\n' || ch == ';')
    {
      if(ch == '\n')
        buff->linenum++;
      ch = buff->get_next_byte(buff->get_next_byte_argument);
      while(ch == '\n' || ch == ';')
      {
        if(ch == '\n')
          buff->linenum++;
        ch = buff->get_next_byte(buff->get_next_byte_argument);
      }
      ungetc(ch, buff->get_next_byte_argument);
      buff->current_token = NEWLINE_T;
      token_finished = 1;
    }

    //it reaches the end of file
    else if(ch == EOF)
    {
      buff->current_token = EOF_T;
      token_finished = 1;
    }
    //reading in && and check the syntax
    else if(ch == '&')
    {
      buff->current_string[numofchar++] = '&';
      ch = buff->get_next_byte(buff->get_next_byte_argument);
      if (ch == '&')
      {
        buff->current_token = AND_T;
        buff->current_string[numofchar++] = '&';
        buff->current_string[numofchar] = '\0';
        buff->stream[buff->stream_loc] = checked_malloc(sizeof (char*) * numofchar + 1);
        strcpy(buff->stream[buff->stream_loc++], buff->current_string);
        buff->stream[buff->stream_loc] = NULL;
        token_finished = 1;
      }
      else
      {
        error (1, 0, "Line %d: Syntax error: &&", buff->linenum);
      }
    }
    //reading in either | or ||
    else if (ch == '|')
    {
      buff->current_string[numofchar++] = '|';
      ch = buff->get_next_byte(buff->get_next_byte_argument);
      if (ch == '|')
      {
        buff->current_token = OR_T;
        buff->current_string[numofchar++] = '|';
        buff->current_string[numofchar] = '\0';
        buff->stream[buff->stream_loc] = checked_malloc(sizeof (char*) * numofchar + 1);
        strcpy(buff->stream[buff->stream_loc++], buff->current_string);
        buff->stream[buff->stream_loc] = NULL;
        token_finished = 1;
      }
      else
      {
        ungetc(ch, buff->get_next_byte_argument);
        buff->current_string[numofchar] = '\0';
        buff->current_token = PIPE_T;
        buff->stream[buff->stream_loc] = checked_malloc(sizeof (char*) * numofchar + 1);
        strcpy(buff->stream[buff->stream_loc++], buff->current_string);
        buff->stream[buff->stream_loc] = NULL;
        token_finished = 1;
      }
    }
    //reading in <
    else if(ch == '<')
    {
      buff->current_token = INPUT_T;
      buff->current_string[numofchar++] = '<';
      buff->current_string[numofchar] = '\0';
      buff->stream[buff->stream_loc] = checked_malloc(sizeof (char*) * numofchar + 1);
      strcpy(buff->stream[buff->stream_loc++], buff->current_string);
      buff->stream[buff->stream_loc] = NULL;
      token_finished = 1;
    }
    //reading in >
    else if(ch == '>')
    {
      buff->current_token = OUTPUT_T;
      buff->current_string[numofchar++] = '>';
      buff->current_string[numofchar] = '\0';
      buff->stream[buff->stream_loc] = checked_malloc(sizeof (char*) * numofchar + 1);
      strcpy(buff->stream[buff->stream_loc++], buff->current_string);
      buff->stream[buff->stream_loc] = NULL;
      token_finished = 1;
    }   
    else if(ch == '(')
    {
      buff->current_token = OPEN_PAREN_T;
      buff->current_string[numofchar++] = '(';
      buff->current_string[numofchar] = '\0';
      buff->stream[buff->stream_loc] = checked_malloc(sizeof (char*) * numofchar + 1);
      strcpy(buff->stream[buff->stream_loc++], buff->current_string);
      buff->stream[buff->stream_loc] = NULL;
      token_finished = 1;
    }   
    else if(ch == ')')
    {
      buff->current_token = CLOSE_PAREN_T;
      buff->current_string[numofchar++] = ')';
      buff->current_string[numofchar] = '\0';
      buff->stream[buff->stream_loc] = checked_malloc(sizeof (char*) * numofchar + 1);
      strcpy(buff->stream[buff->stream_loc++], buff->current_string);
      buff->stream[buff->stream_loc] = NULL;
      token_finished = 1;
    }   
    else if(ch == '#')
    {
      while ((ch = buff->get_next_byte(buff->get_next_byte_argument)) != '\n');
      ungetc(ch, buff->get_next_byte_argument);
    } 
    else
      ch = buff->get_next_byte(buff->get_next_byte_argument);  

  }
  return buff;
}

command_stream_t get_token_array(command_stream_t buff)
{
  free(buff->stream);
  buff->stream = checked_malloc(sizeof (char*) * buff->stream_size);
  buff->stream_loc = 0;
  buff = get_token(buff);
  while(buff->current_token != NEWLINE_T && buff->current_token != SEMICOLON_T 
    && buff->current_token != EOF_T )
  {
    buff = get_token(buff);
    if(buff->stream_size <= buff->stream_loc)
      increase_stream_size(buff);
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
  buff->token_size = 20;
  buff->current_string = checked_malloc(sizeof(char*) * buff->token_size);
  buff->stream_size = 20;
  buff->stream = checked_malloc(sizeof (char*) * buff->stream_size);
  buff->stream_loc = 0;
  buff->linenum = 1;
  buff->last_token = WORD_T;
  buff->get_next_byte = get_next_byte;
  buff->get_next_byte_argument = get_next_byte_argument;
  
  return buff;
}




command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */  
  
  s = get_token_array(s);
  if(s->current_token != EOF_T)
  {
    command_t command_out = checked_malloc( sizeof(struct command) );
    command_out->type = SIMPLE_COMMAND;
    command_out->u.word = s->stream;

    //command_out->type = SIMPLE_COMMAND;
    //command_out->u.word = s->stream;
  
     
    if(s->current_token == WORD_T ||
	s-> current_token == SEMICOLON_T
	|| s-> current_token == NEWLINE_T)
    {
    	command_out->type = SIMPLE_COMMAND;
    	command_out->u.word = s->stream;
    }
    else if(s->current_token == PIPE_T)
    {
        command_out->type = OR_COMMAND;
	command_out->u.word = s->stream;

    }
    else if(s->current_token == AND_T)
    {
	command_out->type = AND_COMMAND;
	command_out->u.word = s->stream;
    }
    

	return command_out;
  }
  else
    return NULL;
}

