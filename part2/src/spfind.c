#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]){
	
	// check if it is valid
	// TODO Make sure this is right and works 
	if(argc < 2 || argc > 5){
		fprintf(stderr, "Usage: %s -d <directory> -p <permissions string> [-h]", argv[0]);
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

		// close all unrelated file descriptors 	
		close(pfind_to_sort[1]); 
		close(sort_to_parent[0]);
		close(sort_to_parent[1]);
	
		// calling the function pfind 
		// TODO - need to make sure it is execv ?
		// make sure it does not fail with the if statment
		if(execv("pfind", argv) == -1){
			fprintf(stderr, "Error: pfind failed.");
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
			fprintf(stderr, "Error: sort failed."); 
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

	char buffer[128];
	ssize_t count = read(STDIN_FILENO, buffer, sizeof(buffer - 1)); 
	if(count == -1){
		perror("read()");
		exit(EXIT_FAILURE);
	}
	buffer[count] = '\0';
	printf("Total matches: %d", atoi(buffer)); 

	// need 2 waits to wait for each child.
	wait(NULL); 
	wait(NULL);

	return EXIT_SUCCESS;
}
