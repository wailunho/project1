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
  char* last_string;
  char* current_string;
  int token_size;
  int linenum;
  enum token_type last_token;
  enum token_type current_token;
  int (*get_next_byte) (void *);
  void *get_next_byte_argument;
  int finalcommand;
  int firstcommand;
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
      buff->current_string[numofchar] = '\0';
      token_finished = 1;
    }
    //a command is terminated by either newline or ;
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

command_t get_simple_command(command_stream_t buff)
{
  int numofchar = 0;
  int word_size = 40;
  int current_line;
  command_t s = checked_malloc(sizeof(struct command));
  s->u.word = checked_malloc(sizeof(char*));
  s->u.word[0] = checked_malloc(sizeof(char*) * word_size);
  s->isfinal  = 0;
  strcpy(s->u.word[0], "\0");
  if(buff->last_token != PIPE_T && buff->last_token != AND_T && buff->last_token != OR_T)
   buff->last_token  = buff->current_token;
  current_line = buff->linenum;

  while(1)
  {
    if(buff->last_token != PIPE_T && buff->last_token != AND_T && buff->last_token != OR_T)
     buff = get_token(buff);

    //reading any word
    if(buff->last_token == WORD_T || buff->last_token == START_T)
    {
      while(buff->last_token == START_T && (buff->current_token == NEWLINE_T || 
        buff->current_token == SEMICOLON_T))
      {
        if(buff->current_token == SEMICOLON_T)
          error (1, 0, "Line %d: syntax error near unexpected token `;'", buff->linenum);
        else
          buff = get_token(buff);
      }

      if(buff->current_token == WORD_T)
      {
        numofchar += buff->numofchar;
        if(word_size <= numofchar)
        {
          word_size += 40;
          s->u.word[0] = checked_realloc(s->u.word[0], sizeof(char*) * word_size);
        }
        if(buff->firstcommand == 1)
        {
          buff->firstcommand = 0;
        }
        else
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
      else
      {
        break;
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
    else if (buff->last_token == SEMICOLON_T || buff->last_token == NEWLINE_T || buff->last_token == AND_T || buff->last_token == OR_T
      || buff->last_token == PIPE_T)
    {
      if(buff->last_token == NEWLINE_T && buff->current_token == SEMICOLON_T)
        error (1, 0, "Line %d: syntax error near unexpected token `;'", buff->linenum);
      numofchar += buff->numofchar;
      if(word_size <= numofchar)
      {
        word_size += 40;
        s->u.word[0] = checked_realloc(s->u.word[0], sizeof(char*) * word_size);
      }
      strcat(s->u.word[0], buff->current_string);
    }
    else if (buff->last_token == OPEN_PAREN_T)
    {
      strcat(s->u.word[0], buff->current_string);
      s = get_simple_command(buff);
      if (buff->current_token != CLOSE_PAREN_T)
        error (1, 0, "Line %d: syntax error: expected token )", buff->linenum);
      else
      {
        strcat(s->u.word[0], buff->current_string);
        s->u.word[1] = NULL;
        s->type = SUBSHELL_COMMAND;
        return s;
      }
    }
    else 
      {
        break;
      }
    buff->last_token  = buff->current_token;
    current_line = buff->linenum;
  }
  s->u.word[1] = NULL;
  s->type = SIMPLE_COMMAND;
  return s;
}

command_t get_andor_command(command_stream_t buff)
{
  command_t left_c = get_pipe_command(buff);
  enum token_type token = buff->current_token;

  if(token == AND_T || token == OR_T)
  {
    if(buff->last_token != WORD_T)
      error (1, 0, "Line %d: syntax error near unexpected token %s", buff->linenum, buff->current_string);
    buff = get_token(buff);
    while(buff->current_token == NEWLINE_T)
        buff = get_token(buff);
    if(buff->current_token != WORD_T)
      error (1, 0, "Line %d: syntax error near unexpected token %s", buff->linenum, buff->current_string);
    else
    {

      buff->last_token = token;
      command_t right_c = get_andor_command(buff);
      command_t s = checked_malloc(sizeof(struct command));
      
      s->u.command[0] = left_c;
      s->u.command[1] = right_c;
      if (token == AND_T)
        s->type = AND_COMMAND;
      else
        s->type = OR_COMMAND;

      return s;
    }
  }
  else
  {
    return left_c; 
  }
}

command_t get_pipe_command(command_stream_t buff)
{
  command_t left_c = get_simple_command(buff);
  enum token_type token = buff->current_token;

  if(token == PIPE_T )
  {
    if(buff->last_token != WORD_T)
      error (1, 0, "Line %d: syntax error near unexpected token %s", buff->linenum, buff->current_string);
    buff = get_token(buff);
    while(buff->current_token == NEWLINE_T)
        buff = get_token(buff);
    if(buff->current_token != WORD_T)
      error (1, 0, "Line %d: syntax error near unexpected token %s", buff->linenum, buff->current_string);
    else
    {
      buff->last_token = PIPE_T;
      command_t right_c = get_pipe_command(buff);
      command_t s = checked_malloc(sizeof(struct command));
      
      s->u.command[0] = left_c;
      s->u.command[1] = right_c;
      s->type = PIPE_COMMAND;
      return s;
    }
  }
  else
  {
    return left_c; 
  }
}

command_t get_complete_command(command_stream_t buff)
{
  buff = get_token(buff);
  command_t s;
  if(buff->current_token == WORD_T || buff->current_token == INPUT_T
    || buff->current_token == OUTPUT_T)
  {
    s = get_simple_command(buff);
    return s;
  }
  else if(buff->current_token == OPEN_PAREN_T)
  {
    s = get_simple_command(buff);
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
  buff->current_string[0] = '\0';
  buff->current_token = START_T;
  buff->get_next_byte = get_next_byte;
  buff->get_next_byte_argument = get_next_byte_argument;
  buff->finalcommand = 0;
  buff->firstcommand = 1;
  
  return buff;
}

command_t
read_command_stream (command_stream_t s)
{
  command_t result = get_andor_command(s);
  if (s->finalcommand == 0)
  {
    if(s->current_token == EOF_T)
      s->finalcommand  = 1;
    return result;
  }
  else
  {
    return NULL;
  }
}
