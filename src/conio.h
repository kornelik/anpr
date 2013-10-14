#include <sys/time.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

void cooked(void);
void raw(void);
int kbhit(void);
int getch(void);