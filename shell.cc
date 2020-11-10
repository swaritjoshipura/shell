#include <cstdio>

#include "shell.hh"
#include "command.hh"
#include "signal.h"
#include "stdlib.h"
#include "stdio.h"
#include <sys/types.h>
#include <sys/wait.h>
       #include <unistd.h>


int yyparse(void);
void yyrestart(FILE * file);


void Shell::prompt() {
  if(isatty(0)){

  printf("myshell>");
  fflush(stdout);
  }
}
extern "C" void disp( int sig )
{
    if (sig == SIGINT) {
        bool run = false;
        if (Shell::_currentCommand._simpleCommands.size() > 0) run = true;
        Shell::_currentCommand.clear();
        printf("\n");

        if (run == false){
          Shell::prompt();
        }
    
   }

}
extern "C" void sig_zombie ( int sig ) {
if(sig == SIGCHLD) {
    pid_t pid = wait3(0, 0, NULL);
    while (waitpid(-1, NULL, WNOHANG) > 0) {
   if(isatty(0)) printf("\n[%d] exited.", pid);
   }
}
}

int main() {
    struct sigaction sig_handler;
    sig_handler.sa_handler = disp;
    sigemptyset(&sig_handler.sa_mask);
    sig_handler.sa_flags = SA_RESTART;
    
    if(sigaction(SIGINT, &sig_handler, NULL)){
        perror("sigaction");
        exit(2);
    }
    
  struct sigaction sig_int_action;
	sig_int_action.sa_handler = sig_zombie;
	sigemptyset(&sig_int_action.sa_mask);
	sig_int_action.sa_flags = SA_RESTART;

	if (sigaction(SIGCHLD, &sig_int_action, NULL)) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}

  FILE * fd = fopen(".shellrc", "r");
	if (fd) {
		yyrestart(fd);
		yyparse();
		yyrestart(stdin);
		fclose(fd);
	} else {
		if ( isatty(0) ) {
  			Shell::prompt();

		}
	}
  yyparse();
}

Command Shell::_currentCommand;
