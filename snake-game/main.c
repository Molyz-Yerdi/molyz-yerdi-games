#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

#define COLS 60
#define ROWS 30

void render_field(void);

int main()
{
    // Hide cursor
    printf("\e[?25l");

    // Configure terminal (disable Enter input and echo)
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    int x[1000], y[1000];
    int quit = 0;

    while (!quit)
    {
        // Render the game field
        render_field();

        // Move the cursor back up to redraw the field
        printf("\e[%iA", ROWS + 2);

        int x[1000], y[1000];
        int head = 0, tail = 0;
        int quit = 0, gameover = 0;
        int xdir = 1, ydir = 0;
        int applex = -1, appley;

        x[head] = COLS / 2;
        y[head] = ROWS / 2;

        while (!quit && !gameover)
        {
            if (applex < 0)
            {
                applex = rand() % COLS;
                appley = rand() % ROWS;

                for (int i = tail; i != head; i = (i + 1) % 1000)
                    if (x[i] == applex && y[i] == appley)
                        applex = -1;

                if (applex >= 0)
                {
                    printf("\e[%iB\e[%iC❤", appley + 1, applex + 1);
                    printf("\e[%iF", appley + 1);
                }
            }

            // Erase the snake's tail
            printf("\e[%iB\e[%iC·", y[tail] + 1, x[tail] + 1);
            printf("\e[%iF", y[tail] + 1);

            if (x[head] == applex && y[head] == appley)
            {
                applex = -1;
                printf("\a"); // Sound (beep)
            }
            else
                tail = (tail + 1) % 1000;

            int newhead = (head + 1) % 1000;
            x[newhead] = (x[head] + xdir + COLS) % COLS;
            y[newhead] = (y[head] + ydir + ROWS) % ROWS;
            head = newhead;

            // Check for self-collision
            for (int i = tail; i != head; i = (i + 1) % 1000)
                if (x[i] == x[head] && y[i] == y[head])
                    gameover = 1;

            // Draw the snake's head
            printf("\e[%iB\e[%iC▓", y[head] + 1, x[head] + 1);
            printf("\e[%iF", y[head] + 1);
            fflush(stdout);

            usleep(5 * 1000000 / 60);

            // Handle keyboard input
            struct timeval tv = {0, 0};
            fd_set fds;

            FD_ZERO(&fds);
            FD_SET(STDIN_FILENO, &fds);
            select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);

            if (FD_ISSET(STDIN_FILENO, &fds))
            {
                int ch = getchar();
                if (ch == 'q') quit = 1;
                else if (ch == 'a' && xdir != 1) { xdir = -1; ydir = 0; } // left
                else if (ch == 'd' && xdir != -1) { xdir = 1; ydir = 0; } // right
                else if (ch == 's' && ydir != -1) { xdir = 0; ydir = 1; } // down
                else if (ch == 'w' && ydir != 1) { xdir = 0; ydir = -1; } //
            }
        }

        if (!quit)
        {
            printf("\e[%iB\e[%iC Game Over! ", ROWS / 2, COLS / 2 - 5);
            printf("\e[%iF", ROWS / 2);
            fflush(stdout);
            getchar();
        }
    }

    // Restore terminal settings
    printf("\e[?25h");
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return 0;
}

void render_field()
{
    printf("┌");
    for (int i = 0; i < COLS; i++)
        printf("─");
    printf("┐\n");

    for (int j = 0; j < ROWS; j++)
    {
        printf("│");
        for (int i = 0; i < COLS; i++)
            printf("·");
        printf("│\n");
    }

    printf("└");
    for (int i = 0; i < COLS; i++)
        printf("─");
    printf("┘\n");
}