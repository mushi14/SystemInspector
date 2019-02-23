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
#include <pwd.h>

#define BUF_SZ 130000

// Declaring the struct for later use
struct Task {
	char pid[1000];
	char task_name[1000];
	char state[500];
	char user[500];
	char task[500];
};

// Variables for information that is going to be printed later
char proc_name[100];
char pid[1000];
char task_name[1000];
char state[500];
char user[500];
char task[500];

int tot_tasks;
char interrups[100];
char switches[100];
char forks[100];

// Function that checks whether the given filename is directory or file
int is_reg_file(const char *path) {
	struct stat path_stat;
	stat(path, &path_stat);

	return S_ISREG(path_stat.st_mode);
}

// Function for combining digits given a string and starting/ending points
void combining_digits(char *temp, int start, int end, char* var) {
	bool seen = false;

	// Iterates over the string
	for (int j = start; j < end; j++) {
		// Checks if character is a digit, if it is adds it to variable var
		if (isdigit(temp[j]) && seen == false) {
			int len = strlen(var);
			var[len] = temp[j];
			var[len + 1] = '\0';
			int holder = j + 1;
			if (holder != end) {
				for (int k = holder; k < end; k++) {
					if (isdigit(temp[k])) {
						int len = strlen(var);
						var[len] = temp[k];
						var[len + 1] = '\0';
					} else {
						seen = true;
						break;
					}
				}
			}
			break;
		}
	}
}

// Finds all the information
void task_info_helper(char *filename) {
	int file_d = open(filename, O_RDONLY);
	char buff[BUF_SZ];

	memset(buff, 0, BUF_SZ);
	if (file_d == -1) {
		printf("There was a problem opening file %s.\n", filename);
	} else {
		// Checks to see if the file name is 'comm'
		if (strstr(filename, "comm")) {
			read(file_d, buff, 10000);
			char *char_set = "\n";
			int len = strcspn(buff, char_set);
			int tracker = 0;

			// Reads the file for the task name
			for (int i = 0; i < len; i++) {
				tracker++;
				if (tracker <= 25) {
					int tn_len = strlen(task_name);
					task_name[tn_len] = buff[i];
					task_name[tn_len + 1] = '\0';
				} else {
					break;
				}
			}
		// Checks to see if the file name is 'status'
		} else if (strstr(filename, "status")) {
			read(file_d, buff, BUF_SZ);
			char temp[25000];
			memset(temp, 0, 25000);
			int tracker = 0;
			bool is_string = false;

			// Reads the file line by line
			for (int i = 0; i < strlen(buff); i++) {
				if (buff[i] != '\n') {
					int len = strlen(temp);
					temp[len] = buff[i];
					temp[len + 1] = '\0';
				} else {
					// Conditional statement to check if everything is taken care of
					if (strlen(state) == 0 || strlen(pid) == 0 || strlen(user) == 0 || strlen(task) == 0) {
						// Checks to see if the line contains 'state'
						if (strstr(temp, "State")) {
							char *char_set = "State:";
							int start = strspn(temp, char_set);
							int end = strcspn(temp, "\n");

							// Parses the line to find the state of the task
							for (int j = start; j < end; j++) {
								if((temp[j] >= 'a' && temp[j] <= 'z')) {
									int s_len = strlen(state);
									state[s_len] = temp[j];
									state[s_len + 1] = '\0';
									is_string = true;
								} else {
									if (is_string == true && temp[j] == ' ') {
										int s_len = strlen(state);
										state[s_len] = temp[j];
										state[s_len + 1] = '\0';
									}
								}
							}
						// Checks to see if the line contains 'pid'
						} else if (strstr(temp, "Pid")) {
							if (temp[0] == 'P' && temp[1] == 'i' && temp[2] == 'd') {
								char *char_set = "Pid:";
								int start = strspn(temp, char_set);
								int end = strcspn(temp, "\n");

								// Parses the line to find the pid of the task
								for (int j = start; j < end; j++) {
									if (isdigit(temp[j])) {
										int p_len = strlen(pid);
										pid[p_len] = temp[j];
										pid[p_len + 1] = '\0';
									}
								}
							}
						// Checks to see if the line contains 'user id'
						} else if (strstr(temp, "Uid")) {
							tracker++;
							if (tracker <= 15) {
								char *char_set = "Uid:";
								int start = strspn(temp, char_set);
								int end = strcspn(temp, "\n");
								// Parses the line to find the uid of the task
								combining_digits(temp, start, end, user);
							}
						// Checks to see if the line contains the tasks
						} else if (strstr(temp, "Threads")) {
							char *char_set = "Threads:";
							int start = strspn(temp, char_set);
							int end = strcspn(temp, "\n");

							// Parses the line to find the running tasks of the given task
							for (int j = start; j < end; j++) {
								if (isdigit(temp[j])) {
									int t_len = strlen(task);
									task[t_len] = temp[j];
									task[t_len + 1] = '\0';
								}
							}
						}
					}
					strcpy(temp, "");
				}
			}
		}
	}
	close(file_d);
}

