#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
int M;
int N;
bool running = true;
struct arg_struct
{
    pthread_t arg1;
    int arg2;
    int member;
};
struct arg_struct_2
{
    int arg1;
    int arg2;
};

struct coordinate
{
    int X;
    int Y;
};

int CreateBinaryFile(char *sFileName, int N, int M)
{
    FILE *pFile = NULL;
    if ((pFile = fopen(sFileName, "wb+")) == NULL)
    {
        printf("Error in creating file: %s\n", sFileName);
        return -1;
    }
    else
    {
        for (int i = 0; i < N * M; i++)
        {
            char ch = 0;
            fprintf(pFile, "%c", ch);
        }
    }
    fclose(pFile);
    close(fileno(pFile));
    return 0;
}

void Print_Matrix(int N, int M)
{
    unsigned char *buffer = (unsigned char *)malloc(N * M * sizeof(unsigned char));
    FILE *ptr;
    ptr = fopen("battlefield.bin", "r");
    if (ptr == NULL)
    {
        perror("Failed: ");
    }
    fread(buffer, N * M * sizeof(unsigned char), 1, ptr);
    fclose(ptr);
    close(fileno(ptr));
    char temp;
    for (int i = 0; i < N * M; i += N)
    {
        for (int j = 0; j < N; j++)
        {
            temp = buffer[i + j];
            if (temp > 9)
                printf("%u ", temp);
            else
                printf("%u  ", temp);
        }
        printf("\n");
    }
    free(buffer);
}
int check_matrix()
{
    unsigned char *buffer = (unsigned char *)malloc(N * M * sizeof(unsigned char));
    FILE *ptr;
    ptr = fopen("battlefield.bin", "r");
    if (ptr == NULL)
    {
        perror("Failed: ");
        return 1;
    }
    fread(buffer, N * M * sizeof(unsigned char), 1, ptr);
    fclose(ptr);
    close(fileno(ptr));
    int team1 = 0;
    int team2 = 0;
    for (int i = 0; i < N * M; i++)
    {
        if (buffer[i] == 0)
            return 0;
        if (buffer[i] == 1 || buffer[i] == 11 || buffer[i] == 10)
            team1++;
        if (buffer[i] == 2 || buffer[i] == 22 || buffer[i] == 20)
            team2++;
    }
    if (team1 > team2)
    {
        printf("\n");
        printf("\n");
        printf("The results are: \n");
        printf("Team 1: %d points\n", team1);
        printf("Team 2: %d points\n", team2);
        return 1;
    }
    if (team1 < team2)
    {
        printf("The results are: \n");
        printf("Team 1: %d points\n", team1);
        printf("Team 2: %d points\n", team2);
        return 2;
    }
    free(buffer);
    return 3;
}
char get_index(int i, int j, int N, int M)
{
    unsigned char *buffer = (unsigned char *)malloc(N * M * sizeof(unsigned char));
    FILE *ptr;
    ptr = fopen("battlefield.bin", "r");
    if (ptr == NULL)
    {
        perror("Failed: ");
        return 1;
    }
    fread(buffer, N * M * sizeof(unsigned char), 1, ptr);
    fclose(ptr);
    close(fileno(ptr));
    char temp = buffer[(i * N) + j];
    free(buffer);
    return temp;
}
int set_index(int i, int j, int N, int M, int mark)
{
    unsigned char *buffer = (unsigned char *)malloc(N * M * sizeof(unsigned char));
    FILE *ptr;
    ptr = fopen("battlefield.bin", "rb+");
    if (ptr == NULL)
    {
        perror("Failed: ");
        return 1;
    }
    fseek(ptr, (i * N) + j, SEEK_SET);
    fputc(mark, ptr);
    fclose(ptr);
    close(fileno(ptr));
    free(buffer);
    return (i * N) + j;
}
struct coordinate *get_vicinity(struct coordinate *cord, int X, int Y)
{
    struct coordinate board_coordinates[9] = {
        {1, 1}, {0, 1}, {-1, 1}, {1, 0}, {0, 0}, {-1, 0}, {1, -1}, {0, -1}, {-1, -1}};
    for (int i = 0; i < 9; i++)
    {
        cord[i].X = X - board_coordinates[i].X;
        cord[i].Y = Y - board_coordinates[i].Y;
    }
    return cord;
}
void print_vicinity(struct coordinate *temp)
{
    int count = 0;
    char element;
    for (int i = 0; i < 9; i++)
    {
        element = get_index(temp[i].Y, temp[i].X, N, M);
        if (element > 9)
            printf("%u ", element);
        else
            printf("%u  ", element);
        count++;
        if (count > 2)
        {
            count = 0;
            printf("\n");
        }
    }
}
int valid_vicinity(struct coordinate *temp)
{
    char element;
    for (int i = 0; i < 9; i++)
    {
        element = get_index(temp[i].Y, temp[i].X, N, M);
        if (element == 0)
            return 0;
    }
    return 1;
}
void check_vicinity(int X, int Y)
{
    struct coordinate temp[9];
    struct coordinate *cord = get_vicinity(temp, X, Y);
    if (valid_vicinity(cord) == 0)
        return;
    printf("There is a struggle for the region around (%d,%d)\n", cord[4].X, cord[4].Y);
    print_vicinity(cord);
    printf("\n");
    int team1 = 0;
    int team2 = 0;
    char element;
    for (int i = 0; i < 9; i++)
    {
        element = get_index(cord[i].Y, cord[i].X, N, M);
        if (element == 1 || element == 11 || element == 10)
            team1++;
        if (element == 2 || element == 22 || element == 20)
            team2++;
    }
    if (team1 > team2)
    {
        printf("Team 1 has conqured region around (%d,%d)\n", cord[4].X, cord[4].Y);
        char check_player;
        for (int i = 0; i < 9; i++)
        {
            check_player = get_index(cord[i].Y, cord[i].X, N, M);
            if (check_player != 11 && check_player != 22)
                set_index(cord[i].Y, cord[i].X, N, M, 10);
        }
        print_vicinity(cord);
    }
    if (team1 < team2)
    {
        printf("Team 2 has conqured region around (%d,%d)\n", cord[4].X, cord[4].Y);
        char check_player;
        for (int i = 0; i < 9; i++)
        {
            check_player = get_index(cord[i].Y, cord[i].X, N, M);
            if (check_player != 11 && check_player != 22)
                set_index(cord[i].Y, cord[i].X, N, M, 10);
        }
        print_vicinity(cord);
    }
}
void fire_missile(int mark, int player)
{
    int team;
    if (mark == 1 || mark == 11)
    {
        team = 1;
    }
    if (mark == 2 || mark == 22)
    {
        team = 2;
    }
    int X = rand() % N;
    int Y = rand() % M;
    char temp = get_index(Y, X, N, M);
    printf("At location (%d,%d), char is %u \n", X, Y, temp);
    if (temp != 11 && temp != 22 &&
        temp != 10 && temp != 20)
    {
        if (mark == 1 && temp == 1)
        {
            printf("Player %d of Team %d release location (%d,%d) \n", player, team, X, Y);
            set_index(Y, X, N, M, 0);
        }
        else if (mark == 2 && temp == 2)
        {
            printf("Player %d of Team %d release location (%d,%d) \n", player, team, X, Y);
            set_index(Y, X, N, M, 0);
        }
        else
        {
            printf("Player %d of Team %d fire a missle to location (%d,%d) \n", player, team, X, Y);
            set_index(Y, X, N, M, mark);
            if (X > 0 && X < (N - 1) && Y > 0 && Y < (M - 1))
            {
                check_vicinity(X, Y);
            }
        }
    }
    else
    {
        if (temp == 11)
            printf("Player %d of Team %d hit a player of Team 1 at location (%d,%d) \n", player, team, X, Y);
        if (temp == 22)
            printf("Player %d of Team %d hit a player of Team 2 at location (%d,%d) \n", player, team, X, Y);
        if (temp == 10)
            printf("Player %d of Team %d hit a conqured point of Team 1 at location (%d,%d) \n", player, team, X, Y);
        if (temp == 20)
            printf("Player %d of Team %d hit a conqured point of Team 2 at location (%d,%d) \n", player, team, X, Y);
    }
}
int g = 0;
void *myThreadFun(void *vargp)
{
    struct arg_struct *args = vargp;
    int *myid = (int *)args->arg1;
    int mark = args->arg2;
    int player = args->member;
    int s = 0;
    while (running == true)
    {
        pthread_mutex_lock(&myMutex);
        printf("Thread ID: %d, Missle Fired: %d, Total Missles Fired(All): %d \n", *myid, ++s, ++g);
        fire_missile(mark, player);
        Print_Matrix(N, M);
        pthread_mutex_unlock(&myMutex);
        sleep(rand() % 4);
    }
}
void *supervisor(void *vargp)
{
    int *myid = (int *)vargp;
    static int s = 0;
    ++s;
    ++g;
    while (running == true)
    {
        int temp = check_matrix();
        if (temp != 0)
        {
            if (temp == 1)
                printf("Team 1 won \n");
            if (temp == 2)
                printf("Team 2 won \n");
            if (temp == 3)
                printf("Draw \n");
            running = false;
            printf("\n");
            printf("Final state of the game: \n");
            printf("\n");
            Print_Matrix(N, M);
            printf("\n");
        }
        sleep(1);
    }
}
void *member_thread(void *argp)
{
    struct arg_struct_2 *args = argp;
    int size = args->arg1;
    int mark = args->arg2;
    pthread_t *p1 = malloc(sizeof(pthread_t) * size);
    pthread_t *p2 = malloc(sizeof(pthread_t) * size);
    struct arg_struct *arg = malloc(sizeof(struct arg_struct) * size);
    for (int i = 0; i < size; i++)
    {
        int player = i + 1;
        arg[i].arg2 = mark;
        arg[i].member = player;
        pthread_create(&p1[i], NULL, &myThreadFun, (void *)&arg[i]);
        arg[i].arg1 = p1[i];
        pthread_create(&p2[i], NULL, &supervisor, (void *)&p2[i]);
    }
    for (int i = 0; i < size; i++)
    {
        pthread_join(p1[i], NULL);
        pthread_join(p2[i], NULL);
    }
}
void set_players(int num1, int num2, int N, int M)
{
    int X;
    int Y;
    char temp;
    for (int i = 0; i < (num1 + num2); i++)
    {
        X = rand() % N;
        Y = rand() % M;
        temp = get_index(Y, X, N, M);
        while (temp == 11 || temp == 22)
        {
            X = rand() % N;
            Y = rand() % M;
            temp = get_index(Y, X, N, M);
        }
        if (i < num1)
        {
            set_index(Y, X, N, M, 11);
        }
        else
        {
            set_index(Y, X, N, M, 22);
        }
    }
}
//***************************************
// main function
//***************************************
int main(int argc, char *argv[])
{
    M = atoi(argv[3]);
    N = atoi(argv[4]);
    int num1 = atoi(argv[1]);
    int num2 = atoi(argv[2]);
    CreateBinaryFile("battlefield.bin", N, M);
    pthread_t p1, p2;
    int *arg = malloc(sizeof(*arg));
    *arg = num1;
    srand(time(NULL));
    set_players(num1, num2, N, M);
    struct arg_struct_2 args1;
    struct arg_struct_2 args2;
    args1.arg1 = *arg;
    args1.arg2 = 1;
    if (pthread_create(&p1, NULL, &member_thread, (void *)&args1) != 0)
    {
        return 1;
    }
    int *arg2 = malloc(sizeof(*arg2));
    *arg2 = num2;
    args2.arg1 = *arg2;
    args2.arg2 = 2;
    if (pthread_create(&p2, NULL, &member_thread, (void *)&args2) != 0)
    {
        return 2;
    }
    if (pthread_join(p1, NULL) != 0)
    {
        return 3;
    }
    if (pthread_join(p2, NULL) != 0)
    {
        return 4;
    }
    printf("\n");
    printf("Signs:\n");
    printf("Players:\n");
    printf("Team 1: 11, Team 2: 22\n");
    printf("Conqured points:\n");
    printf("Team 1: 10, Team 2: 20\n");
    printf("Occupied points:\n");
    printf("Team 1: 1, Team 2: 2\n");
    if (remove("battlefield.bin") != 0)
        printf("Unable to delete the file");
    return 0;
}
