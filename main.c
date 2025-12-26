#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <sys/ioctl.h>
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

void getTerminalSize(int *width)
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    *width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    *width = w.ws_col;
#endif
}
#define MOVE_CURSOR(r, c) printf("\033[%d;%dH", r, c);

int main_head, mini_board, super_board, width;
int essentials(void)
{
    if (width < 40)
    {
        printf("Too small of a screen size to play.");
        return 0;
    }
    int board_width = 12;

    main_head = width / 2 - 9;
    if (main_head < 20)
        main_head = 20;
    printf("%d  ", main_head);
    if (width >= 60)
    {
        printf("NORM LAW");
        mini_board = width * 0.2;
        super_board = mini_board + width * 0.4 + 5;
    }
    else
    {
        printf("ALT LAW");
        mini_board = 2;
        super_board = width - board_width - 2;
    }
    return 1;
}

char board[9][9];
int win_pos[] = {9, 9, 9}, current = 0, active = 0;
int win_lines[8][3] = {
    {0, 1, 2}, // 1st row
    {3, 4, 5}, // 2nd row
    {6, 7, 8}, // 3rd row
    {0, 3, 6}, // 1st col
    {1, 4, 7}, // 2nd col
    {2, 5, 8}, // 3rd col
    {0, 4, 8}, // rht dig
    {2, 4, 6}  // lft dig
};

void init(void);
void inner_gameplay(int, int, int, int);
void regular_print(int);
void print_board(int);
int check_win(void);
int game_mode(int);
char get_keys(void);

void init(void)
{
    // board initialization
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            board[i][j] = '1' + j;
            // board[i][0] = '_';
            // board[i][1] = 'X';
    board[0][0] = '_';
    board[0][1] = 'X';
    board[5][0] = '_';
    board[5][1] = '=';
    board[8][0] = '_';
    board[8][1] = 'O';
    board[4][5] = 'X';
    board[3][1] = 'O';
}

void inner_gameplay(int game, int player, int status, int choice)
{
    int flag = 0, box;
    while (1)
    {
        if (choice)
            print_board(status);
        else
            regular_print(status);
        char mark = player == 1 ? 'X' : 'O';
        if (flag)
            printf("\033[1;35mInvalid input by Player %d\n\tEnter again.\033[0m\n", player);
        printf("\033[1mPlayer %d ", player);
        if (player == 1)
            printf("\033[31m[X]");
        else
            printf("\033[32m[O]");
        printf("\033[0m\033[1m, enter position (1-9): ");
        if (scanf("%d", &box) != 1) // char validation
        {
            while (getchar() != '\n')
                ;
            flag = 1;
            continue;
        }
        while (getchar() != '\n')
            ;                                                        // float validation
        if (box < 1 || box > 9 || board[game][box - 1] != box + '0') // valid bounds + occupied box
        {
            flag = 1;
            continue;
        }
        board[game][box - 1] = mark;
        break;
    }
}

int main(void)
{
    getTerminalSize(&width);
    if (!essentials())
        return 0;
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    int player = 1, status = 0;
    char mark;
    int choice = game_mode(0);
    init();
    while (1)
    {
        printf("mode: %d\n", choice);
        if (choice == 2)
            return 0;
        if (!choice)
            inner_gameplay(0, player, status, choice);
        else
        {
            if (!active)
            {
            _extras:
                while (board[current][0] == '_')
                {
                    current++;
                    if (current == 9)
                        break;
                }
            _start:
                print_board(status);
                printf("\033[1mPlayer %d ", player);
                if (player == 1)
                    printf("\033[31m[X]");
                else
                    printf("\033[32m[O]");
                printf("\033[0m\033[1;36m, Press TAB to change the board and ENTER to select.\033[0m");
                char key = get_keys();
                if (key == 'T')
                {
                    current++;
                    if (current == 9)
                        current = 0;
                    while (board[current][0] == '_')
                    {
                        current++;
                        if (current == 9)
                        {
                            current = 0;
                            break;
                        }
                    }
                    goto _extras;
                }
                else if (key == 'E')
                    active = 1;
                else
                    goto _start;
            }
            inner_gameplay(current, player, status, choice);
        }
        status = check_win();
        player = player == 1 ? 2 : 1;
    }
    return 0;
}

