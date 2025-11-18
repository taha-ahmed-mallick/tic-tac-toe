#include <stdio.h>
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
int getch(void)
{
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // Turn off canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif

char get_keys(void)
{
    char ch = getch();
    if (ch == 0 || ch == -32 || ch == 27)
    {
#ifdef __linux__
        getch();
#endif
        ch = getch();
        printf("%c", ch);
        switch (ch)
        {
        case 'H':
        case 'A': printf("->U\n");
            break;
        case 'P':
        case 'B': printf("->D\n");
            break;
        case 'M':
        case 'C': printf("->R\n");
            break;
        case 'K':
        case 'D': printf("->L\n");
            break;
        default:
            break;
        }
        return ch;
    }
    printf("\n...%c", ch);
}

int main()
{
    while (1)
        get_keys();
    return 0;
}