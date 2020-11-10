/*
 * CS252: Systems Programming
 * Purdue University
 * Example that shows how to read one line with simple editing
 * using raw terminal.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <termios.h>


#define MAX_BUFFER_LINE 2048

extern void tty_raw_mode(void);
extern void tty_exit_mode(void);


// Buffer where line is stored
int line_length;
char line_buffer[MAX_BUFFER_LINE];

// Simple history array
// This history does not change. 
// Yours have to be updated.
int history_index = 0;
char * history [MAX_BUFFER_LINE]; //history should be a blank slate!

int history_length = 0;

void read_line_print_usage(){
	char * usage = "\n"
		" ctrl-?       Print usage\n"
		" Backspace    Deletes last character\n"
		" up arrow     See last command in the history\n";

	write(1, usage, strlen(usage));
}

/* 
 * Input a line with some basic editing.
 */
char * read_line() {
	// Set terminal in raw mode
	tty_raw_mode();
	int up_key, down_key = 0; 
	line_length = 0;
	int location = 0;
	// Read one line until enter is typed
	while (1) {
		// Read one character in raw mode.
		char ch;
		read(0, &ch, 1);
		bool flag =ch == 127&& location > 0; 
		if (ch>=32 && ch != 127) {
			// It is a printable character. 

			// Do echo
			if(location == line_length){
				write(1,&ch,1);
				line_buffer[line_length]=ch;
				location++;
			}
			else{
				int size = line_length -location +1; 
				char arr[size];
				int line_length_arr = line_length - location; 

				for(int i = 0; i < line_length_arr; i++){
					arr[i] = line_buffer[location+i];
				}
				line_buffer[location] = ch;
				write(1,&ch,1);
				for (int i = 0; i < line_length_arr; i++){
					line_buffer[location+i+1] = arr[i];
					write(1,&arr[i],1);
				}
				for(int i = 0; i < line_length_arr;i++){
					ch = 8;
					write(1,&ch,1);
				}
			}
			// If max number of character reached return.
			if (line_length==MAX_BUFFER_LINE-2) break; 

			// add char to buffer.

			line_length++;

		}
		else if (ch==10) {
			// <Enter> was typed. Return line

			// Print newline
			write(1,&ch,1);
			
			if(strlen(line_buffer) != 0){
				history[history_length] = (char *)calloc(1, strlen(line_buffer)*sizeof(char)+1); 
				strcpy(history[history_length], line_buffer);
				history_length++;
				history_index = history_length - 1;
				break;
			}
		}
		else if (ch == 31) {
			// ctrl-?
			read_line_print_usage();
			line_buffer[0]=0;
			break;
		}
		else if (ch == 8 || flag ) {
			// <backspace> was typed. Remove previous character read.
			if(location==line_length){
				// Go back one character
				ch = 8;
				write(1,&ch,1);

				// Write a space to erase the last character read
				ch = ' ';
				write(1,&ch,1);

				// Go back one character
				ch = 8;
				write(1,&ch,1);
				line_buffer[line_length-1] = 0;
				// Remove one character from buffer
				location--;
			}
		else{
			ch = 8;
			write(1,&ch,1);
				for (int i = 0; i < line_length - location; i++){
					write(1,&line_buffer[i+location],1);
					line_buffer[i+location-1] = line_buffer[i+location];
				}
				ch = ' ';
				write(1,&ch,1);
				location--;
				line_buffer[line_length-1] = 0;
				for (int i = 0; i  < line_length - location; i++) {
					ch = 8;
					write(1, &ch, 1);
				}	
			}
			line_length--;
		}
		else if ((ch == 4) && location < line_length) {
			for (int i = 0; i < line_length - location ; i++) {
				write(1, (&line_buffer[i + location + 1]), 1);
				line_buffer[i + location] = line_buffer[i + location + 1];
			}
			ch = ' ';
			write(1, &ch, 1);
			line_buffer[line_length] = 0;
			for (int i = 0; i  < line_length - location; i++) {
				ch = 8;
				write(1, &ch, 1);
			}
			line_length--;
   		 }else if(ch==1){
			//home
			while(location > 0){
				ch = 8;
				write(1,&ch,1);
				location--;
			}
		}
		else if(ch==5){
			//end
			while(location <= line_length){
				ch = line_buffer[location];
				write(1,&ch,1);
				location++;
			}
		}
		else if (ch==27) {
			// Escape sequence. Read two chars more
			//
			// HINT: Use the program "keyboard-example" to
			// see the ascii code for the different chars typed.
			//
			char ch1; 
			char ch2;
			read(0, &ch1, 1);
			read(0, &ch2, 1);
			if (ch1==91 && ch2==65) {
				if(down_key == 1 ){
						down_key = 0;
						if(history_index < history_length -1){
						history_index--;
						}
					}
				// Up arrow. Print next line in history.
				if (history_index >= 0){
				// Erase old line
				// Print backspaces
				int i = 0;
				for (i =0; i < line_length; i++) {
					ch = 8;
					write(1,&ch,1);
				}

				// Print spaces on top
				for (i =0; i < line_length; i++) {
					ch = ' ';
					write(1,&ch,1);
				}

				// Print backspaces
				for (i =0; i < line_length; i++) {
					ch = 8;
					write(1,&ch,1);
				}

					
				// Copy line from history
				if(history[history_index][0]=='\n'){
						strcpy(line_buffer,"");
				}




				else{
					strcpy(line_buffer, history[history_index]);
				}
				history_index--;
				line_length = strlen(line_buffer);
		 		for(int i = 0; i < line_length;i++){
					ch = line_buffer[i];
					write(1,&ch,1);
				}
				location = line_length;
				up_key = 1;
				}




			}
			// echo line
			else if(ch1 == 91 && ch2 == 66){
				//Down arrow next command
				// Erase old line
				// Print backspaces
				if(up_key == 1){
					history_index ++;
					up_key = 0;
				}
				int i = 0;
				for (i =0; i < line_length; i++) {
					ch = 8;
					write(1,&ch,1);
				}

				// Print spaces on top
				for (i =0; i < line_length; i++) {
					ch = ' ';
					write(1,&ch,1);
				}

				// Print backspaces
				for (i =0; i < line_length; i++) {
					ch = 8;
					write(1,&ch,1);
				}
				
				// Copy line from history
				if (history_index < history_length-1){
					history_index++;
					if(history[history_index][0]=='\n'){
						strcpy(line_buffer,"");
					}
					else{
						strcpy(line_buffer, history[history_index]);
					}
					down_key = 1;
					
				}
				else{
					strcpy(line_buffer,"");
				}
				line_length = strlen(line_buffer);
				for(int i = 0; i < line_length;i++){
						ch = line_buffer[i];
						write(1,&ch,1);
					}
					location = line_length;
			} //implement left
			else if(ch1 == 91 && ch2 == 68){
				if(location > 0){
					ch = 8;
					write(1,&ch,1);
					location--;		
				}
			} //implement right
			else if(ch1 == 91 && ch2 == 67){
				if(location < line_length){
					ch = line_buffer[location];
					write(1,&ch,1);
					location++;		
				}
			}

		}
	}

	// Add eol and null char at the end of string
	line_buffer[line_length]=10;
	line_length++;
	line_buffer[line_length]=0;

	//if you get to the end and they delete everyting then you need to make sure u add the var prompt back!
	if(line_buffer[0] == '\n'){
					if( isatty(0) ){
						printf("myshell>");
						fflush(stdout);
					}
				
       }
           //tcsetattr(0,TCSANOW,&orig_attr);

	return line_buffer;
}
