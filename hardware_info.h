#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

char model[100];
int units;
char load_avg[100];
float c_usage;
float mem_total;
float mem_used;
bool memory;

int is_file(const char *path) {
	struct stat path_stat;
	stat(path, &path_stat);

	return S_ISREG(path_stat.st_mode);
}

void cpu_info(char *filename) {
	units = 1;
	int file_d = open(filename, O_RDONLY);
	char *buff = (char *) calloc(2000, sizeof(char));
	char *token;

	if (file_d == -1) {
		printf("There was a problem opening file %s.\n", filename);
	} else {
		read(file_d, buff, 2000);
		while ((token = strsep(&buff, "\n")) != NULL) {
			if (strstr(token, "model name") || strstr(token, "processor")) {
				char *temp_token;
				if (strstr(token, "model name")) {
					while ((temp_token = strsep(&token, ":")) != NULL) {
						strcpy(model, temp_token);
					}
				} else {

					while ((temp_token = strsep(&token, ":")) != NULL) {
						int numb = atoi(temp_token);
						units += numb;
					}
				}
			}
		}
	}
	close(file_d);
}

void load_average(char *filename) {
	int file_d = open(filename, O_RDONLY);
	char *buff = (char *) calloc(2000, sizeof(char));
	char *token;
	int count = 0;
	char *space = " ";

	if (file_d == -1) {
		printf("There was a problem opening file %s.\n", filename);
	} else {
		read(file_d, buff, 2000);
		while ((token = strsep(&buff, " ")) != NULL) {
			if (count < 3) {
				strcat(load_avg, token);
				if (count < 2) {
					strcat(load_avg, space);
				}
				count++;
			} else {
				break;
			}
		}
	}
	close(file_d);
}

int *cpu_stat_helper(char *token, char *buff) {
	int total = 0;
	int idle = 0;
	static int temp[2];
	while ((token = strsep(&buff, "\n")) != NULL) {
		if (strstr(token, "cpu")) {
			char *temp_token;
			int count = 0;

			while ((temp_token = strsep(&token, "cpu ")) != NULL) {
				if (strcmp(temp_token, "") != 0) {
					printf("temp token %s\n", temp_token);
					count++;
					total += atoi(temp_token);
					if (count == 4) {
						idle = atoi(temp_token);
					}
				}
			}
			printf("Just for this one total: %d\n", total);
			printf("Just for this one idle: %d\n", idle);
			printf("\n");
			break;
		}
	}

	temp[0] = total;
	temp[1] = idle;
	return temp;
}

void cpu_stat(char *filename) {
	if (strcmp(filename, "/proc/stat") == 0) {
		char *buff = (char *) calloc(2000, sizeof(char));
		c_usage = 1;
		int total = 0;
		int idle = 0;
		int *p;

		for (int i = 0; i < 2; i++) {
			int file_d = open(filename, O_RDONLY);
			if (file_d == -1) {
				printf("There was a problem opening file %s.\n", filename);
			} else {
				read(file_d, buff, 2000);
				char *token;
				if (i == 1) {
					sleep(1);
					p = cpu_stat_helper(token, buff);
				} else {
					p = cpu_stat_helper(token, buff);
				}
				total += p[0];
				idle += p[1];
			}
			close(file_d);
		}
		printf("\n");
		printf("\n");
		printf("Total altogether: %d\n", total);
		printf("Idle altogether: %d\n", idle);
	}
}

int hashes(float average) {
	int count = (int) average / 5;
	
	return count;
}

float kb_to_gb(int numb) {
	float convert;
	convert = (((float) numb / 1000000) * 100);

	return convert;
}

void mem_info(char *filename) {
	int file_d = open(filename, O_RDONLY);
	char *buff = (char *) calloc(2000, sizeof(char));
	char *token;
	int total;
	int used;
	bool seen = false;

	if (file_d == -1) {
		printf("There was a problem opening file %s.\n", filename);
	} else {
		read(file_d, buff, 2000);
		while ((token = strsep(&buff, "\n")) != NULL) {
			if (strstr(token, "MemTotal") || strstr(token, "Active")) {
				char *temp_token;
				if (strstr(token, "MemTotal")) {
					while ((temp_token = strsep(&token, "MemTotal:")) != NULL) {
						if (strcmp(temp_token, "") != 0) {
							total = atoi(temp_token);
						}
					}
				} else {
					if (seen == false) {
						while ((temp_token = strsep(&token, "Active:")) != NULL) {
							if (strcmp(temp_token, "") != 0) {
								used = atoi(temp_token);
								seen = true;
							}
						}
					} else {
						break;
					}
				}
			}
		}
	}
	close(file_d);

	mem_total = kb_to_gb(total);
	mem_used = kb_to_gb(used);
	memory = true;
}

void hw_traverse(char *name) {
	DIR *dir = opendir(name);
	struct dirent *sub_dir;

	if (dir != NULL) {
		if (strlen(model) == 0 || units == 0 || strlen(load_avg) == 0 /*|| c_usage == 0 */|| memory == false) {
			while ((sub_dir = readdir(dir)) != NULL) {
				char *new_name = sub_dir->d_name;
				char *temp = "/";
				char *new_dir = (char *) malloc(1 + strlen(name) + strlen(new_name));
				char *a = (char *) malloc(1 + strlen(name) + strlen(temp));
				strcpy(a, name);
				strcat(a, temp);
				strcpy(new_dir, a);
				strcat(new_dir, new_name);

				if (strcmp(sub_dir->d_name, "cpuinfo") == 0) {
					cpu_info(new_dir);
				// } else if (strcmp(sub_dir->d_name, "stat") == 0) {
					// cpu_stat(new_dir);
					// printf("%d\n", wit);
				} else if (strcmp(sub_dir->d_name, "meminfo") == 0) {
					mem_info(new_dir);
				} else if (strcmp(sub_dir->d_name, "loadavg") == 0) {
					load_average(new_dir);
				} else if (strcmp(sub_dir->d_name, ".") == 0 || strcmp(sub_dir->d_name, "..") == 0) {

				} else {
					if (is_file(new_dir) == 0) {
						hw_traverse(new_dir);
					}
				}
			}
		}
	}
	closedir(dir);
}

void print_hardware(char *name) {

	int count = 0;
	hw_traverse(name);
	printf("Hardware Information\n");
	printf("--------------------\n");
	printf("CPU Model: %s\n", model);
	printf("Processing Units: %d\n", units);
	printf("Load Average (1/5/15 min): %s\n", load_avg);
	// printf("CPU Usage:    [##########----------] %s\n", c_usage);

	printf("Memory Usage: [");
	count = hashes(mem_used);
	for (int i = 0; i < 100; i += 5) {
		if (count != 0) {
			printf("#");
			count --;
		} else {
			printf("-");
		}
	}
	printf("] %.1f%% ", mem_used);
	mem_used = mem_used / 100;
	mem_total = mem_total / 100;
	printf("(%.1f GB / %.1f GB)\n", mem_used, mem_total);
}