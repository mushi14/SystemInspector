#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "system_info.h"
#include "hardware_info.h"
#include "task_info.h"

/* Preprocessor Directives */
#ifndef DEBUG
#define DEBUG 1
#endif
/**
 * Logging functionality. Set DEBUG to 1 to enable logging, 0 to disable.
 */
#define LOG(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
            __LINE__, __func__, __VA_ARGS__); } while (0)


/* Function prototypes */
void print_usage(char *argv[]);


/* This struct is a collection of booleans that controls whether or not the
 * various sections of the output are enabled. */
struct view_opts {
    bool hardware;
    bool system;
    bool task_list;
    bool task_summary;
};

void print_usage(char *argv[])
{
    printf("Usage: %s [-ahlrst] [-p procfs_dir]\n" , argv[0]);
    printf("\n");
    printf("Options:\n"
"    * -a              Display all (equivalent to -lrst, default)\n"
"    * -h              Help/usage information\n"
"    * -l              Task List\n"
"    * -p procfs_dir   Change the expected procfs mount point (default: /proc)\n"
"    * -r              Hardware Information\n"
"    * -s              System Information\n"
"    * -t              Task Information\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    /* Default location of the proc file system */
    char *procfs_loc = "/proc";

    /* Set to true if we are using a non-default proc location */
    bool alt_proc = false;

    struct view_opts all_on = { true, true, true, true };
    struct view_opts options = { false, false, false, false };

    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "ahlp:rst")) != -1) {
        switch (c) {
            case 'a':
                options = all_on;
                print_sys(procfs_loc);
                print_hardware(procfs_loc);
            case 'h':
                print_usage(argv);
                return 0;
            case 'l':
                options.task_list = true;
                break;
            case 'p':
                procfs_loc = optarg;
                alt_proc = true;
                break;
            case 'r':
                options.hardware = true;
                print_hardware(procfs_loc);
            case 's':
                options.system = true;
                print_sys(procfs_loc);
            case 't':
                options.task_summary = true;
                // print_task(procfs_loc);
                // break;
            case '?':
                if (optopt == 'p') {
                    fprintf(stderr,
                            "Option -%c requires an argument.\n", optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                } else {
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n", optopt);
                }
                print_usage(argv);
                return 1;
            default:
                abort();
        }
    }

    if (alt_proc == true) {
        LOG("Using alternative proc directory: %s\n", procfs_loc);

        /* Remove two arguments from the count: one for -p, one for the
         * directory passed in: */
        argc = argc - 2;
    }

    if (argc <= 1) {
        /* No args (or -p only). Enable all options: */
        options = all_on;
    }

    LOG("Options selected: %s%s%s%s\n",
            options.hardware ? "hardware " : "",
            options.system ? "system " : "",
            options.task_list ? "task_list " : "",
            options.task_summary ? "task_summary" : "");
    
    return 0;
}
