#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>

#define BUF_SZ 1000

struct Task {
	int pid;
	char task_name[100];
	char state[50];
	char user[50];
	int task;
};

int pid;
char task_name[100];
char state[50];
char user[50];
int task;

int tot_tasks;

int is_reg_file(const char *path) {
	struct stat path_stat;
	stat(path, &path_stat);

	return S_ISREG(path_stat.st_mode);
}

void task_info_helper(char *filename) {
	int file_d = open(filename, O_RDONLY);
	char buff[BUF_SZ];

	if (file_d == -1) {
		printf("There was a problem opening file %s.\n", filename);
	} else {
		if (strstr(filename, "comm")) {
			read(file_d, buff, 2000);
			char *char_set = "\n";
			int len = strcspn(buff, char_set);
			for (int i = 0; i < len; i++) {
				int tn_len = strlen(task_name);
				task_name[tn_len] = buff[i];
				task_name[tn_len + 1] = '\0';
			}
		} else if (strstr(filename, "status")) {
			bool terminate = false;
			read(file_d, buff, BUF_SZ);
			char temp[50];
			for (int i = 0; i < strlen(buff); i++) {
				if (buff[i] != '\n') {
					int len = strlen(temp);
					temp[len] = buff[i];
					temp[len + 1] = '\0';
				} else {
					if (strlen(state) == 0 || pid == 0) {
						if (strstr(temp, "State")) {
							char *char_set = "State:";
							int start = strspn(temp, char_set);
							int end = strcspn(temp, "\n");
							for (int j = start; j < end; j++) {
								if( (temp[j] >= 'a' && temp[j] <= 'z')) {
									int s_len = strlen(state);
									state[s_len] = temp[j];
									state[s_len + 1] = '\0';
								}	
							}
						} else if (strstr(temp, "Pid")) {
							char *char_set = "Pid:";
							// int start = strspn(temp, char_set);
							// int end = strcspn(temp, "\n");
							for (int j = 0; j < )
							printf("%d    %d\n", start, end);
						}
					}
					strcpy(temp, "");
				}
			}
			// }
			// printf("%d\n", file_d);
			// int i = 0;
			// int temp = 0;
			// while (i < 200) {
			// 	char *end = "\n";
			// 	int endline = strcspn(buff, end);

			// 	for (int j = temp; j < endline; i++) {

			// 	}
			// 	temp += endline;
			// 	printf("%d\n", endline);
			// 	i++;
			// }
			// char *start = "m";
			// int startline = strspn(buff, start);
			// printf("%d\n", startline);
			// for (int i = startline; i < endline + startline; i++) {
			// 	printf("%c\n", buff[i]);
			// }
		}
	}
	// printf("%s\n", state);
}

void task_info(char *filename, char *name) {
	DIR *dir = opendir(filename);
	struct dirent *sub_dir;
	// char format[80];
	// sprintf(format, "/proc/%s", name);
	// printf("This is acutal file name %s\n", filename);
	// printf("This is the format %s\n",format);


	if (dir != NULL) {
		while ((sub_dir = readdir(dir)) != NULL) {
			char *new_name = sub_dir->d_name;
			char *temp = "/";
			char *new_dir = (char *) malloc(1 + strlen(filename) + strlen(new_name));
			char *a = (char *) malloc(1 + strlen(filename) + strlen(temp));
			strcpy(a, filename);
			strcat(a, temp);
			strcpy(new_dir, a);
			strcat(new_dir, new_name);

			if (strcmp(sub_dir->d_name, "comm") == 0) {
				task_info_helper(new_dir);
				// strcpy(format.task_name, task_name);
				// pid = 5;
				// strcpy(state, "sleeping");
				// puts(format.task_name);
				// printf("%d\n", format->pid);
			} else if (strcmp(sub_dir->d_name, "status") == 0) {
				task_info_helper(new_dir);
			} else if (strcmp(sub_dir->d_name, ".") == 0 || strcmp(sub_dir->d_name, "..") == 0) {

			}
			// } else {
			// 	if (is_file(new_dir) == 0) {
			// 		task_info(new_dir, sub_dir->d_name);
			// 	}
			// }
		}
	}
	closedir(dir);
}

void task_traverse(char *filename) {
	DIR *dir = opendir(filename);
	struct dirent *sub_dir;
	int count = 0;
	char format[80];
 
	if (dir != NULL) {
		while ((sub_dir = readdir(dir)) != NULL) {
			char *new_name = sub_dir->d_name;
			char *temp = "/";
			char *new_dir = (char *) malloc(1 + strlen(filename) + strlen(new_name));
			char *a = (char *) malloc(1 + strlen(filename) + strlen(temp));
			strcpy(a, filename);
			strcat(a, temp);
			strcpy(new_dir, a);
			strcat(new_dir, new_name);

			bool is_int = false;
			char *temp_str;
			temp_str = sub_dir->d_name;


			for (int i = 0; i < strlen(temp_str); i++) {
				char c = temp_str[i];
				int x = c - '\0';
				if (isdigit(x)) {
					is_int = true;
				} else {
					is_int = false;
					break;
				}
			}
			if (is_int == true) {
				count++;
				sprintf(format, "task%d", count);
				struct Task format; /*= malloc (sizeof (struct Task))*/
				task_info(new_dir, sub_dir->d_name);
				break;
			}
		}
	}
	tot_tasks = count;
	closedir(dir);
}

void print_task(char *name) {
	task_traverse(name);
}