#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>

void display_usage(char *basename, FILE *stream) {
	char* usage = "Usage: %s -d <directory> -p <permissions string> [-h]\n";
//		      "   -d: Searches directory and list files matching specified permissions.\n"
//		      "   -p: Specifies the permissions for files contained in the directory.\n"
//		      "   -h: Displays help.\n";
	fprintf(stream, usage, basename);
}

/**
 * Takes a char*  and checks if the length or its characters form
 * a valid permission string. Returns 0 if the char* is a valid
 * permission string, else -1.
 */
int valid_perm_str( char *perm_str) {	
	int PERM_STR_MAX = 9;
	size_t str_len = strlen(perm_str);
	if (str_len != PERM_STR_MAX) {
		return -1;
	}
	
	for (int i = 0; i < PERM_STR_MAX; i++) {
		if ( i % 3 == 0) {
			if (perm_str[i] != 'r' && perm_str[i] != '-') {
				return -1;
			}
		}else if (i % 3 == 1) {
			if (perm_str[i] != 'w' && perm_str[i] != '-') {
				return -1;
			}
		}else if (i % 3 == 2) {
			if (perm_str[i] != 'x' && perm_str[i] != '-') {
				return -1;
			}
		}
	}
	return 0;
}

int directory_rec( char *path, char *perm_str) {
	DIR *dir;
	struct dirent *entry;
	struct stat statbuf;
	char fullpath[PATH_MAX];
	if ((dir = opendir(path)) == NULL) {
		return -1
	}
	while ((entry = readdir(dir)) != NULL) {
		// Skip . and ..
		if


int main(int argc, char **argv) {
	int d_flag = 0, p_flag = 0, h_flag = 0, opt = 0;
	opterr = 0; // suppresses the getopts error messages.
	while ((opt = getopt(argc, argv, "dph")) != -1) {
		switch (opt) {
			case 'd':
				d_flag = 1;
				break;
			case 'p':
				p_flag = 1;
				break;
			case 'h':
				h_flag = 1;
				break;
			case '?':
				fprintf(stderr, "Error: Unknown option '-%c' received.\n", optopt);
				return EXIT_FAILURE;
			default:
				return EXIT_FAILURE;

		}
	}

	if ((d_flag == 1 && p_flag == 0)) {
		fprintf(stderr, "Error: Required argument -p <permissions string> not found.\n");
		return EXIT_FAILURE;
	}

	if ((d_flag == 0 && p_flag == 1)) {
		fprintf(stderr, "Error: Required argument -d <directory> not found.\n");
		return EXIT_FAILURE;
	}

	if ((h_flag == 1) || ((d_flag + p_flag + h_flag) > 2)) {
		display_usage(argv[0], stdout);
		return EXIT_SUCCESS;
	}

	
	if (valid_perm_str(argv[optind + 1]) != 0) {
		fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", argv[optind + 1]);
		return EXIT_FAILURE;
	}
	
	
	return EXIT_SUCCESS;
}