// Finds the necessary files needed for populating the struct
void task_info(char *filename) {
	DIR *dir = opendir(filename);
	struct dirent *sub_dir;

	if (dir != NULL) {
		while ((sub_dir = readdir(dir)) != NULL) {
			// Creating a variable 'new_dir' that has the exact path of the current working directory
			char *new_name = sub_dir->d_name;
			char *temp = "/";
			char *new_dir = (char *) malloc(100000 + strlen(filename) + strlen(new_name));
			char *a = (char *) malloc(100000 + strlen(filename) + strlen(temp));
			strcpy(a, filename);
			strcat(a, temp);
			strcpy(new_dir, a);
			strcat(new_dir, new_name);

			if (strcmp(sub_dir->d_name, "comm") == 0) {
				task_info_helper(new_dir);
			} else if (strcmp(sub_dir->d_name, "status") == 0) {
				task_info_helper(new_dir);
			} else if (strcmp(sub_dir->d_name, ".") == 0 || strcmp(sub_dir->d_name, "..") == 0) {

			}

			// Freeing memory
			free(new_dir);
			free(a);
		}
	}
	closedir(dir);
}

// Function that populates the task struct
void task_processor(char *filename) {
	DIR *dir = opendir(filename);
	struct dirent *sub_dir;
	int count = 0;
	char format[4000];
 
	if (dir != NULL) {
		while ((sub_dir = readdir(dir)) != NULL) {
			// Creating a variable 'new_dir' that has the exact path of the current working directory
			char *new_name = sub_dir->d_name;
			char *temp = "/";
			char *new_dir = (char *) malloc(100000 + strlen(filename) + strlen(new_name));
			char *a = (char *) malloc(100000 + strlen(filename) + strlen(temp));
			strcpy(a, filename);
			strcat(a, temp);
			strcpy(new_dir, a);
			strcat(new_dir, new_name);

			bool is_int = false;
			char *temp_str;
			temp_str = sub_dir->d_name;

			// Checks to see if the directory is a task
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
				// To make the task names more reasonable
				sprintf(format, "task%d", count);
				if (is_reg_file(new_dir) == 0) {
					// Creating the task and populating it with the information that was found
					task_info(new_dir);
					struct Task format;
					strcpy(format.pid, pid);
					strcpy(format.state, state);
					strcpy(format.task_name, task_name);
					strcpy(format.user, user);
					strcpy(format.task, task);

					// Printing the info
					printf("%5s | %12s | %25s | %15s | %5s \n", format.pid, format.state, format.task_name, format.user, format.task);

					// Resetting the variables so the next task can use them
					memset(pid, 0, 1000);
					memset(task_name, 0, 1000);
					memset(state,0 ,500);
					memset(user, 0, 500);
					memset(task, 0, 500);				
				}
			}

			// Freeing memory
			free(new_dir);
			free(a);
		}
	}
	closedir(dir);
}

