// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <stdio.h>


#include "alloc.h"
  
#include <error.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
  
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h> 


int simple_command(command_t x, bool time_travel, bool andor);
int pipe_command(command_t x, bool time_travel);
int sequence_command(command_t x, bool time_travel);
int andor_command(command_t x, bool time_travel);
int new_command(command_t x, bool time_travel, bool andor);



//
/*
Plan
1) Subshells
2) I/O redirects
3) Pipelines
4) AND/ORs
*/
//


//0-stdin
//1-stdout
void swap_descriptors(char c, command_t x, int *fdnew, int fdold)
{

	switch(c){
	case 's':  //swap case-->swap file descriptors
		//check if input file exists
		if(x->input !=NULL)
		{
			//swap fd with stdin and file input
			if((*fdnew = open(x->input, O_RDONLY)) == -1)
			{
				fprintf(stderr, "Input file failed to open\n");
				exit(-1);
			}
			else
			{
				close(0);
				dup2(*fdnew, 0);
				close(*fdnew);
			}
		}
		//check if output file exists
		if(x->output !=NULL)
		{
			//swap fd with stdout and file output
			if((*fdnew = open(x->output, O_WRONLY | O_CREAT, 0666)) == -1)
			{
				fprintf(stderr, "failed to create output file\n");
				exit(-1);
			}
			else
			{
				close(1);
				dup2(*fdnew,1);
				close(*fdnew);
			}
		}
		break;

	case 'r':	//reset file descriptor case
		if(dup2(*fdnew, fdold) < 0)	//copy the new fd to the old fd
		{
			fprintf(stderr, "could not reset input | output\n");
			exit(-1);
		}
		break;
	
	default:
		fprintf(stderr, "unexpected error detected\n");
		exit(-1);
		break;
	}
}

int
simple_command(command_t x, bool time_travel, bool andor)
{
    int r_value;
    int c_value=-1;
    int fdnew = -1;
    int fdout = dup(1);
    int fdin = dup(0);
    char ** argv = x->u.word;
    pid_t t;



    //swap IO?

    swap_descriptors('s', x, &fdnew, 0);
    if(x->canfork == true)
    {
	//fork process
	if((t = fork()) <0)
	{
		fprintf(stderr, "failed to fork\n");
		return -1;
	}
	if( t == 0) //child process
	{
		//execute process
		if(execvp(argv[0], &(argv[0])) == -1)
		{
			fprintf(stderr, "failed to execute\n");
			return -1;
		}
	}
	else if (t < 0)
	{
		fprintf(stderr, "failed to fork\n");
		return -1;
	}
	else	//wait for parent process to finish
	{

		//wait for process to finish, store turn value in r_value
		if(waitpid(-1,&r_value, 0) < 0)
		{
			fprintf(stderr, "wait failure\n");
			return -1;
		}
		//close fdnew, and reset fdout and fdin
		close(fdnew);
		swap_descriptors('r', x, &fdout, 1);
		swap_descriptors('r', x, &fdin, 0);

		if(WIFEXITED(r_value))//if the child exited
		{
			c_value =WEXITSTATUS(r_value);	
		}
		else
		{
			fprintf(stderr, "command faliure\n");
			return -1;
		}
	}
	x->status = WEXITSTATUS(r_value);
	return c_value;

    }
    else
    {

	    if(execvp(argv[0], &(argv[0])) == -1)
   	    {
	      fprintf(stderr, "failed to execute simple command\n");
	      return -1;
            }
    }    
    x->status = r_value;
    return c_value;
}

