#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

volatile sig_atomic_t timeout_flag = 0;

void handle_timeout(int signum) { timeout_flag = 1; }
void handler_sigint(int signum) { /* do nothing */ }

int mygetchar(int timeout_sec) {
    struct sigaction sa_alarm, sa_alarm_backup;
    struct sigaction sa_sigint, sa_sigint_backup;
    struct sigevent sev;
    timer_t timerid;
    struct itimerspec itimer;

    // sigint handler
    sigemptyset(&sa_sigint.sa_mask); // memset(&sa, 0, sizeof(sa));
    sa_sigint.sa_flags = 0;
    sa_sigint.sa_handler = handler_sigint;
    if (sigaction(SIGINT, &sa_sigint, &sa_sigint_backup) < 0) {
        perror("sigaction");
        return -3;
    }

    // timeout handler
    sigemptyset(&sa_alarm.sa_mask); // memset(&sa, 0, sizeof(sa));
    sa_alarm.sa_flags = 0;
    sa_alarm.sa_handler = handle_timeout;
    if (sigaction(SIGALRM, &sa_alarm, &sa_alarm_backup) < 0) {
        perror("sigaction");
        return -3;
    }

    // create timer
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;
    sev.sigev_value.sival_ptr = &timerid;
    if (timer_create(CLOCK_REALTIME, &sev, &timerid) < 0) {
        perror("timer_create");
        return -3;
    }

    // set timer
    itimer.it_value.tv_sec = itimer.it_interval.tv_sec = timeout_sec;
    itimer.it_value.tv_nsec = itimer.it_interval.tv_nsec = 0;
    if (timer_settime(timerid, 0, &itimer, NULL) < 0) {
        perror("timer_settime");
        return -3;
    }

    // wait until input
    int ch = getchar();

    // delete timer
    if (timer_delete(timerid) < 0) {
        perror("timer_delete");
        return -3;
    }
    if (sigaction(SIGALRM, &sa_alarm_backup, NULL) < 0) {
        perror("sigaction");
        return -3;
    }
    if (sigaction(SIGINT, &sa_sigint_backup, NULL) < 0) {
        perror("sigaction");
        return -3;
    }

    if (ch == EOF) {
        if (timeout_flag) {
            timeout_flag = 0; // reset
            ch = -2;          // timeout
        } else if (feof(stdin)) {
            ch = -1; // EOF
        } else {
            ch = -3; // other signal
        }
    }

    return ch;
}

void print_current_time() {
    time_t current_time = time(NULL);
    char *time_str = ctime(&current_time);
    printf("Current time: %s", time_str);
}

int main(void) {
    print_current_time();
    printf("within 5 seconds: ");

    int ch = mygetchar(5);

    printf("\nmygetchar returned: %d\n", ch);
    if (ch >= 0) {
        printf("\ninput: '%c' (%d)\n", ch, ch);
    }

    printf("==========\n");
    print_current_time();

    sleep(5);

    return 0;
}
