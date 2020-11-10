
/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%code requires 
{
#include <string>
#include <string.h>

#if __cplusplus > 199711L
#define register      // Deprecated in C++11 so remove the keyword
#endif
}

%union
{
  char        *string_val;
  // Example of using a c++ type in yacc
  std::string *cpp_string;
}

%token <cpp_string> WORD
%token NOTOKEN GREAT LESS GREATGREAT GREATAMPERSAND GREATGREATAMPERSAND AMPERSAND PIPE ERR NEWLINE
%{
//#define yylex yylex
#include <cstdio>
#include <cstring>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <regex.h>
#include <algorithm>
#include <vector>
#include "shell.hh"
void yyerror(const char * s);
int yylex();
void expandWildcard(std::string prefix, std::string suffix, bool dir_exists);



%}

%%

goal:
  commands
  ;

commands:
  command
  | commands command
  ;

command: simple_command
       ;

simple_command:
  pipe_list iomodifier_list background_opt NEWLINE {
    /*printf("   Yacc: Execute command\n");*/
    Shell::_currentCommand.execute();
  }
  | NEWLINE
  | error NEWLINE { yyerrok; }
  ;

pipe_list:
  command_and_args
  |
  pipe_list PIPE command_and_args
  ;

command_and_args:
  command_word argument_list {
    Shell::_currentCommand.
    insertSimpleCommand( Command::_currentSimpleCommand );
  }
  ;
argument_list:
  argument_list argument
  | /* can be empty */
  ;
argument:
  WORD {
    expandWildcard("", *($1), false);\
    }
  ;
command_word:
  WORD {
    /*printf("   Yacc: insert command \"%s\"\n", $1);*/
    Command::_currentSimpleCommand = new SimpleCommand();
    Command::_currentSimpleCommand->insertArgument( $1 );
  }
  ;
iomodifier_list:
  iomodifier_list iomodifier_opt
  |
  ;
iomodifier_opt:
  GREAT WORD {
    /*printf("   Yacc: insert output \"%s\"\n", $2);*/
     if (Shell::_currentCommand._outFile) {
     Shell::_currentCommand._outFile_two = true;
    }
    Shell::_currentCommand._outFile = $2; 
    }
  |
  LESS WORD {
     if (Shell::_currentCommand._inFile) {
     Shell::_currentCommand._inFile_two= true;
     }
      Shell::_currentCommand._inFile = $2;
        }
  |
  GREATGREAT WORD {
 if (Shell::_currentCommand._outFile) {
    Shell::_currentCommand._outFile_two=true;
 }
      Shell::_currentCommand._outFile = $2;
      Shell::_currentCommand._outOW = true;
     }
  |
  GREATAMPERSAND WORD{
   if (Shell::_currentCommand._outFile){
    Shell::_currentCommand._outFile_two=true;
    }
      if (Shell::_currentCommand._errFile){
      Shell::_currentCommand._errFile_two= true;
      }
      Shell::_currentCommand._outFile = $2;
      Shell::_currentCommand._errFile = $2;
  }
  |
  GREATGREATAMPERSAND WORD{
   if (Shell::_currentCommand._outFile){ 
   Shell::_currentCommand._outFile_two=true;
   }
      if (Shell::_currentCommand._errFile){ 
      Shell::_currentCommand._errFile_two = true;
      }
      Shell::_currentCommand._outFile = $2;
      Shell::_currentCommand._errFile = $2;
       Shell::_currentCommand._outOW= true;
      Shell::_currentCommand._outOW= true;
      }
  |
  ERR WORD{
     if (Shell::_currentCommand._errFile){
     Shell::_currentCommand._errFile_two = true;
      }
      Shell::_currentCommand._errFile = $2;  
      }
  ;
background_opt:
  AMPERSAND {
         Shell::_currentCommand._background = true;
  }
  |/* Can be empty */
  ;

%%

void
yyerror(const char * s)
{
  fprintf(stderr,"%s", s);
}



void expandWildcard(std::string prefix, std::string suffix, bool dir_exists) {
    if (suffix == "") {
        Command::_currentSimpleCommand->insertArgument(new std::string(prefix.begin() + 1, prefix.end()));
        return;
    }
    std::string s_in_arr;
    auto slash = suffix.find("/");
    if (slash == std::string::npos) {
        s_in_arr = suffix;
        suffix = "";
    }
    else {
        s_in_arr = std::string(suffix.begin(), suffix.begin() + slash);
        suffix = suffix.substr(slash + 1, suffix.size() - slash);
    }
    auto a_sand = s_in_arr.find("*");
    auto q_mark= s_in_arr.find("?");
    if ((a_sand == std::string::npos) && (q_mark == std::string::npos)) {        
        expandWildcard(prefix + "/" + s_in_arr, suffix, dir_exists);
        return;
    }
    std::string reg = "^";
    bool check = false;
    if (s_in_arr[0] == '.'){
    check = true;
    }
    for (int i = 0; i < s_in_arr.size(); i++) {
            if(s_in_arr[i] == '*') {
                reg += ".*";
              }
           else if(s_in_arr[i] == '?') {
                reg += ".";
             }
           else  if(s_in_arr[i] == '.') {
                reg += "\\.";
           }
            else{
                reg += s_in_arr[i];
             }
    }
    reg += "$";
    regex_t r_struct;
    regcomp(&r_struct, reg.c_str(), REG_EXTENDED | REG_NOSUB);
    struct dirent **dir_list;
    int rep_count = 0;
    if ((prefix != "") || (rep_count < 0 && check)) {
        rep_count = scandir(prefix.c_str(), &dir_list, 0, alphasort);
    }
    else {
        rep_count = scandir(".", &dir_list, 0, alphasort);
    }
    for (int i = 0; i < rep_count; i++) {
        if (regexec(&r_struct, (*(dir_list + i))->d_name, 0, NULL, 0) == 0) {
            dir_exists = true;
            std::string p1 = "";
            if (*(s_in_arr.begin()) == '.') {
                p1 = (*(dir_list + i))->d_name;
            }
            else {
               p1 = prefix + "/" + (*(dir_list + i))->d_name;
            }
            if (*((*(dir_list + i))->d_name) != '.') {
                expandWildcard(prefix + "/" + (*(dir_list + i))->d_name, suffix, dir_exists);
            } else if (check) {
                expandWildcard(std::string(".") + (*(dir_list + i))->d_name, suffix, dir_exists);
            }
        }
    }
    if (dir_exists == false) {
        expandWildcard(prefix + suffix + "/" + s_in_arr, "", dir_exists);
    }
}
#if 0
main()
{
  yyparse();
}
#endif
