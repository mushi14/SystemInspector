#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <float.h>
#include <math.h>

#define BUF_SZ 130000

// Variables for information that is going to be printed later
char proc_name[100];
char model[100];
int units;
char load_avg[100];
float c_usage;
float mem_total;
float mem_used;
bool memory;

// Function that checks whether the given filename is directory or file
int is_file(const char *path) {
	struct stat path_stat;
	stat(path, &path_stat);

	return S_ISREG(path_stat.st_mode);
}

// Function for combining digits given a string and starting/ending points
void digits_combiner(char *temp, int start, int end, char* var) {
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

// Function for finding the cpu info
void cpu_info(char *filename) {
	int file_d = open(filename, O_RDONLY);
	char buff[BUF_SZ];

	memset(buff, 0, BUF_SZ);
	if (file_d == -1) {
		printf("There was a problem opening file %s.\n", filename);
	} else {
		read(file_d, buff, BUF_SZ);
		char temp[80000];
		memset(temp, 0, 80000);
		bool model_seen = false;

		// Goes through the file line by line
		for (int i = 0; i < strlen(buff); i++) {
			if (buff[i] != '\n') {
				int len = strlen(temp);
				temp[len] = buff[i];
				temp[len + 1] = '\0';
			} else {
				// If the given line has 'model name'
				if (strstr(temp, "model name") && model_seen == false) {
					char *char_set = "model name";
					int start = strspn(temp, char_set);
					int end = strcspn(temp, "\n");
					bool go = false;
					int counter = 0;

					// Parses the line for the model name 
					for (int j = start; j < end; j++) {
						if (go == true && counter > 1) {
							int m_len = strlen(model);
							model[m_len] = temp[j];
							model[m_len + 1] = '\0';
						} else {
							if (temp[j] == ':') {
								go = true;
							} else {
								counter++;
							}
						}	
					}
					model_seen = true;

				// If the given line has 'processor'
				} else if (strstr(temp, "processor")) {
					char *char_set = "processor";
					int start = strspn(temp, char_set);
					int end = strcspn(temp, "\n");
					bool stop = false;

					// Parses the line for the processor name
					for (int j = start; j < end; j++) {
						if (isdigit(temp[j])) {
							char temp_unit[50];
							int c = strlen(temp_unit);
							temp_unit[c] = temp[j];
							temp_unit[c + 1] = '\0';
							if (stop == false) {
								units += 1;
							}
							stop = true;
						} else {
							stop = false;
						}
					}
				}
				strcpy(temp, "");
			}
		}
	}
	close(file_d);
}

// Calculates the average given the loadavg file from proc
void load_average(char *filename) {
	int file_d = open(filename, O_RDONLY);
	char buff[BUF_SZ];
	int counter = 0;

	memset(buff, 0, BUF_SZ);
	if (file_d == -1) {
		printf("There was a problem opening file %s.\n", filename);
	} else {
		read(file_d, buff, BUF_SZ);
		char temp[1000];

		// Goes through the loadavg file and calculates the load average
		for (int i = 0; i < strlen(buff); i++) {
			if (buff[i] != '\n') {
				int len = strlen(temp);
				temp[len] = buff[i];
				temp[len + 1] = '\0';
			} else {
				for (int j = 0; j < strlen(temp); j++) {
					if (counter < 3) {
						if (isdigit(temp[j]) || temp[j] == '.') {
							int a_len = strlen(load_avg);
							load_avg[a_len] = temp[j];
							load_avg[a_len + 1] = '\0';
						} 
						if (temp[j] == ' ') {
							counter++;
							int a_len = strlen(load_avg);
							load_avg[a_len] = temp[j];
							load_avg[a_len + 1] = '\0';
						}
					}
				}
				strcpy(temp, "");
			}
		}
	}
	close(file_d);
}

// Function for calculating the cpu usage
void calculate(int t1, int t2, int i1, int i2) {
	float calc = 0;
	int t_difference = (t2 - t1);
	int i_difference = (i2 - i1);
	calc = ((float) i_difference / (float) t_difference);
	if (isnan(calc)) {
		c_usage = ((float) 1 - 1) * 100;
	} else {
		c_usage = ((float) 1 - calc) * 100;
	}
}

// Function for finding the cpu usage
void cpu_stat(char *filename) {
	char buff[BUF_SZ];
	int t1 = 0;
	int i1 = 0;
	int t2 = 0;
	int i2 = 0;

	// Loop for calculating twice
	for (int t = 0; t < 2; t++) {
		// Sleeps the second time
		if (t == 1) {
			sleep(1);
		}
		int file_d = open(filename, O_RDONLY);
		if (file_d == -1) {
			printf("There was a problem opening file %s.\n", filename);
		} else {
			read(file_d, buff, BUF_SZ);
			char temp[1000];
			memset(temp, 0, 1000);

			// Reads the file line by line
			for (int i = 0; i < strlen(buff); i++) {
				if (buff[i] != '\n') {
					int len = strlen(temp);
					temp[len] = buff[i];
					temp[len + 1] = '\0';
				} else {
					// If the line has the word 'cpu'
					if (strstr(temp, "cpu")) {
						char char_set[250];
						strcpy(char_set, "cpu");
						int start = strspn(temp, char_set);
						int end = strcspn(temp, "\n");
						int space = 0;
						int count = 0;

						char sub_str[100];
						// Goes through the cpu line and adds the 4th number to i1/i2
						// and the altogther total to t1/t2
						for (int j = start; j < end; j++) {
							if (temp[j] != ' ') {
								space++;
								int len = strlen(sub_str);
								sub_str[len] = temp[j];
								sub_str[len + 1] = '\0';
							} else {
								if (space > 0) {
									char temp_calc[250];
									memset(temp_calc, 0, 250);
									count++;
									digits_combiner(sub_str, 0, strlen(sub_str), temp_calc);
									if (t == 0) {
										t1 += atoi(temp_calc);
									} else if (t == 1) {
										t2 += atoi(temp_calc);
									}
									if (count == 4) {
										if (t == 0) {
											i1 = atoi(temp_calc);
										} else if (t == 1) {
											i2 = atoi(temp_calc);
										}
									}
									strcpy(sub_str, "");
								}
							}
							if (j == end - 1) {
								char temp_calc[250];
								memset(temp_calc, 0, 250);
								digits_combiner(sub_str, 0, strlen(sub_str), temp_calc);
								if (t == 0) {
									t1 += atoi(temp_calc);
								} else if (t == 1) {
									t2 += atoi(temp_calc);
								}
								strcpy(sub_str, "");
							}
						}
						// Breaking so it only reads 'cpu'
						break;
					}
					strcpy(temp, "");
				}
			}
			close(file_d);
		}
	}
	// Calling calculate to get the cpu usage
	calculate(t1, t2, i1, i2);
}

// Function for calculating how many hashes to print
int hashes(float average) {
	int count = (int) average / 5;
	return count;
}

// Converting kb to gb
float kb_to_gb(int numb) {
	float convert;
	convert = (((float) numb / 1000000));
	return convert;
}

// Function for calculating the memory info
void mem_info(char *filename) {
	int file_d = open(filename, O_RDONLY);
	char buff[BUF_SZ];
	int total;
	int used;
	bool seen = false;

	memset(buff, 0, BUF_SZ);
	if (file_d == -1) {
		printf("There was a problem opening file %s.\n", filename);
	} else {
		read(file_d, buff, BUF_SZ);
		char temp[1000];

		// Reading the file line by line
		for (int i = 0; i < strlen(buff); i++) {
			if (buff[i] != '\n') {
				int len = strlen(temp);
				temp[len] = buff[i];
				temp[len + 1] = '\0';
			} else {
				// Checks to see if line contains 'MemTotal'
				if (strstr(temp, "MemTotal")) {
					char *char_set = "MemTotal:";
					int start = strspn(temp, char_set);
					int end = strcspn(temp, "\n");
					
					char temp_total[100];
					memset(temp_total, 0, 100);
					// Function that parses the memeory total number from the line
					digits_combiner(temp, start, end, temp_total);
					total = atoi(temp_total);
				// Checks to see if the line contains 'Active'
				} else if (strstr(temp, "Active") && seen == false) {
					char *char_set = "Active:";
					int start = strspn(temp, char_set);
					int end = strcspn(temp, "\n");

					char temp_active[100];
					memset(temp_active, 0, 100);
					// Function that parses the active memory from the line
					digits_combiner(temp, start, end, temp_active);
					used = atoi(temp_active);
					seen = true;
				}
				strcpy(temp, "");
			}
		}
	}
	close(file_d);
	mem_total = kb_to_gb(total);
	mem_used = kb_to_gb(used);
	memory = true;
}

// Function for traversing the proc directory 
void hw_traverse(char *name) {
	DIR *dir = opendir(name);
	struct dirent *sub_dir;

	if (dir != NULL) {
		// Conditional to check if everything is taken care of
		if (strlen(model) == 0 || units == 0 || strlen(load_avg) == 0 || memory == false) {
			while ((sub_dir = readdir(dir)) != NULL) {
				// Creating a variable 'new_dir' that has the exact path of the current working directory
				char *new_name = sub_dir->d_name;
				char *temp = "/";
				char *new_dir = (char *) malloc(100000 + strlen(name) + strlen(new_name));
				char *a = (char *) malloc(100000 + strlen(name) + strlen(temp));
				strcpy(a, name);
				strcat(a, temp);
				strcpy(new_dir, a);
				strcat(new_dir, new_name);

				if (strcmp(sub_dir->d_name, "cpuinfo") == 0) {
					cpu_info(new_dir);
				} else if (strcmp(proc_name, name) == 0 && strcmp(sub_dir->d_name, "stat") == 0) {
					cpu_stat(new_dir);
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

				// Freeing memory
				free(new_dir);
				free(a);
			}
		}
	}
	closedir(dir);
}

// Prints the hardware information
void print_hardware(char *name) {

	int count = 0;
	memset(proc_name, 0, 100);
	strcpy(proc_name, name);
	hw_traverse(name);
	printf("Hardware Information\n");
	printf("--------------------\n");
	printf("CPU Model: %s\n", model);
	printf("Processing Units: %d\n", units);
	printf("Load Average (1/5/15 min): %s\n", load_avg);
	printf("CPU Usage: [");	
	count = ((int) c_usage / 5);
	for (int i = 0; i < 100; i += 5) {
		if (count != 0) {
			printf("#");
			count --;
		} else {
			printf("-");
		}
	}
	printf("] %.1f%% \n", c_usage);
	
	count = 0;
	printf("Memory Usage: [");
	float percentage = ((float) mem_used / (float) mem_total) * 100;
	count = hashes(percentage);
	for (int i = 0; i < 100; i += 5) {
		if (count != 0) {
			printf("#");
			count --;
		} else {
			printf("-");
		}
	}
	printf("] %.1f%% ", percentage);
	printf("(%.1f GB / %.1f GB)\n", mem_used, mem_total);
}