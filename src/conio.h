#include <sys/time.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

static void cooked(void);
static void raw(void);
static int kbhit(void);
static int getch(void);