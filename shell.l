
/*
 *
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <string.h>
#include <fcntl.h>
#include "y.tab.hh"
////////////  Start added code ///////////
extern "C" char * read_line();

int mygetc(FILE * f) {
 static char *p;
 char ch;

 if (!isatty(0)) {
 // stdin is not a tty. Call real getc
   return getc(f);
 }

 // stdin is a tty. Call our read_line.
 if (p==NULL || *p == 0) {
   char * s = read_line();
   p = s;
 }

 ch = *p;
 p++;

 return ch;
}

#undef getc
#define getc(f) mygetc(f)
/////////// End added code ///////////
static  void yyunput (int c,char *buf_ptr  );




void myunputc(int c) {
  unput(c);
}

%}

%option noyywrap

%%

\n {
  return NEWLINE;
}
[ \t] {
  /* Discard spaces and tabs */
}

">" {
  return GREAT;
}
"<" {
  return LESS;
}

">>" {
  return GREATGREAT;
}

">&" {
  return GREATAMPERSAND;
}

">>&" {
  return GREATGREATAMPERSAND;
}

"|" {
  return PIPE;
}

"&" {
  return AMPERSAND;
}

"2>" {
  return ERR;
}

(source)[^\t\n]+ {
  char * yySTR = yytext;
  char * yyTextSS = new char[strlen(yytext) -4];
  int ptr = 0;
  for(int i = 7; i < strlen(yytext);i++) {
    yyTextSS[ptr++] = yySTR[i];
  }


  yyTextSS[strlen(yytext)-4] = '\0';
  yylval.cpp_string = new std::string(yyTextSS);
  FILE * srcf = fopen(yyTextSS, "r");
  if(!srcf) {
    return NEWLINE;
  }
  YY_BUFFER_STATE original = YY_CURRENT_BUFFER;
  YY_BUFFER_STATE cp_of_file = yy_create_buffer(srcf, YY_BUF_SIZE);
  if(cp_of_file == NULL) {
    return NEWLINE;
  }
  yy_switch_to_buffer(cp_of_file);
  yyparse();
  yy_delete_buffer(cp_of_file);
  yy_switch_to_buffer(original);

  }

\$\([^\n\$]*\)|`[^\n`]*` {
  std::string command_gene = std::string(yytext);
  if (strchr(command_gene.c_str(), '`') != NULL) {
    command_gene =  std::string(yytext + 1);
      }
  else {
    command_gene =  std::string(yytext + 2);
    
  }
  command_gene.pop_back();

  int inp = dup(0);
  int out = dup(1);

  int pipes_in[2];
  int pipes_out[2];

  pipe(pipes_in);
  pipe(pipes_out);

  dup2(pipes_in[0], 0);
  dup2(pipes_out[1], 1);
  close(pipes_in[0]);
  close(pipes_out[1]);

  write(pipes_in[1], command_gene.c_str(), command_gene.size());
  write(pipes_in[1], "\n", 1);
  close(pipes_in[1]);


  close(pipes_in[0]);
  close(pipes_out[1]);

  int ret = fork();
  if (ret == 0) {
  //  const char ** argos = {NULL, NULL};
    std::vector<char *> char_V (2);
    char_V[0] = const_cast <char*>("/proc/self/exe");
    char_V[1] = nullptr;
    execvp(char_V[0], char_V.data());
        //execlp("/proc/self/exe", "/proc/self/exe", NULL);

    _exit(1);
  }
    else {
  waitpid(ret, NULL, 0);
  dup2(inp, 0);
  dup2(out, 1);
  close(inp);
  close(out);

  int ind = 0;
  char outc;
  char * buf = new char[7000];
  while (read(pipes_out[0], &outc, 1)) {
    if (outc == '\n') {
      buf[ind] = '\t';
    }
    else {
      buf[ind] = outc;
    }
    ind++;
  }
  buf[ind] = '\0';
  for (int i = ind - 1; i >= 0; i--) {
   myunputc(buf[i]);
  }
  close(pipes_out[0]);
  command_gene.clear();
  }
}














































[^ \t\n|><&]*\\[^ \t\n]* {
  int i=0;
	char * sequence; 
	sequence = new char[strlen(yytext)*2];
	char * args = yytext;
	while ( *args ) {
		if ( *args == '\\' ) {	
			sequence[i] = *(++args);
			i++;	
		} else {
			sequence[i] = *args;
			i++;
		}
		args++;
	}
	sequence[i] = '\0';
	yylval.cpp_string = new std::string(sequence);
	return WORD;
}

["][^\n\"]*["] {
int i = 0;
char * quotes = new char[strlen(yytext)];
char * text_generate = yytext;

while ( *text_generate ) {
  if ( *text_generate == '\"' ) {
    quotes[i] = *(++text_generate);
    i++;
  }
  else {
    quotes[i] = *text_generate;
    i++;
   }
  text_generate++;
}
quotes[i] = '\0';
yylval.cpp_string = new std::string(quotes);
	return WORD;
}


[^ \t\n|><][^ \t\n|><]*  {
  /* Assume that file names have only alpha chars */
  yylval.cpp_string = new std::string(yytext);
  return WORD;
}



