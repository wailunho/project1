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
  WORD_T, //ASCII letters, digits, or any of: ! % + , - . / : @ ^ _
  SEMICOLON_T, // ;
  PIPE_T, // |
  AND_T,  //&&  
  OR_T,   // ||
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
  int numofchar;
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
    s->stream_size += 30;
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
      token_finished = 1;
    }
    else if (ch == ';')
    {
      buff->current_token = SEMICOLON_T;
      buff->current_string[numofchar++] = ';';
      buff->current_string[numofchar] = '\0';
      token_finished = 1;
    }
    //a command is terminated by either newline or ;
    else if (ch == '\n')
    {
      buff->linenum++;
      ch = buff->get_next_byte(buff->get_next_byte_argument);
      //ignore any non-command token
      while(ch == '\n' || ch == ';' || ch == '#' || ch == ' ' || ch == '\t' || ch == EOF)
      {
        //code below ignore any extra newlines, ;, comments and whitespace.
        //depends on the char it read, it does different setup.
        //For extra newlines - increment the line number;
        //For comments - ignore any character after # till \n
        //and no actions for the others
        if(ch == '\n')
          buff->linenum++;
        else if (ch == '#')
        {
          while((ch = buff->get_next_byte(buff->get_next_byte_argument)) != '\n');
          buff->linenum++;
        }  
        else if(ch == EOF)
        {
          buff->current_token = EOF_T;
          token_finished = 1;
          break;
        }
        ch = buff->get_next_byte(buff->get_next_byte_argument);
      }
      ungetc(ch, buff->get_next_byte_argument);
      if(ch != EOF)
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
        token_finished = 1;
      }
      else
        error (1, 0, "Line %d: Syntax error: &&", buff->linenum);
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
        token_finished = 1;
      }
      else
      {
        ungetc(ch, buff->get_next_byte_argument);
        buff->current_string[numofchar] = '\0';
        buff->current_token = PIPE_T;
        token_finished = 1;
      }
    }
    //reading in <
    else if(ch == '<')
    {
      buff->current_token = INPUT_T;
      buff->current_string[numofchar++] = '<';
      buff->current_string[numofchar] = '\0';
      token_finished = 1;
    }
    //reading in >
    else if(ch == '>')
    {
      buff->current_token = OUTPUT_T;
      buff->current_string[numofchar++] = '>';
      buff->current_string[numofchar] = '\0';
      token_finished = 1;
    }   
    else if(ch == '(')
    {
      buff->current_token = OPEN_PAREN_T;
      buff->current_string[numofchar++] = '(';
      buff->current_string[numofchar] = '\0';
      token_finished = 1;
    }   
    else if(ch == ')')
    {
      buff->current_token = CLOSE_PAREN_T;
      buff->current_string[numofchar++] = ')';
      buff->current_string[numofchar] = '\0';
      token_finished = 1;
    }   
    else if(ch == '#')
    {
      while ((ch = buff->get_next_byte(buff->get_next_byte_argument)) != '\n');
      ungetc(ch, buff->get_next_byte_argument);
    } 
    else
      error (1, 0, "Line %d: Invalid character %c entered", buff->linenum, ch);
   
  }
  buff->numofchar = numofchar;
  return buff;
}

command_stream_t get_token_array(command_stream_t buff)
{
  int numofchar = 0;
  free(buff->stream);
  buff->stream = checked_malloc(sizeof (char*));
  buff->stream[0] = checked_malloc(sizeof (char*) * buff->stream_size);
  buff = get_token(buff);
  strcpy(buff->stream[0], "\0");

  while(buff->current_token != NEWLINE_T && buff->current_token != SEMICOLON_T 
    && buff->current_token != EOF_T )
  {
    numofchar += buff->numofchar;
    if(buff->stream_size <= numofchar)
      increase_stream_size(buff);

    strcat(buff->stream[0], buff->current_string);
    buff = get_token(buff);
  }
  buff->stream[1] = NULL;
  return buff;
}