void regular_print(int status)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    printf("\t\033[1;34m┌────────────────┐\n");
    printf("\t│TIC TAC TOE GAME│\n");
    printf("\t└────────────────┘\033[0m\n");
    printf("\t\033[1mPlayer 1: \033[1;31mX (RED)\033[0m");
    printf("\n\t\033[1mPlayer 2: \033[1;32mO (Green)\033[0m\n\n");
    for (int i = 0; i < 3; i++)
    {
        printf("\033[1m\n\t    ");
        for (int j = 0; j < 3; j++)
        {
            char mark = board[0][j + i * 3];
            if (mark == 'X')
                printf("\033[31m");
            else if (mark == 'O')
                printf("\033[32m");
            if (status)
                for (int k = 0; k < 3; k++)
                    if (win_pos[k] == j + i * 3)
                        printf("\033[4;34m");
            printf("%c\033[0m\033[1m", mark);
            j != 2 ? printf(" │ ") : 0;
        }
        i != 2 ? printf("\n\t   ───┼───┼───") : 0;
    }
    printf("\n\n");
    /*
    i j -> n
    0 0 -> 0
    0 1 -> 1
    0 2 -> 2
    1 0 -> 3
    1 1 -> 4
    1 2 -> 5
    2 0 -> 6
    2 1 -> 7
    2 2 -> 8
    n = j + i*3
    */
}

void print_board(int status)
{
    int pos;
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    printf("\033[1mPlayer 1: \033[1;31mX (RED)\033[0m");
    printf("\n\033[1mPlayer 2: \033[1;32mO (Green)\033[0m\n\n");

    MOVE_CURSOR(1, main_head);
    printf("\033[1;34m┌────────────────┐\n");
    MOVE_CURSOR(2, main_head);
    printf("│TIC TAC TOE GAME│\n");
    MOVE_CURSOR(3, main_head);
    printf("└────────────────┘\033[0m\n");

    if (active)
        printf("\033[35m");
    MOVE_CURSOR(4, mini_board);
    printf("\033[1m┌───────────┐\n");
    MOVE_CURSOR(5, mini_board);
    printf("│INNER  GAME│\n");
    MOVE_CURSOR(6, mini_board);
    printf("└───────────┘\033[0m\n");

    if (!active)
        printf("\033[35m");
    MOVE_CURSOR(4, super_board);
    printf("\033[1m┌───────────┐\n");
    MOVE_CURSOR(5, super_board);
    printf("│SUPER BOARD│\n");
    MOVE_CURSOR(6, super_board);
    printf("└───────────┘\033[0m\n");

    for (int i = 0; i < 3; i++)
    {
        MOVE_CURSOR(i * 2 + 7, mini_board + 2);
        printf("\033[1m");
        for (int j = 0; j < 3; j++)
        {
            pos = j + i * 3;
            char mark = board[current][pos];
            if (mark == 'X')
                printf("\033[31m");
            else if (mark == 'O')
                printf("\033[32m");
            if (status)
                for (int k = 0; k < 3; k++)
                    if (win_pos[k] == pos)
                        printf("\033[4;34m");
            printf("%c\033[0m\033[1m", mark);
            j != 2 ? printf(" │ ") : 0;
        }
        MOVE_CURSOR(i * 2 + 8, mini_board + 1);
        i != 2 ? printf("───┼───┼───") : 0;
    }

    for (int i = 0; i < 3; i++)
    {
        MOVE_CURSOR(i * 2 + 7, super_board + 2);
        printf("\033[1m");
        for (int j = 0; j < 3; j++)
        {
            pos = j + i * 3;
            char solved = board[pos][0], mark;
            // setting the mark
            if (solved == '_')
            {
                mark = board[pos][1]; // X, O, =
            }
            else
                mark = '_';
            if (pos == current)
                mark = '\0';
            // assigning color to it
            if (mark == 'X')
                printf("\033[31m");
            else if (mark == 'O')
                printf("\033[32m");
            else if (mark == '=')
                printf("\033[33m");
            else if (mark == '\0')
                printf("\033[36m");
            else
                printf("\033[35m"); // for under_score

            if (status)
                for (int k = 0; k < 3; k++)
                    if (win_pos[k] == pos)
                        printf("\033[4;34m");
            if (mark)
                printf("%c\033[0m\033[1m", mark);
            else
                printf("■\033[0m\033[1m");
            j != 2 ? printf(" │ ") : 0;
        }
        MOVE_CURSOR(i * 2 + 8, super_board + 1);
        i != 2 ? printf("───┼───┼───") : 0;
    }
    printf("\n\n");
}

