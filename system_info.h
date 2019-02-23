#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SZ 130000

// Variables for information that is going to be printed later
char hostname[2000];
char version[2000];
bool uptime;
int years;
int days;
int hours;
int minutes;
int seconds;

// Checks to see if the file given is a directory or a regular file
int is_regular_file(const char *path) {
	struct stat path_stat;
	stat(path, &path_stat);

	return S_ISREG(path_stat.st_mode);
}

// Converts the uptime in years, days, hours, and seconds and updates variables accordingly
void convert(char *val) {
	int total_seconds = 0;
	total_seconds = atoi(val);
	years = total_seconds / (365 * 24 * 3600);
	total_seconds = total_seconds % (365 * 24 * 3600);
	days = total_seconds / (24 * 3600);
	total_seconds = total_seconds % (24 * 3600);
	hours = total_seconds / 3600;
	total_seconds %= 3600;
	minutes = total_seconds / 60 ; 
	total_seconds %= 60;
	seconds = total_seconds;
}

// Finds the uptime given uptime file
void set_uptime(char *filename) {
	int file_d = open(filename, O_RDONLY);
	char buff[BUF_SZ];
	char c[255];

	memset(buff, 0, BUF_SZ);
	if (file_d == -1) {
		printf("There was a problem opening file %s.\n", filename);
	} else {
		// Parsing the file for uptime info
		read(file_d, buff, 100);
		strcpy(c, buff);
		for (int i = 0; c[i] != '\0'; i++) {
			if (c[i] == ' ') {
				c[i] = '\0';
			}
		}
	}
	close(file_d);
	convert(c);
}

// Reads the file looking for hostname or kernel version
void file_read(char *filename) {
	int file_d = open(filename, O_RDONLY);
	char buff[BUF_SZ];

	memset(buff, 0, BUF_SZ);
	if (file_d == -1) {
		printf("There was a problem opening file %s.\n", filename);
	} else {
		read(file_d, buff, 100);
	}
	close(file_d);

	if (strlen(hostname) == 0) {
		strcpy(hostname, buff);
	} else {
		strcpy(version, buff);
	}
}

// Function for traversing the proc directory 
void sys_traverse(char *name) {
	DIR *dir = opendir(name);
	struct dirent *sub_dir;

	if (dir != NULL) {
		// Conditional to check if everything is taken care of
		if (strlen(hostname) == 0 || strlen(version) == 0 || uptime == false) {
			while ((sub_dir = readdir(dir)) != NULL) {
				// Creating a variable 'new_dir' that has the exact path of the current working directory
				char *temp = "/";
				char *new_dir = (char *) malloc(100000 + strlen(name) + strlen(sub_dir->d_name));
				char *a = (char *) malloc(100000 + strlen(name) + strlen(temp));
				strcpy(a, name);
				strcat(a, temp);
				strcpy(new_dir, a);
				strcat(new_dir, sub_dir->d_name);

				if (strcmp(sub_dir->d_name, "hostname") == 0) {
					file_read(new_dir);
				} else if (strcmp(sub_dir->d_name, "osrelease") == 0) {
					file_read(new_dir);
				} else if (strcmp(sub_dir->d_name, "uptime") == 0) {
					set_uptime(new_dir);
					uptime = true;
				} else if (strcmp(sub_dir->d_name, ".") == 0 || strcmp(sub_dir->d_name, "..") == 0) {

				} else {
					if (is_regular_file(new_dir) == 0) {
						sys_traverse(new_dir);
					}
				}

				// Freeing memory
				free(new_dir);
				free(a);
			}
		}
	}
 	closedir(dir);	
}

// Prints the system information
void print_sys(char *name) {

	sys_traverse(name);
	printf("System Information\n");
	printf("------------------\n");
	printf("Hostname: %s", hostname);
	printf("Kernel Version: %s", version);

	printf("Uptime: ");
	if (years == 0) {
		if (days == 0) {
			if (hours == 0) {
				printf("%d minutes, %d seconds\n", minutes, seconds);
			} else {
				printf("%d hours, %d minutes, %d seconds\n", hours, minutes, seconds);
			} 
		} else {
			if (hours == 0) {
				printf(" %d days, %d minutes, %d seconds\n", days, minutes, seconds);
			} else {
				printf("%d days, %d hours, %d minutes, %d seconds\n", days, hours, minutes, seconds);
			} 
		}
	} else {
		printf("%d years, ", years);

		if (days == 0) {
			if (hours == 0) {
				printf("%d minutes, %d seconds\n", minutes, seconds);
			} else {
				printf("%d hours, %d minutes, %d seconds\n", hours, minutes, seconds);
			} 
		} else {
			if (hours == 0) {
				printf(" %d days, %d minutes, %d seconds\n", days, minutes, seconds);
			} else {
				printf("%d days, %d hours, %d minutes, %d seconds\n", days, hours, minutes, seconds);
			} 
		}
	}
}