int
pipe_command(command_t x, bool time_travel)
{

	int fd[2] = {-1, -1};	//fd has the file descriptors for parent and child
	pid_t childpid1;	//pid of first child
	pid_t childpid2;	//pid of second child
	int r_value1;		//return value of first parent
	int r_value2;		//return value of second parent
	int c_value1 = -1;	//first child's return value
	int c_value2 = -1;	//second child's return value

	if(pipe(fd) < 0)
	{
		fprintf(stderr, "failed pipe\n");
		return -1;
	}


	//======= CHILD PROCESS 1 =============//
	if( childpid1 = fork()  < 0)
	{
		fprintf(stderr, "failed to fork\n");
		return -1;
	}
	else if( childpid1 == 0)  //run first child
	{
		if(x->u.command[0]->output != NULL)
		{
			fprintf(stderr, "No redirect location.\n");
			_exit(-1);//exit the calling process immediately
		}

		close(fd[0]);	//close parent's read
		close(1);	//close stdout
		dup2(fd[1], 1);	//set stdout to child fd
		close(fd[1]);   //close child write

		x->u.command[0]->canfork = false; //prevents additional forking
		r_value1  = new_command(x->u.command[0], time_travel, false);
		_exit(r_value1);	
	}
	else  //run first parent process
	{
		close(fd[1]); //close child's read
		if(waitpid(childpid1, &(r_value1),0) < 0)
		{
			fprintf(stderr, "child 1 wait fail\n");
			x->u.command[0]->status = r_value1;
			return -1;

		}
		x->u.command[0]->status = r_value1;
		if(WIFEXITED(r_value1))  //child exited properly
		{
			c_value1 = WEXITSTATUS(r_value1);
		}
	}
	
	//==================END CHILD PROCESS ============

	//first child exited
	if(c_value1 ==0 )
	{	//fork the second child
		if(childpid2 = fork() < 0)
		{
			fprintf(stderr, "2nd child failed to fork\n");
			return -1;
		}
		else if (childpid2 ==0)
		{
			if(x->u.command[1]->input != NULL)
			{
				fprintf(stderr, "Ambiguous input redirect.\n");
				_exit(-1);
			}	
			close(fd[1]);
			close(0);
			dup2(fd[0], 0);
			close(fd[0]);
		
			x->u.command[1]->canfork= false;
			r_value2 = new_command(x->u.command[1], time_travel, false);
			_exit(r_value2);
		}
		else	
		{
			close(fd[0]);
			if(waitpid(childpid2, &r_value2, 0)<0)
			{
				fprintf(stderr, "child2 wait fail\n");
				x->u.command[1]->status = r_value2;
				return -1;
			}
			x->u.command[1]->status = r_value2;
			if(WIFEXITED(r_value2))
			{
				c_value2 = WEXITSTATUS(r_value2);
			}
			
		}
	}

	return (c_value1 || c_value2);

}

int
sequence_command(command_t x, bool time_travel)
{
	int r_value1=0;
	int r_value2=0;
	
	if(x->u.command[0] != NULL){
		r_value1 = new_command(x->u.command[0], time_travel, false);
	}
	if(x->u.command[1] != NULL){
		r_value2= new_command(x->u.command[1], time_travel, false);
	}

	return (r_value1 || r_value2);
}


int
andor_command(command_t x, bool time_travel)
{
	int r_value;
	switch(x->type){
		case AND_COMMAND:
			r_value=new_command(x->u.command[0], time_travel, true);
			if(r_value >= 0)
			{
				new_command(x->u.command[1], time_travel, true);
			}
			else if(r_value == -1)
			{
				fprintf(stderr, "AND fail\n");
				return -1;
			}
			break;
		case OR_COMMAND:
			r_value=new_command(x->u.command[0], time_travel, true);
			if(r_value == -1)
			{
				//left side or failed, but run right side
				r_value = new_command(x->u.command[1], time_travel, true);
			} else
			{
				r_value = -2;
				return r_value;
			}
			break;
		default:
			break;
	}
	return r_value;

}
//

//#1 Subshells
//recursive function
//depth first search
//andor - true if used before or after a && or ||, false otherwise
int
new_command(command_t x, bool time_travel, bool andor)
{


int cmd_complete = -1;	 //return value of command is completed
int fdnew = -1; 	 //variable for new file descriptor
int fdout = dup(1);	 //initialize fdout to stdout
int fin = dup(0);	 //initialize fin to stdin

	switch(x->type)
	{
		case SIMPLE_COMMAND:
			//execute simple command
			cmd_complete = simple_command(x, time_travel, false);	
			if(cmd_complete < 0)
			{
				fprintf(stderr, "simple command failed\n");
				return -1;
			}
			break;
		case SEQUENCE_COMMAND:
			cmd_complete = sequence_command(x, time_travel);
			break;
		case AND_COMMAND:
		case OR_COMMAND:
			//do and/ors
			cmd_complete= andor_command(x, time_travel);
			break;
		case PIPE_COMMAND:
			//do pipe
			cmd_complete = pipe_command(x, time_travel);
			break;
		case SUBSHELL_COMMAND:

			//a subshell command is the start of a new command
			//recursively call new_command
			swap_descriptors('s', x, &fdnew, fdout);
			cmd_complete = new_command(x->u.subshell_command, time_travel, false);
			close(fdnew);
			swap_descriptors('r', x, &fdout, 1);
			swap_descriptors('r', x, &fin, 0);
			break;
		default:
			fprintf(stderr, "Invalid command type\n");
			exit(1);
	}
	return cmd_complete;
}

void
execute_command (command_t x, bool time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  int run = -1;
  if(time_travel)
  {
		error (1, 0, "command execution not yet implemented");
  }
  else {
	run = new_command(x, time_travel, false);
	
  }
}



int
command_status (command_t c)
{
  return c->status;
}


