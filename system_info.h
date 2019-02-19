#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

char hostname[100];
char version[100];
char uptime[100];
int hours;
int minutes;
int seconds;

int is_regular_file(const char *path) {
	struct stat path_stat;
	stat(path, &path_stat);

	return S_ISREG(path_stat.st_mode);
}

void convert(char *val) {

	int total_seconds = 0;
	total_seconds = atoi(val);
	hours = total_seconds / 3600;
	minutes = (total_seconds - (3600 * hours)) / 60;
	seconds = (total_seconds - (3600 * hours) - (minutes * 60));
}

void set_uptime(char *filename) {
	int file_d = open(filename, O_RDONLY);
	char *buff = (char *) calloc(100, sizeof(char));
	char c[255];

	if (file_d == -1) {
		printf("There was a problem opening file %s.\n", filename);
	} else {
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

char *file_read(char *filename) {
	int file_d = open(filename, O_RDONLY);
	char *buff = (char *) calloc(100, sizeof(char));

	if (file_d == -1) {
		printf("There was a problem opening file %s.\n", filename);
	} else {
		read(file_d, buff, 100);
	}
	close(file_d);

	return buff;
}

void sys_traverse(char *name) {
	DIR *dir = opendir(name);
	struct dirent *sub_dir;

	if (dir != NULL) {
		if (strlen(hostname) == 0 || strlen(version) == 0 || strlen(uptime) == 0) {

			while ((sub_dir = readdir(dir)) != NULL) {
				char *new_name = sub_dir->d_name;
				char *temp = "/";
				char *new_dir = (char *) malloc(1 + strlen(name) + strlen(new_name));
				char *a = (char *) malloc(1 + strlen(name) + strlen(temp));
				strcpy(a, name);
				strcat(a, temp);
				strcpy(new_dir, a);
				strcat(new_dir, new_name);

				if (strcmp(sub_dir->d_name, "hostname") == 0) {
					strcpy(hostname, file_read(new_dir));
				} else if (strcmp(sub_dir->d_name, "osrelease") == 0) {
					strcpy(version, file_read(new_dir));
				} else if (strcmp(sub_dir->d_name, "uptime") == 0) {
					set_uptime(new_dir);
					strcpy(uptime,"done");
				} else if (strcmp(sub_dir->d_name, ".") == 0 || strcmp(sub_dir->d_name, "..") == 0) {

				} else {
					if (is_regular_file(new_dir) == 0) {
						sys_traverse(new_dir);
					}
				}
			}
		 	closedir(dir);
		}
	}
}

void print_sys(char *name) {

	sys_traverse(name);
	printf("System Information\n");
	printf("------------------\n");
	printf("Hostname: %s", hostname);
	printf("Kernel Version: %s", version);
	printf("Uptime: %d hours, %d minutes, %d seconds", hours, minutes, seconds);
	printf("\n");
}