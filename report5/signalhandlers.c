#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// handler for SIGSEGV
void handle_sigsegv(int signum, siginfo_t *info, void *ctx) {
    const char *msg = "Caught signal SIGSEGV: Invalid memory access.\n";
    const char *addr_msg = "Attempted to access memory address: ";
    const char *newline = "\n";
    char addr_str_buffer[20];

    snprintf(addr_str_buffer, sizeof(addr_str_buffer), "%p", info->si_addr);

    write(STDERR_FILENO, msg, strlen(msg));
    write(STDERR_FILENO, addr_msg, strlen(addr_msg));
    write(STDERR_FILENO, addr_str_buffer, strlen(addr_str_buffer));
    write(STDERR_FILENO, newline, strlen(newline));

    exit(EXIT_FAILURE);
}

// handler for SIGFPE
void handle_sigfpe(int signum, siginfo_t *info, void *ctx) {
    const char *msg = "Caught signal SIGFPE: Division by zero.\n";

    write(STDERR_FILENO, msg, strlen(msg));

    exit(EXIT_FAILURE);
}

// handler for SIGWINCH
void handle_sigwinch(int signum, siginfo_t *info, void *ctx) {
    const char *msg = "Caught signal SIGWINCH: Window size changed.\n";

    write(STDERR_FILENO, msg, strlen(msg));

    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s type\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (strlen(argv[1]) != 1) {
        fprintf(stderr, "invalid argument.\n");
        exit(EXIT_FAILURE);
    }

    char type = argv[1][0];
    struct sigaction sa;
    switch (type) {
    case '0':
        // Illegal memory access
        sigemptyset(&sa.sa_mask); // memset(&sa, 0, sizeof(sa));
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = handle_sigsegv;

        if (sigaction(SIGSEGV, &sa, NULL) < 0) {
            perror("sigaction");
            exit(EXIT_FAILURE);
        }

        int *ptr = NULL;
        *ptr = 998244353; // SIGSEGV
        break;
    case '1':
        // division by zero
        sigemptyset(&sa.sa_mask); // memset(&sa, 0, sizeof(sa));
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = handle_sigfpe;

        if (sigaction(SIGFPE, &sa, NULL) < 0) {
            perror("sigaction");
            exit(EXIT_FAILURE);
        }

        int x = 1700;
        int y = 0;
        y = x / y; // SIGFPE
        break;
    case '2':
        // SIGWINCH
        sigemptyset(&sa.sa_mask); // memset(&sa, 0, sizeof(sa));
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = handle_sigwinch;

        if (sigaction(SIGWINCH, &sa, NULL) < 0) {
            perror("sigaction");
            exit(EXIT_FAILURE);
        }

        pause(); // wait until WINCH
        break;
    default:
        fprintf(stderr, "invalid argument.\n");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
