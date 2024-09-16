/*120403934*/
#include <stdio.h>
#include "command.h"
#include "executor.h"
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <err.h>
#include <sysexits.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

#define OPEN_FLAGS (O_WRONLY | O_TRUNC | O_CREAT)
#define DEF_MODE 0664
/*static void print_tree(struct tree *t);*/

void child_redirection(int *fds){
	
		if((dup2(fds[0], STDIN_FILENO)) < 0 ){
			perror("Failed");
			exit(EX_OSERR);
		}
		
		if((dup2(fds[1], STDOUT_FILENO)) < 0 ){
			perror("Failed");
			exit(EX_OSERR);
		}
}

int execute_none_linux(struct tree *t, int *fds){

	int result;
	int status;

	if((result = fork()) < 0){
		perror("Failed to create process");
		exit(EX_OSERR);
	}

	if(result > 0){

		wait(&status);
		return WEXITSTATUS(status);

	}else{
		
		child_redirection(fds);
		execvp(t->argv[0],t->argv);
		fprintf(stderr, "Failed to execute %s\n", t->argv[0]);
		fflush(stdout);
		exit(EX_OSERR);
	}	

}

int execute_none_shell(struct tree *t, int *fds){
	int result = 0;

	if(strcmp(t->argv[0],"exit") == 0){
		exit(0);

	}else if(strcmp(t->argv[0],"cd") == 0){

		if(t->argv[1] == NULL){
			getenv("HOME");

		}else{

			if(chdir(t->argv[1]) < 0){
				perror(t->argv[1]);
			}
		}	
	
	}else{
					
		result = execute_none_linux(t, fds);
		return result;
	}

	return result;
	
}

void translate_fds(char *input, char *output, int *output_fds){
				
	output_fds[0] = (input == NULL) ? STDIN_FILENO : open(input, O_RDONLY);

	if(output_fds[0] < 0){
		perror("FAILED");
		exit(EX_OSERR);
	}

	output_fds[1] = (output == NULL) ? STDOUT_FILENO : open(output, OPEN_FLAGS, DEF_MODE );

	if(output_fds[1] < 0){
		perror("FAILED");
		exit(EX_OSERR);
	}

}

int execute_aux(struct tree *t, int pinput_fd, int poutput_fd){
	int result = 0;
	int fds[2];

	if(t->conjunction == SUBSHELL){
		int status, process_id;
		
		translate_fds(t->input, t->output, fds);

		fds[0] = (fds[0] == 0) ? pinput_fd : fds[0];
		fds[1] = (fds[1] == 1) ? poutput_fd : fds[1];

		if((process_id = fork()) < 0){
			perror("FAILED TO FORK");
			exit(EX_OSERR);
		} 

		if(process_id > 0){
			wait(&status);
			return WEXITSTATUS(status);
		
		}else{	
			execute_aux(t->left,fds[0],fds[1]);
			exit(0);
		
		}	
	
	}

	if(t->conjunction == PIPE){
		int status, process_id;
		int pipe_fds[2];

		if(t->left->output){
			printf("Ambiguous output redirect.\n");
			return 71;
		}
		if(t->right->input){
			printf("Ambiguous input redirect.\n");
			return 71;
		}

		if(pipe(pipe_fds) < 0){
			perror("FAILED");
			exit(EX_OSERR);
		}

		translate_fds(t->input, t->output, fds);

		fds[0] = (fds[0] == 0) ? pinput_fd : fds[0];
		fds[1] = (fds[1] == 1) ? poutput_fd : fds[1];

		if((process_id = fork()) < 0){
			perror("FAILED TO FORK");
			exit(EX_OSERR);
		}

		if(process_id > 0){

			close(pipe_fds[1]);
			result = execute_aux(t->right,pipe_fds[0],fds[1]);
			wait(&status);
			return WEXITSTATUS(status);
		
		}else{

			close(pipe_fds[0]);
			execute_aux(t->left,fds[0],pipe_fds[1]);
			exit(0);
		
		}

	}

	if(t->conjunction == NONE){

			translate_fds(t->input, t->output, fds);

			fds[0] = (fds[0] == 0) ? pinput_fd : fds[0];
			fds[1] = (fds[1] == 1) ? poutput_fd : fds[1];

			result = execute_none_shell(t,fds);
				
			if(fds[0] != 0 && (close(fds[0]) < 0)){
				perror("FAILED");
				exit(EX_OSERR);
			}
			
			if(fds[1] != 1 && (close(fds[1]) < 0)){
				perror("FAILED");
				exit(EX_OSERR);
			}

			if(result != 0){
				return result;
			}

		}

	if(t->conjunction == AND){

		result = execute_aux(t->left,pinput_fd, poutput_fd);

		if(!result){
			
			result = execute_aux(t->right,pinput_fd, poutput_fd);

			if(result != 0){
				return result;
			}
		}	
	}

	
	return result;
		
}


int execute(struct tree *t) {
		int result = 0;

		result = execute_aux(t,0,1);


  	return result;
}

/*static void print_tree(struct tree *t) {
   if (t != NULL) {
      print_tree(t->left);

      if (t->conjunction == NONE) {
         printf("NONE: %s, ", t->argv[0]);
      } else {
         printf("%s, ", conj[t->conjunction]);
      }
      printf("IR: %s, ", t->input);
      printf("OR: %s\n", t->output);

      print_tree(t->right);
   }
}*/

