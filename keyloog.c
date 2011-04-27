#define _BSD_SOURCE // For usleep() and getdtablesize()
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <math.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>

void daemonize(const char *option_pidfile)
{
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    } else if (pid > 0) {
        // Write child PID to file
        if (option_pidfile) {
            FILE *pidfile = fopen(option_pidfile, "w");
            if (!pidfile) {
                perror(option_pidfile);
                exit(1);
            }
            fprintf(pidfile, "%d\n", pid);
            fclose(pidfile);
        }
        exit(0);
    }
    
    // Detach from controlling terminal
    setsid();
    
    // Close all file descriptors
    for (int i = getdtablesize(); i >= 0; i--)
        close(i);
    
    // Open stdin, stdout, stderr to /dev/null
    int fd = open("/dev/null", O_RDWR);
    dup(fd);
    dup(fd);
}

// Signal handlers
bool end = false;

void signal_quit(int s)
{
    end = true;
}

void print_usage(const char *exec_name)
{
    printf("Usage: %s [OPTION]... [FILE]\n\n", exec_name);
    printf("  -d, --daemonize       run in the background\n");
    printf("  -p, --pid-file=FILE   write PID to FILE\n");
    printf("  -h, --help            display this help and exit\n");
}

int main(int argc, char *argv[])
{
    // Parse command-line options
    static struct option long_options[] = {
        {"daemonize", no_argument, NULL, 'd'},
        {"pid-file", required_argument, NULL, 'p'},
        {"help", no_argument, NULL, 'h'},
        {0, 0, 0, 0}
    };
    
    bool option_daemonize = false;
    char *option_pidfile = NULL, *option_file = NULL;
    
    char o;
    while ((o = getopt_long(argc, argv, "dp:h", long_options, NULL)) != -1) {
        switch (o) {
        case 'd':
            option_daemonize = true;
            break;
        case 'p':
            option_pidfile = optarg;
            break;
        case 'h':
            print_usage(argv[0]);
            return 0;
        case '?': // Unrecognized argument
            return 1;
        }
    }
    
    if (optind < argc)
        option_file = argv[optind];
    
    // Set up signals
    signal(SIGTERM, signal_quit);
    signal(SIGQUIT, signal_quit);
    signal(SIGINT, signal_quit);
    
    // Daemonize
    if (option_daemonize)
        daemonize(option_pidfile);
    
    // Open X display
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Error: Could not open X display\n");
        return 1;
    }
    
    char keys_current[32], keys_last[32];
    
    XQueryKeymap(display, keys_last);
    while (!end) {
        fflush(stdout);
        usleep(5000);
        
        XQueryKeymap(display, keys_current);
        for (int i = 0; i < 32; i++) {
            if (keys_current[i] > keys_last[i]) {
                // TODO: Things
            }
            keys_last[i] = keys_current[i];
        }
    }
    
    XCloseDisplay(display);
    
    return 0;
}
