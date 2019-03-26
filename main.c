#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include "lexer.h"

//Parses a command and its arguments
char **parse_command(struct LEXER *lx) {
	//Set up pointers for program and arguments
	char **cmd = malloc(sizeof(char*)*16+1);
	int cmd_cur = 1;
	int cmd_max = 16;

	//Check that the first token is a string (program name)
	if (lx->c_tok != TOK_STRING) {
		printf("Error: missing program name\n");
		lexer_adv(lx);
		free(cmd);
		return NULL;
	}
	cmd[0] = lx->tok_str;
	lexer_adv(lx);

	//Read arguments
	while (lx->c_tok == TOK_STRING) {
		//Perform reallocation if needed
		cmd[cmd_cur] = lx->tok_str;
		if (++cmd_cur == cmd_max) {
			cmd_max *= 2;
			cmd = realloc(cmd, sizeof(char*)*cmd_max+1);
		}
		lexer_adv(lx);
	}

	//Add NULL to the end of the array
	cmd[cmd_cur] = NULL;
	return cmd;
}

//Processes a command chain (one or more commands in pipe)
void process_command(struct LEXER *lx) {
	//Loop for each command in pipe
	int input_fd = STDIN_FILENO, output_fd = STDOUT_FILENO;
	while (lx->c_tok != TOK_SEMI && lx->c_tok != TOK_END) {
		//Read a command from the lexer
		char **cmd = parse_command(lx);

		//Check if the following token is a pipe
		int pipe_fd[2] = { -1, -1 };
		if (lx->c_tok == TOK_PIPE) {
			lexer_adv(lx);
			pipe(pipe_fd);
			output_fd = pipe_fd[1];
		}

		//Check if the following token is file redirection
		switch (lx->c_tok) {
			case TOK_LRDIR: //Input from file
				lexer_adv(lx);
				input_fd = open(lx->tok_str, O_RDONLY);
				free(lx->tok_str);
				lexer_adv(lx);
				break;
			case TOK_RRDIR: //Output to file
				lexer_adv(lx);
				output_fd = open(lx->tok_str, O_WRONLY | O_CREAT, 0644);
				free(lx->tok_str);
				lexer_adv(lx);
				break;
			case TOK_APPEND: //Append to file
				lexer_adv(lx);
				output_fd = open(lx->tok_str, O_WRONLY | O_CREAT | O_APPEND, 0644);
				free(lx->tok_str);
				lexer_adv(lx);
				break;
		}

		//Execute the command
		int pid = fork();
		switch (pid) {
			case 0: //Child
				//Redirect input and output
				dup2(input_fd, STDIN_FILENO);
				dup2(output_fd, STDOUT_FILENO);

				//Run the command
				execvp(cmd[0], cmd);
				return;
			default: //Parent
				waitpid(pid, NULL, 0);
				break;
			case -1: //Error
				break;
		}

		//If a pipe was made, save the read side of the pipe for the next command
		if (pipe_fd[0] > 0) {
			input_fd = pipe_fd[0];
			close(pipe_fd[1]);
		}

		//Free all parts of the command
		for (int i=0; cmd[i] != NULL; i++)
			free(cmd[i]);
	}

	//Advance past semicolon
	if (lx->c_tok == TOK_SEMI)
		lexer_adv(lx);
	return;
}

//Execute all commands on the line, delimited by semicolons
void process_input(char *str) {
	//Initialize lexer
	struct LEXER lx;
	lexer_init(&lx, str);

	//Run all commands on the line
	while (lx.c_tok != TOK_END)
		process_command(&lx);

	free(str);
	return;
}

//Entry point
int main(int argc, char **argv) {
	//Main loop
	int run = 1;
	while (run) {
		//Get hostname and current working directory
		char hbuf[256];
		char cdbuf[256];
		gethostname(hbuf, 256);
		getcwd(cdbuf, 256);

		//Get the name of the current directory and the prompt symbol
		char *dirname = strrchr(cdbuf, '/') + ((strlen(hbuf) == 1) ? 0 : 1);
		char psym = (!strcmp(getenv("USER"), "root")) ? '#' : '$';

		//Print the prompt
		printf("[%s@%s %s]-A3SHELL%c ", getenv("USER"), hbuf, dirname, psym);

		//Read and process user input
		char *buf = NULL;
		size_t bufsize;
		getline(&buf, &bufsize, stdin);
		process_input(buf);
	}

	return 0;
}
