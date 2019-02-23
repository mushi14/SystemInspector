# Project 1: System Inspector

See: https://www.cs.usfca.edu/~mmalensek/cs326/assignments/project-1.html 

To compile and run:

```bash
make
./inspector
```

## Testing

To execute the test cases, use `make test`. To pull in updated test cases, run `make testupdate`. You can also run a specific test case instead of all of them:

```
# Run all test cases:
make test

# Run a specific test case:
make test run=4

# Run a few specific test cases (4, 8, and 12 in this case):
make test run='4 8 12'
```



Mushahid Hassan
CS 326, Section 1
Matthew Malensek
Project 1

For this project I decided to create 3 header files in which I did almost all of my work in. Each header file finds and
and specific part of the project. system_info.h works with the system info, hardware_info with the hardware info, etc. 
I decided to traverse through the proc in this project. So each header file has its own traversing function that traverses
through the proc directory looking for specific files that contain the info that is needed for this project. For example,
for hardware information, cpu usage and memory usage is needed. So in my hardware_info header file I have a traversal 
function that traverses through the given proc and finds the "cpuinfo" and "meminfo" files and parses them to find the
cpu and memory usage. This traversal function is present in every header file except for the task_info, which doesn't 
need traversing because it goes through each file in proc to find the tasks and later goes through "status" file in the 
task directory to find the information needed about the particular task.


- system_info.h
	This file is used to find and display the system information. The print_sys function is called in the inspector.c
	which takes in a filename (proc). sys_traverse is called which traverses through the given filename (proc) and finds
	any file with the name of either "hostname", "version", or "uptime". If hostname is found, the program then calls another
	function named file_read which reads the file and updates the variable hostname as the string that is found within the
	file. Same thing is done for version. Uptime is different. Once uptime file is found, the program reads the file and
	parses it to find the uptime. Once uptime is found, another function (convet) is called which convert the uptime from
	total seconds into years, days, hours, mintues, and seconds. All of the this info is printed in the print_sys.
	
	
- hardware_info.h
	This file is used to find and display the hardware information. The print_hardware function is called in the incpector.c
	which takes in a filename (proc). hw_traverse is called in the print_hardware function which traverses through the given
	filename (proc) and finds any file with the name of "cpuinfo", "meminfo", or "loadavg". Once either of the one is found
	the program goes into their respective helper function and populates the variables. For example for cpuinfo, cpu_stat
	is called which reads the file line by line and tries to find the line that contains "cpu" followed by numbers. Once this 
	lines is found, another helper function digits_combiner is called which finds the total and idle of the cpu 2 times. 
	Later, calculate function is called which calculates the cpu usage for the system. Same is done for load average and
	memory info. All of this information is called and printed in print_hrdware.
	

- task_info.h
	This file is used to find and display the task information and the task list. The print_task and print_list functions
	are called in inspector.c which takes in a filename (proc). print_task function calls get_count function finds the total
	number of tasks running. Once this is found, print_task calls gather_info which populates interrupts, switches, and forks
	variables for the given filename (proc). All of this is printed in print_task. The print_list function calls task_processor
	which finds all the files in the given proc and looks for the ones that named as numbers only as those are the tasks.
	For each file, task_info is called. task_info goes through the task directory and looks for "status" and "comm" files. 
	When the file named "comm" is found, the program goes in the file and retrives the task's name from the file and populates
	the variable task_name as such. When status file is found, it is then read line by line and parsed looking for words
	such as "pid", "state", "uid", and "threads". When these lines are found, their respective variables are set to the
	information found. When all of this is done, a new task struct is created and given these variables. These variables are
	then printed within the task_info function.