command_t get_simple_command(command_stream_t buff)
{
  int numofchar = 0;
  int word_size = 40;
  int current_line;
  command_t s = checked_malloc(sizeof(struct command));
  s->u.word = checked_malloc(sizeof(char*));
  s->u.word[0] = checked_malloc(sizeof(char*) * word_size);
  s->isfinal  = 0;
  strcpy(s->u.word[0], buff->current_string);
  buff->last_token  = buff->current_token;
  current_line = buff->linenum;

  while(buff = get_token(buff))
  {
    //reading any word
    if(buff->last_token == WORD_T)
    {
      if(buff->current_token == WORD_T)
      {
        numofchar += buff->numofchar;
        if(word_size <= numofchar)
        {
          word_size += 40;
          s->u.word[0] = checked_realloc(s->u.word[0], sizeof(char*) * word_size);
        }
        strcat(s->u.word[0], " ");
        strcat(s->u.word[0], buff->current_string);
      }
      else if (buff->current_token == INPUT_T || buff->current_token == OUTPUT_T)
      {
        numofchar += buff->numofchar;
        if(word_size <= numofchar)
        {
          word_size += 40;
          s->u.word[0] = checked_realloc(s->u.word[0], sizeof(char*) * word_size);
        }
        strcat(s->u.word[0], buff->current_string);
      }
    }
    //last token is a <
    else if(buff->last_token == INPUT_T)
    {
      if(buff->current_token != WORD_T)
        error (1, 0, "Line %d: Syntax error: a word come after <", current_line);
      else
      {
        numofchar += buff->numofchar;
        if(word_size <= numofchar)
        {
          word_size += 40;
          s->u.word[0] = checked_realloc(s->u.word[0], sizeof(char*) * word_size);
        }
        strcat(s->u.word[0], buff->current_string);
      }
    }
    //last token is a >
    else if(buff->last_token == OUTPUT_T)
    {
      if(buff->current_token != WORD_T)
        error (1, 0, "Line %d: Syntax error: a word come after >", current_line);
      else
      {
        numofchar += buff->numofchar;
        if(word_size <= numofchar)
        {
          word_size += 40;
          s->u.word[0] = checked_realloc(s->u.word[0], sizeof(char*) * word_size);
        }
        strcat(s->u.word[0], buff->current_string);
      }
    }
    else
    {
      s->u.word[1] = NULL;
      s->type = SIMPLE_COMMAND;
      s->isfinal = 1;
      return s;
    }

    buff->last_token  = buff->current_token;
    current_line = buff->linenum;
  }
}

command_t get_command(command_stream_t buff)
{
  buff = get_token(buff);
  if(buff->current_token == WORD_T)
  {
    command_t s = get_simple_command(buff);
    return s;
  }
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
  buff->stream_size = 30;
  buff->stream = checked_malloc(sizeof (char*));
  buff->linenum = 1;
  buff->current_token = WORD_T;
  buff->get_next_byte = get_next_byte;
  buff->get_next_byte_argument = get_next_byte_argument;
  
  return buff;
}

command_t
read_command_stream (command_stream_t s)
{
 
  /*
  TODO: 
     WORD_T
           word=s->stream
     PIPE_T, 
           fill in command[0] (left side of pipe)
    
  */

 /*
  s = get_token(s);
  if(s->current_token != EOF_T)
  {
    command_t command_out = checked_malloc( sizeof(struct command) );
   
    if(s->current_token == WORD_T)
    {
        printf("found a word!: %s\n", s->current_string);
        command_out->type = SIMPLE_COMMAND;
        command_out->u.word = s->stream;
    }
    else if(s->current_token == PIPE_T)
    {
  printf("found a pipe!: %s\n", s->current_string);
        command_out->type = SIMPLE_COMMAND;
        command_out->u.word = s->stream;
    }
    else if (s->current_token == AND_T)
    {
  printf("found an AND: %s\n", s->current_string);
        command_out->type = SIMPLE_COMMAND;
        command_out->u.word = s->stream;
    }
    else if(s->current_token == OR_T)
    {
  printf("found an OR: %s\n", s->current_string);
        command_out->type = SIMPLE_COMMAND;
        command_out->u.word = s->stream;
    }
    else if (s->current_token == INPUT_T)
    {   //still default, where does input go?
  printf("Found an INPUT: %s\n", s->current_string);
        command_out->type = SIMPLE_COMMAND;
        command_out->u.word = s->stream;
    }
    else if(s->current_token == OUTPUT_T)
    {   //still default, where does output go?
        printf("Found an OUTPUT: %s\n", s->current_string);
        command_out->type = SIMPLE_COMMAND;
        command_out->u.word = s->stream;
    }
    else if(s->current_token == OPEN_PAREN_T)
    {   //default, should fill in SUBSHELL_COMMAND
        printf("Found an OPEN paren: %s\n", s->current_string);
        command_out->type = SIMPLE_COMMAND;
        command_out->u.word = s->stream;
    }
    else if(s->current_token == CLOSE_PAREN_T)
    {    //default, should fill in subshell_command
         printf("Found a CLOSE PAREN: %s\n", s->current_string);
         command_out->type = SIMPLE_COMMAND;
         command_out->u.word = s->stream;
    }
    else if(s->current_token ==NEWLINE_T || s->current_token == SEMICOLON_T)
    {   //default
  //skip new lines
  command_out->type = SIMPLE_COMMAND;
        command_out->u.word = s->stream;
    }
error
    //printf("Testing: token type is : %s\n", s->current_string);
    //command_out->type = SIMPLE_COMMAND;
    //command_out->u.word = s->stream;
 
   return command_out;
  }
  else
  {
  return NULL;
  }
  */
  command_t result = get_command(s);
  if (count == 1)
  {
    return NULL;
  }
  else
  {
    count++;
    return result;
  }
}