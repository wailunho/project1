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
command_stream_t get_token(command_stream_t buff);
command_t get_complete_command(command_stream_t buff);
command_t get_simple_command(command_stream_t buff);
command_t get_andor_command(command_stream_t buff);
command_t get_pipe_command(command_stream_t buff);
command_t get_command(command_stream_t buff);
command_t get_subshell_command(command_stream_t buff);

enum token_type
  {
  START_T,
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
  char* next_string;
  char* current_string;
  int token_size;
  int linenum;
  enum token_type next_token;
  enum token_type current_token;
  int (*get_next_byte) (void *);
  void *get_next_byte_argument;
  int finalcommand;
  int firstcommand;
};

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

  strcpy(buff->current_string, buff->next_string);
  buff->current_token = buff->next_token;
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
      buff->next_string[numofchar++] = ch;
      while(ch = buff->get_next_byte(buff->get_next_byte_argument))
      {
          if(isalnum(ch)  || ch == '!' ||
             ch == '%' || ch == '+' || ch == ',' || ch == '-' || 
             ch == '.' || ch == '/' || ch == ':' || ch == '@' || ch == '^')
          {
            buff->next_string[numofchar++] = ch;
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
      buff->next_string[numofchar] = '\0';
      buff->next_token = WORD_T;
      token_finished = 1;
    }
    else if (ch == ';')
    {
      buff->next_token = SEMICOLON_T;
      buff->next_string[numofchar] = '\0';
      token_finished = 1;
    }
    //a command is terminated by either newline
    else if (ch == '\n')
    {
      buff->linenum++;
      ch = buff->get_next_byte(buff->get_next_byte_argument);
      //ignore any non-command token
      while(ch == '\n' || ch == '#' || ch == ' ' || ch == '\t' || ch == EOF)
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
          buff->next_token = EOF_T;
          token_finished = 1;
          break;
        }
        ch = buff->get_next_byte(buff->get_next_byte_argument);
      }
      ungetc(ch, buff->get_next_byte_argument);
      if(ch != EOF)
        buff->next_token = NEWLINE_T;
      token_finished = 1;
    }

    //it reaches the end of file
    else if(ch == EOF)
    {
      buff->next_token = EOF_T;
      token_finished = 1;
    }
    //reading in && and check the syntax
    else if(ch == '&')
    {
      buff->next_string[numofchar++] = '&';
      ch = buff->get_next_byte(buff->get_next_byte_argument);
      if (ch == '&')
      {
        buff->next_token = AND_T;
        buff->next_string[numofchar++] = '&';
        buff->next_string[numofchar] = '\0';
        token_finished = 1;
      }
      else
        error (1, 0, "Line %d: Syntax error: &&", buff->linenum);
    }
    //reading in either | or ||
    else if (ch == '|')
    {
      buff->next_string[numofchar++] = '|';
      ch = buff->get_next_byte(buff->get_next_byte_argument);
      if (ch == '|')
      {
        buff->next_token = OR_T;
        buff->next_string[numofchar++] = '|';
        buff->next_string[numofchar] = '\0';
        token_finished = 1;
      }
      else
      {
        ungetc(ch, buff->get_next_byte_argument);
        buff->next_string[numofchar] = '\0';
        buff->next_token = PIPE_T;
        token_finished = 1;
      }
    }
    //reading in <
    else if(ch == '<')
    {
      buff->next_token = INPUT_T;
      buff->next_string[numofchar++] = '<';
      buff->next_string[numofchar] = '\0';
      token_finished = 1;
    }
    //reading in >
    else if(ch == '>')
    {
      buff->next_token = OUTPUT_T;
      buff->next_string[numofchar++] = '>';
      buff->next_string[numofchar] = '\0';
      token_finished = 1;
    }   
    else if(ch == '(')
    {
      buff->next_token = OPEN_PAREN_T;
      buff->next_string[numofchar++] = '(';
      buff->next_string[numofchar] = '\0';
      token_finished = 1;
    }   
    else if(ch == ')')
    {
      buff->next_token = CLOSE_PAREN_T;
      buff->next_string[numofchar++] = ')';
      buff->next_string[numofchar] = '\0';
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

command_t get_simple_command(command_stream_t buff)
{
  int numofchar = 0;
  int word_size = 20;
  int current_line;
  int i = 0;
  command_t s = checked_malloc(sizeof(struct command));
  s->u.word = checked_malloc(sizeof(char*) * word_size);

  buff = get_token(buff);
  if(buff->current_token != WORD_T)
      error (1, 0, "Line %d: 1syntax error near unexpected token %s", buff->linenum, buff->next_string);

  s->u.word[i] = checked_malloc(sizeof(char*) * strlen(buff->current_string));
  strcpy(s->u.word[i++], buff->current_string);

  while(buff->next_token == WORD_T && (buff = get_token(buff)))
  {
    s->u.word[i] = checked_malloc(sizeof(char*) * strlen(buff->current_string));
    strcpy(s->u.word[i++], buff->current_string);

    if(i >= word_size)
    {
      word_size += 20;
      s->u.word = checked_realloc(s->u.word, sizeof(char*)* word_size);
    }
  }
  s->u.word[i] = '\0';
  s->type = SIMPLE_COMMAND;
  s->canfork = true;
  return s;
}

command_t get_andor_command(command_stream_t buff)
{
  command_t left_c = get_pipe_command(buff);

  while(buff->next_token == AND_T || buff->next_token == OR_T)
  {
    command_t s = checked_malloc(sizeof(struct command));
    if (buff->next_token == AND_T)
      s->type = AND_COMMAND;
    else
      s->type = OR_COMMAND;

    buff = get_token(buff);
    command_t right_c = get_pipe_command(buff); 

    s->u.command[0] = left_c;
    s->u.command[1] = right_c;
    s->canfork = true;
    left_c = s;
  }
  return left_c; 
}

command_t get_pipe_command(command_stream_t buff)
{
  command_t left_c = get_command(buff);

  while(buff->next_token == PIPE_T)
  {
    buff = get_token(buff);
    command_t right_c = get_command(buff);
    command_t s = checked_malloc(sizeof(struct command));
    
    s->u.command[0] = left_c;
    s->u.command[1] = right_c;
    s->type = PIPE_COMMAND;
    s->canfork = true;
    left_c = s;
  }
  return left_c; 
}

command_t get_complete_command(command_stream_t buff)
{
  command_t left_c = get_andor_command(buff);
  while(buff->next_token == NEWLINE_T || buff->next_token == SEMICOLON_T)
  {
    buff = get_token(buff);
    if(buff->next_token == EOF_T)
      break;
    command_t right_c = get_andor_command(buff);
    command_t s = checked_malloc(sizeof(struct command));
    
    s->u.command[0] = left_c;
    s->u.command[1] = right_c;
    s->type = SEQUENCE_COMMAND;
    s->canfork= true;
    left_c = s;
  }
  return left_c;
}

command_t get_subshell_command(command_stream_t buff)
{
  //need to take the ( first
  buff = get_token(buff);

  command_t c = get_complete_command(buff);

  if(buff->next_token != CLOSE_PAREN_T)
     error (1, 0, "Line %d: syntax error: ')' is missing", buff->linenum);
  buff = get_token(buff);

  command_t s = checked_malloc(sizeof(struct command));
  s->type = SUBSHELL_COMMAND;
  s->u.subshell_command = c;
  s->canfork = true;  

  return s;
}

command_t get_command(command_stream_t buff)
{
  //0 for simple command, 1 for subshell command
  int issimple = 0;
  command_t s;

  while(buff->next_token == NEWLINE_T && (buff = get_token(buff)));

  if(buff->next_token == OPEN_PAREN_T)
  {
    s = get_subshell_command(buff);
    issimple = 1;
  }
  else
    s = get_simple_command(buff);

  if(buff->next_token == INPUT_T)
  {
    buff = get_token(buff);
    if(buff->next_token != WORD_T)
      error (1, 0, "Line %d: syntax error: a word followed by <", buff->linenum);

    buff = get_token(buff);

    s->input = checked_malloc(strlen(buff->current_string));
    strcpy(s->input, buff->current_string);

    if(buff->next_token == OUTPUT_T)
    {
      buff = get_token(buff);
      if(buff->next_token != WORD_T)
        error (1, 0, "Line %d: syntax error: a word followed by >", buff->linenum);

      buff = get_token(buff);

      s->output = checked_malloc(strlen(buff->current_string));
      strcpy(s->output, buff->current_string);
    }
  }

  else if(buff->next_token == OUTPUT_T)
  {
    buff = get_token(buff);
    if(buff->next_token != WORD_T)
      error (1, 0, "Line %d: syntax error: a word followed by >", buff->linenum);

    buff = get_token(buff);

    s->output = checked_malloc(strlen(buff->current_string));
    strcpy(s->output, buff->current_string);
    if(buff->next_token == INPUT_T)
      error (1, 0, "Line %d: syntax error: < followed by a word followed by >", buff->linenum);
  }
  else if(buff->next_token == CLOSE_PAREN_T)
  {
    if(issimple = 1)
      error (1, 0, "Line %d: syntax error: missing a ( before )", buff->linenum);
  }
  return s;
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
  buff->next_string = checked_malloc(sizeof(char*) * buff->token_size);
  buff->stream_size = 30;
  buff->stream = checked_malloc(sizeof (char*));
  buff->linenum = 1;
  buff->current_string[0] = '\0';
  buff->current_token = WORD_T;
  buff->next_string[0] = '\0';
  buff->next_token = WORD_T;
  buff->get_next_byte = get_next_byte;
  buff->get_next_byte_argument = get_next_byte_argument;
  buff->finalcommand = 0;
  buff->firstcommand = 1;
  
  return buff;
}

command_t
read_command_stream (command_stream_t s)
{
  s = get_token(s);
  if(s->next_token == EOF_T)
    return NULL;
  else
  {
    command_t result = get_andor_command(s);
    return result;
  }
}
