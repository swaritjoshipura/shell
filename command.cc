/*
 * Swarit Joshipura Shell
 */

#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <pwd.h>

#include <regex>

#include "command.hh"
#include "shell.hh"

#include <iostream>    
#include <fstream>

int bPID =0;
int status;
void myunputc(int c);

std::string* val_to_store;

Command::Command() {
    // Initialize a new vector of Simple Commands
    _simpleCommands = std::vector<SimpleCommand *>();

    _outFile = NULL;
    _inFile = NULL;
    _errFile = NULL;
    _background = false;


    _outOW = false;
    _errOW = false;
    _inFile_two = false;
    _outFile_two = false;
    _errFile_two = false;
}

void Command::insertSimpleCommand( SimpleCommand * simpleCommand ) {
    // add the simple command to the vector
    _simpleCommands.push_back(simpleCommand);
}

void Command::clear() {
    // deallocate all the simple commands in the command vector
    for (auto simpleCommand : _simpleCommands) {
        delete simpleCommand;
    }

    // remove all references to the simple commands we've deallocated
    // (basically just sets the size to 0)
    _simpleCommands.clear();

    if ( _outFile) {
        delete _outFile;
    }
    _outFile = NULL;

    if ( _inFile ) {
        delete _inFile;
    }
    _inFile = NULL;

    if ( _errFile ) {
        delete _errFile;
    }
    _errFile = NULL;
    _background = false;
    _inFile_two = false;
    _outFile_two = false;
    _errFile_two = false;
    _outOW = false;
    _errOW = false;
}


void Command::func_1(){
    for(unsigned int x=0;x<_simpleCommands.size();x++){
	for(unsigned int y=0;y<_simpleCommands.at(x)->_arguments.size();y++){
	    std::string* string_to_expand = new std::string();
	    const char * argfs = _simpleCommands.at(x)->_arguments.at(y)->c_str();
	    char* dup_val = strdup(argfs);

	    if (strchr(argfs, '$') && strchr(argfs, '{')) {
		int i = 0;
		int j = 0;
		int flag=1;
		while(argfs[i] != '\0') {
		    flag=1;
		    if (argfs[i] == '$') {
			char* var_temp = (char*)malloc(strlen(argfs));
			i+=2;
			while (argfs[i] != '}') {
			    var_temp[j] = argfs[i];
			    i++;
			    j++;
			}
			var_temp[j] = '\0';
			if(strcmp(var_temp,"$")!=0 && strcmp(var_temp,"?")!=0 && strcmp(var_temp,"_")!=0 && strcmp(var_temp,"SHELL")!=0 && strcmp(var_temp,"!")!=0){
			    char* s = getenv(var_temp);
			    std::string toUse2 = std::string(s);
			    string_to_expand->append(s);
			}
			//3.1 - expand stuff


			if(strcmp(var_temp,"SHELL")==0){
			    char actualpath [100];
			    char * path = realpath("/proc/self/exe",actualpath);
			    std::string* newString = new std::string(path);
			    _simpleCommands.at(x)->_arguments.at(y) = newString;
			    flag=0;
			}





			else if(strcmp(var_temp,"?")==0){
			    std::string temp_string = std::to_string(status);
			    std::string* needle = new std::string(temp_string);
			    _simpleCommands.at(x)->_arguments.at(y) = needle;
			    flag=0;
			}




			else if(strcmp(var_temp,"$")==0){
			    std::string temp_string = std::to_string(getpid());
			    std::string* needle = new std::string(temp_string);
			    _simpleCommands.at(x)->_arguments.at(y) = needle;
			    flag=0;
			} 



			else if(strcmp(var_temp,"_")==0){
			    _simpleCommands.at(x)->_arguments.at(y) = val_to_store;
			    flag=0;
			}


			else if(strcmp(var_temp,"!")==0){
			    std::string temp_string = std::to_string(bPID);
			    std::string* needle = new std::string(temp_string);
			    _simpleCommands.at(x)->_arguments.at(y) = needle;
			    flag=0;
			}
			
//end 3.1

			free(var_temp);
			j = 0;
		    }
		    else {
			char* var_temp = (char*)malloc(strlen(argfs));
			while(argfs[i] != '\0' && argfs[i] != '$') {
			    var_temp[j] = argfs[i];
			    i++;
			    j++;
			}
			var_temp[j] = '\0';
			string_to_expand->append(std::string(var_temp));;
			free(var_temp);
			j=0;
			i--;
		    }
		    i++;
		}
		if(flag==1){
		    _simpleCommands.at(x)->_arguments.at(y) = string_to_expand;
		}
	    }  
	    free(dup_val);
	}

	val_to_store = new std::string(_simpleCommands.at(x)->_arguments.back()->c_str());
    }
}










//tilde stuff

void Command::tilde(std::string* arg) {
  while (1) {   
    auto tilde_checker = std::find(arg->begin(), arg->end(), '~');
    if (tilde_checker == arg->end()){
        break;
    }


    if (arg->size() == 1 || *(tilde_checker + 1) == '/') {          
        struct passwd* pwd = getpwnam(getenv("USER"));
      if (pwd) {
        arg->replace(tilde_checker, tilde_checker + 1, pwd->pw_dir);
      } 
      else {
      break;
      }
    }
    else {       
      auto not_present = std::find(tilde_checker, arg->end(), '/');
      std::string uname(tilde_checker + 1, not_present);
      struct passwd* pwd = getpwnam(uname.c_str());
      if (pwd) {
        arg->replace(tilde_checker, not_present, pwd->pw_dir);
      } 
      else {
           break;
        }
      }
  }
}

