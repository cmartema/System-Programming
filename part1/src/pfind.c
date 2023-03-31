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

int is_matching_perm(struct stat *sb, char *perm_str) {
	int perms[] = {S_IRUSR, S_IWUSR, S_IXUSR,
		S_IRGRP, S_IWGRP, S_IXGRP,
		S_IROTH, S_IWOTH, S_IXOTH};

	char *file_perm_string;
	if ((file_perm_string = malloc(10 * sizeof(char))) == NULL) {
	fprintf(stderr, "Error: malloc failed. %s.\n",
		strerror(errno));
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < 9; i += 3) {
		// Using the ternary operator for succinct code.
		file_perm_string[i] = sb->st_mode & perms[i] ? 'r' : '-';
		file_perm_string[i + 1] = sb->st_mode & perms[i + 1] ? 'w' : '-';
		file_perm_string[i + 2] = sb->st_mode & perms[i + 2] ? 'x' : '-';
	}

	file_perm_string[9] = '\0';
	if (strcmp(file_perm_string, perm_str) == 0) {
		free(file_perm_string);
		return 0;
	}

	free(file_perm_string);
	return -1;
}

int directory_rec( char *fullpath, char *perm_str) {
	char path[PATH_MAX];
	DIR *dir;
	if (realpath(fullpath, path) == NULL) {
		fprintf(stderr, "Error: Cannot get full path of file '%s'. %s.\n",
				fullpath, strerror(errno));
		return EXIT_FAILURE;	
	}
	if ((dir = opendir(path)) == NULL) {
		fprintf(stderr, "Error: Cannot open directory '%s'. %s.\n",
				path, strerror(errno));
		return EXIT_FAILURE;	
	}

	// All paths typically end in the name of the file, as in
	// /home/user.
	// Add 1 to PATH_MAX so we have room for a trailing '/' after the true
	// path name.
	char full_filename[PATH_MAX + 1];
	// Set the initial character to the NULL byte.
	// If the path is root '/', full_filename will be an empty string, and
	// we will need to take its strlen, so make sure it's null-terminated.
	full_filename[0] = '\0';
	if (strcmp(path, "/")) {
		// This code executes when the path is not the root '/'.
		// If there is no NULL byte in path, the full_filename will not be
		// terminated. So, take the minimum of the path length and PATH_MAX, and
		// copy the smaller amount of character into full_filename.
		size_t copy_len = strnlen(path, PATH_MAX);
		memcpy(full_filename, path, copy_len);
		full_filename[copy_len] = '\0';
	}

	// Add + 1 for the trailing '/' that we're going to add.
	size_t pathlen = strlen(full_filename) + 1;
	full_filename[pathlen - 1] = '/';
	full_filename[pathlen] = '\0';
	struct dirent *entry;
	struct stat sb;
	while ((entry = readdir(dir)) != NULL) {
		// Skip . and ..
		if (strcmp(entry->d_name, ".") == 0 ||
				strcmp(entry->d_name, "..") == 0) {
			continue;
		}
		// Add the current entry's name to the end of full_filename, following
		// the trailing '/' and overwriting the '\0'.
		strncpy(full_filename + pathlen, entry->d_name, PATH_MAX - pathlen);
		if (lstat(full_filename, &sb) < 0) {
			fprintf(stderr, "Error: Cannot stat file '%s'. %s.\n",
					full_filename, strerror(errno));
			continue;
		}

		if (is_matching_perm(&sb, perm_str)) { 
			printf("%s\n", full_filename);
		}

		if (S_ISDIR(sb.st_mode)) {
			directory_rec(full_filename, perm_str);
		}

	}

	closedir(dir);
	return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
	int d_flag = 0, p_flag = 0, h_flag = 0, opt = 0;
	opterr = 0; // suppresses the getopts error messages.
	while ((opt = getopt(argc, argv, "d:p:h")) != -1) {
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

	struct stat statbuf;
	if (stat(argv[2], &statbuf) < 0) {
		fprintf(stderr, "Error: Cannot stat '%s'. No such file or directory.\n", argv[2]);	
		return EXIT_FAILURE;
	}

	if (!S_ISDIR(statbuf.st_mode)) {
		fprintf(stderr, "Error: Cannot stat '%s'. No such file or directory.\n", argv[2]);
		return EXIT_FAILURE;
	} else if (!((statbuf.st_mode & S_IRUSR) && (statbuf.st_mode & S_IWUSR))) {
		fprintf(stderr, "Error: Cannot open directory '%s'. Permission denied.\n", argv[2]);
		return EXIT_FAILURE;
	}else if (valid_perm_str(argv[4]) != 0) {
		fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", argv[4]);
		return EXIT_FAILURE;
	}else {
		directory_rec(argv[2], argv[4]);
	}


}
