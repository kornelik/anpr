#include "conio.h"

#include <memory.h>
#include <sys/time.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

static struct termios g_old_kbd_mode;

void cooked(void) {
    tcsetattr(0, TCSANOW, &g_old_kbd_mode);
}

void raw(void) {
    static char init;
    struct termios new_kbd_mode;

    if (init) {
        return;
    }

    tcgetattr(0, &g_old_kbd_mode);
    memcpy(&new_kbd_mode, &g_old_kbd_mode, sizeof(struct termios));
    new_kbd_mode.c_lflag &= ~(ICANON | ECHO);
    new_kbd_mode.c_cc[VTIME] = 0;
    new_kbd_mode.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &new_kbd_mode);

    atexit(cooked);
    init = 1;
}

int kbhit(void) {
    struct timeval timeout;
    fd_set read_handles;
    int status;

    raw();
    FD_ZERO(&read_handles);
    FD_SET(0, &read_handles);
    timeout.tv_sec = timeout.tv_usec = 0;
    status = select(0 + 1, &read_handles, NULL, NULL, &timeout);
    if(status < 0) {
        printf("select() failed in kbhit()\n");
        exit(1);
    }
    return status;
}

int getch(void) {
    unsigned char temp;
    raw();
    if(read(0, &temp, 1) != 1) {
        return 0;
    }
    return temp;
}
