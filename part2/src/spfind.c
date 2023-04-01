#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h> 

int main(int argc, char *argv[]){
	
	// check if it is valid
	// TODO Make sure this is right and works
	if(argc < 2 || argc > 5){
		fprintf(stderr, "Usage: %s -d <directory> -p <permissions string> [-h]\n", argv[0]);
	       return EXIT_FAILURE; 	
	}

	// the exec of pfind should handle all other error messages, permission string reqs, etc.
	// check return code of all system and function calls 
	// if exec-ing pfind or sort fails, indicate that
	
	// create the pipes
	int pfind_to_sort[2], sort_to_parent[2];
	pipe(pfind_to_sort);
	pipe(sort_to_parent); 
	
	//make an array of the pids of the upcoming sorts
	pid_t pid[2];
	
	if((pid[0] = fork()) == 0) {
		// pfind process
		close(pfind_to_sort[0]); 
		dup2(pfind_to_sort[1], STDOUT_FILENO);
		close(pfind_to_sort[1]); 
		// close all unrelated file desciptors 
		close(sort_to_parent[0]);
		close(sort_to_parent[1]);
	
		// calling the function pfind 
		// TODO - need to make sure it is execv ?
		// make sure it does not fail with the if statment
		if(execv("./pfind", argv) == -1){
			fprintf(stderr, "Error: pfind failed.\n");
			return EXIT_FAILURE;	
		}
	}

	if((pid[1] = fork()) == 0){
		// sort process
		close(pfind_to_sort[1]);
		dup2(pfind_to_sort[0], STDIN_FILENO);
		close(pfind_to_sort[0]);
		close(sort_to_parent[0]);
		dup2(sort_to_parent[1], STDOUT_FILENO);
		close(sort_to_parent[1]);

		if(execlp("sort", "sort", NULL) == -1){
			fprintf(stderr, "Error: sort failed.\n"); 
			return EXIT_FAILURE;
		}	
	}

	// back in parent proccess - close everything
	close(sort_to_parent[1]);
	dup2(sort_to_parent[0], STDIN_FILENO);
	close(sort_to_parent[0]);

	// close all unrelated file descriptors 
	close(pfind_to_sort[0]);
	close(pfind_to_sort[1]); 
	
	// read through output and write to stdout 
	char buffer[8192];
	ssize_t bytes_read;
	int ctr = 0; 
        while((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer - 1)) > 0)){
		//buffer[bytes_read] = '\0';	
		//printf("bytes_read = %ld\n", sizeof(bytes_read)); 
		if(write(STDOUT_FILENO, buffer, sizeof(bytes_read)) < 0){
			perror("Write()");
			exit(EXIT_FAILURE);
		}
		
		// printf("Buffer = %s\n", buffer); 
		
		 if(bytes_read == -1){
			 perror("read()");
			 exit(EXIT_FAILURE);
		}
		
		// check for newlines -> +1 match
		char *newline = strchr(buffer, '\n'); 

		if(newline != NULL){
			ctr++; 		
		}

	}

	buffer[bytes_read] = '\0';	
	printf("Total matches: %d\n", ctr);

	// waitpid
	// if status of either child is failure - exit failure 
	int status;
	int i = 0; 
	do{
		pid_t w = waitpid(pid[i], &status, WUNTRACED | WCONTINUED);
		if(w == -1){
			// waitpid failed.
			perror("waitpid()");
			exit(EXIT_FAILURE);
		}

		if(WIFEXITED(status) || WIFSIGNALED(status) || WIFSTOPPED(status) || WIFCONTINUED(status)){
			exit(EXIT_FAILURE); 
		}	

		i++; 

	} while(!WIFEXITED(status) && !WIFSIGNALED(status));

	// check that pfind didn't print the usage - strcmp - exit success
	
	if(buffer[0] == 'U'){
		exit(EXIT_SUCCESS);
	}

	printf("Total matches: %d\n", ctr);

	return EXIT_SUCCESS;
}