// Function for finding the total number of tasks running
void get_count(char *filename) {
	DIR *dir = opendir(filename);
	struct dirent *sub_dir;
	int count = 0;
 
	if (dir != NULL) {
		while ((sub_dir = readdir(dir)) != NULL) {
			// Creating a variable 'new_dir' that has the exact path of the current working directory
			char *new_name = sub_dir->d_name;
			char *temp = "/";
			char *new_dir = (char *) malloc(100000 + strlen(filename) + strlen(new_name));
			char *a = (char *) malloc(100000 + strlen(filename) + strlen(temp));
			strcpy(a, filename);
			strcat(a, temp);
			strcpy(new_dir, a);
			strcat(new_dir, new_name);

			bool is_int = false;
			char *temp_str;
			temp_str = sub_dir->d_name;

			// Checks to see if the directory is name with all numbers
			// if it is, then increments count
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
				if (is_reg_file(new_dir) == 0) {
					count++;
				}
			}

			// Freeing memory
			free(new_dir);
			free(a);
		}
	}
	tot_tasks = count;
	closedir(dir);
}

// Function for finding the interrupts, switches, and forks
void gather_info_helper(char *filename) {
	int file_d = open(filename, O_RDONLY);
	char buff[BUF_SZ];

	memset(buff, 0, BUF_SZ);
	if (file_d == -1) {
		printf("There was a problem opening file %s.\n", filename);
	} else {
		read(file_d, buff, BUF_SZ);
		char temp[80000];
		strcpy(temp, "");

		// Reads the file line by line
		for (int i = 0; i < strlen(buff); i++) {
			if (buff[i] != '\n') {
				int len = strlen(temp);
				temp[len] = buff[i];
				temp[len + 1] = '\0';
			} else {
				// Checks to see if the line contains interrupts
				if (strstr(temp, "intr")) {
					char *char_set = "intr";
					int start = strspn(temp, char_set);
					int end = strcspn(temp, "\n");
					// Parses the line to find interrupts
					combining_digits(temp, start, end, interrups);
				// Checks to see if the line contains switches
				} else if (strstr(temp, "ctxt")) {
					char *char_set = "ctxt";
					int start = strspn(temp, char_set);
					int end = strcspn(temp, "\n");
					// Parses the line to find switches
					combining_digits(temp, start, end, switches);
				// Checks to see if the line contains forks
				} else if (strstr(temp, "processes")) {
					char *char_set = "processes";
					int start = strspn(temp, char_set);
					int end = strcspn(temp, "\n");
					// Parses the line to find forks
					combining_digits(temp, start, end, forks);
				}
				strcpy(temp, "");
			}
		}
	}
	close(file_d);
}

// Function for finding th correct file that contains 'stat'
void gather_info(char *filename) {
	DIR *dir = opendir(filename);
	struct dirent *sub_dir;

	if (dir != NULL) {
		// Going through given proc and looking for 'stat'
		while ((sub_dir = readdir(dir)) != NULL) {
			// Creating a variable 'new_dir' that has the exact path of the current working directory
			char *new_name = sub_dir->d_name;
			char *temp = "/";
			char *new_dir = (char *) malloc(100000 + strlen(filename) + strlen(new_name));
			char *a = (char *) malloc(100000 + strlen(filename) + strlen(temp));
			strcpy(a, filename);
			strcat(a, temp);
			strcpy(new_dir, a);
			strcat(new_dir, new_name);

			if (is_reg_file(new_dir) != 0) {
				if (strcmp(proc_name, filename) == 0 && strcmp(sub_dir->d_name, "stat") == 0) {
					gather_info_helper(new_dir);
				}
			}

			// Freeing memory
			free(new_dir);
			free(a);
		}
	}
	closedir(dir);
}

// Prints the task information
void print_task(char *name) {
	memset(proc_name, 0, 100);
	strcpy(proc_name, name);
	get_count(name);
	gather_info(name);

	printf("Task Information\n");
	printf("----------------\n");
	printf("Tasks running: %d\n", tot_tasks);
	printf("Since boot:\n");
	printf("	Interrupts: %s\n", interrups);
	printf("	Context Switches: %s\n", switches);
	printf("	Forks: %s\n", forks);
}

// Prints the list of all tasks and their information
void print_list(char *name) {
	printf("%5s | %12s | %25s | %15s | %5s \n",
	"PID", "State", "Task Name", "User", "Tasks");
	printf("------+--------------+---------------------------+-----------------+-------\n");
	task_processor(name);
}