int check_win(void)
{
    // Win Check
    for (int i = 0; i < 8; i++)
    {
        int pos1 = win_lines[i][0];
        int pos2 = win_lines[i][1];
        int pos3 = win_lines[i][2];
        if (board[0][pos1] == board[0][pos2] && board[0][pos2] == board[0][pos3])
        {
            win_pos[0] = pos1;
            win_pos[1] = pos2;
            win_pos[2] = pos3;
            return 1;
        }
    }

    // Draw Check
    int filled_box = 0;
    for (int i = 0; i < 9; i++)
    {
        if (board[0][i] == 'X' || board[0][i] == 'O')
            filled_box++;
    }
    if (filled_box == 9)
        return -1;
    return 0; // continue as normal
}

int game_mode(int choice)
{
    while (1)
    {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        printf("\033[1;30m=====LOG=====\nrows: %d\nmain: %d\nmini: %d\nsuper: %d\n=====LOG=====\n\033[0m", width, main_head, mini_board, super_board);
        printf("\033[1mUse arrow-keys or (1-3) to select:\n");
        if (!choice)
            printf("\033[4;34m> ");
        printf("Regular 3x3\033[0m\n");
        if (choice == 1)
            printf("\033[1;4;34m> ");
        printf("\033[1mSuper mode\033[0m\n");
        if (choice == 2)
            printf("\033[1;4;31m> ");
        printf("\033[1;31mExit\033[0m\n");
        printf("\033[1mPress Enter to confirm your choice\n");
        char ch = get_keys();
        if (ch == 'U')
            choice--;
        else if (ch == 'D')
            choice++;
        else if (ch == 'E')
            return choice;
        else if (ch > '0' && ch < '4')
            choice = ch - '1';
        if (choice == 3)
            choice = 2;
        else if (choice == -1)
            choice = 0;
    }
}

char get_keys(void)
{
    char ch = getch();
    if (ch == 0 || ch == -32 || ch == 27)
    {
#ifdef __linux__
        getch();
#endif
        ch = getch();
        switch (ch)
        {
        case 'H':
        case 'A':
            return 'U';
            break;
        case 'P':
        case 'B':
            return 'D';
            break;
        case 'M':
        case 'C':
            return 'R';
            break;
        case 'K':
        case 'D':
            return 'L';
            break;
        default:
            break;
        }
    }
    if (ch == '\n' || ch == '\r')
        return 'E';
    if (ch == '\t')
        return 'T';
    return ch;
}

/*
 X │ ◽ │ O
───┼───┼───
 = │ ☐ │ 6
───┼───┼───
 ⃞ │ ▣ │
*/
/*
 1 │ □ │ 3
───┼───┼───
 4 | █ │ ■
───┼───┼───
 ■ │ ▀ │ 9
*/

/*
        if (status)
        {
            print_board(status);
            if (status == 1)
            {
                printf("\033[1;36m\tPlayer %d Wins!!\n", player);
            }
            else if (status == -1)
            {
                printf("\033[1;33m\tThis is a draw...\n");
            }
            printf("\033[0m");
            char choice = 'y';
            do
            {
                if (choice != 'y')
                    printf("Enter valid input.\n");
                printf("\nPlay again? (y/n): ");
                scanf(" %c", &choice);
            } while (choice != 'y' && choice != 'Y' && choice != 'n' && choice != 'N');

            if (choice == 'y' || choice == 'Y')
            {
                choice = game_mode(0);
                for (int i = 0; i < 9; i++)
                    board[0][i] = '1' + i;
                for (int i = 0; i < 3; i++)
                    win_pos[i] = 9;

                player = 1;
                status = 0;
                continue;
            }
            else
                return 0;
        }
*/