void Command::call_functions() {
  for (auto cmd : _simpleCommands) {
    for (auto arg : cmd->_arguments) {
      if (arg) {
        tilde(arg);
       }
    }
  }
}








void Command::print() {
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    int i = 0;
    // iterate over the simple commands and print them nicely
    for ( auto & simpleCommand : _simpleCommands ) {
        printf("  %-3d ", i++ );
        simpleCommand->print();
    }

    printf( "\n\n" );
    printf( "  Output       Input        Error        Background\n" );
    printf( "  ------------ ------------ ------------ ------------\n" );
    printf( "  %-12s %-12s %-12s %-12s\n",
            _outFile?_outFile->c_str():"default",
            _inFile?_inFile->c_str():"default",
            _errFile?_errFile->c_str():"default",
            _background?"YES":"NO");
    printf( "\n\n" );
}

void Command::execute() {
    // Don't do anything if there are no simple commands
    if ( _simpleCommands.size() == 0 ) {
        if (isatty(0)) Shell::prompt();
        return;
    }
    
    if (_inFile_two) {
        std::cout << "Ambiguous input redirect." << std::endl;
        
        return;
    } else if (_outFile_two) {
        std::cout << "Ambiguous output redirect." << std::endl;
        
        return;
    }
    else if (_errFile_two) {
        std::cout << "Ambiguous error redirect." << std::endl;
        
        return;
    }
    if ( !strcmp(_simpleCommands[0]->_arguments[0]->c_str(),"exit")) {
		printf( "Good Bye!\n" );
		_exit(1);
	}

    func_1();
    call_functions();
    // Print contents of Command data structure
    if (isatty(0)) print();
    int dup_in_file = dup(0);
    int dup_out_file = dup(1);
    int dup_err_file = dup(2);
    
    int in_file_copy;
    int out_file_copy;
    int err_file_copy;
    if (_inFile) {
        in_file_copy = open(_inFile->c_str(), O_RDONLY, 0664);
    } 
    else {
        in_file_copy = dup(dup_in_file); 
    }
    pid_t ret;
    for (size_t i = 0; i < _simpleCommands.size(); i++) {
        dup2(in_file_copy, 0);
        close(in_file_copy);
        if(!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "setenv")){
          if(setenv(_simpleCommands[i]->_arguments[1]->c_str(), _simpleCommands[i]->_arguments[2]->c_str(), 1)) {
           perror("setenv");
          }
           clear();
           Shell::prompt();
             return;
        }
        if(!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "unsetenv")){

            if(unsetenv(_simpleCommands[i]->_arguments[1]->c_str())) {
                perror("unsetenv");
            }
            clear();
            Shell::prompt();
            return;
        }

if (!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "cd")) {
    if (_simpleCommands[i]->_arguments.size() == 1) {
      if( chdir(getenv("HOME")))
        perror("cd");
    }
     else {
      const char * path = _simpleCommands[i]->_arguments[1]->c_str();
      char to_print[1024] = "cd: can't cd to ";
      strcat(to_print, path);
       if (chdir(path)){
          perror(to_print);
       }
     }
    clear();
    Shell::prompt();
    return;
}
        if (i == _simpleCommands.size() - 1) {
            if (_outFile) {
                if (_outOW) out_file_copy = open(_outFile->c_str(), O_WRONLY | O_APPEND | O_CREAT, 0664);

                else {
                    out_file_copy = open(_outFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0664);
                }
            } 
            else {
                out_file_copy = dup(dup_out_file);
            }
            
            if (_errFile) {
                if (_errOW) err_file_copy = open(_errFile->c_str(), O_WRONLY | O_APPEND | O_CREAT, 0664);
                else {
                    err_file_copy = open(_errFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0664);
                }
            } else {
                err_file_copy = dup(dup_out_file);
            }
            dup2(err_file_copy, 2);
            close(err_file_copy);
            
        }
        else {
            int pipe_arr[2];
            pipe(pipe_arr);
            in_file_copy = pipe_arr[0];
            out_file_copy = pipe_arr[1];
        }

        dup2(out_file_copy, 1);
        close(out_file_copy);
         ret = fork();
        if (ret == 0) {
          if (*(_simpleCommands[i]->_arguments[0]) == "printenv") {
                char **env= environ;

                while (*env != NULL) {
                    printf("%s\n", *env);
                    env++;
                }
                //continue;
                fflush(stdout);
                Shell::prompt();
                _exit(1);
            }
            // Convert vector<string*> to vector<char*>
            std::vector<char *> char_V;
            for (auto str : _simpleCommands[i]->_arguments) {
                char_V.push_back(const_cast <char*>(str->c_str()));
            }
            char_V.push_back(nullptr);
            execvp(char_V[0], char_V.data());
        }
    }

    if(!_background) {
      	int rc;
        	if ( waitpid(ret, &rc, 0) == -1 ) {
	         // perror("waitpid failed");
         	}
	        const int pin = WEXITSTATUS(rc);
         	status = pin;
	        clear();
      }
      else{
        	bPID = ret;
    }


       dup2(dup_in_file, 0);
    dup2(dup_out_file, 1);
    dup2(dup_err_file, 2);
    close(dup_in_file);
    close(dup_out_file);
    close(dup_err_file);
    close(in_file_copy);
    close(out_file_copy);
    close(err_file_copy);
    // Clear to prepare for next command
    clear();
    // Print new prompt
    Shell::prompt();
}

SimpleCommand * Command::_currentSimpleCommand;

