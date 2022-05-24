#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
int PORT_NUMBER = 54321;
char *ips = "127.0.0.1";
int max_burger = 10;
char *out_of_stock = "Krustys Ran out of burgers\n";
int count = 0;
int action(int sock)
{
    char server_reply[2000];
    char *message = "1";
    int time_to_eat = 0;
    puts("Connected\n");
    recv(sock, server_reply, 2000, 0);
    puts("Server reply :");
    if (strcmp(server_reply, "0") == 0)
    {
        puts("Krustys is closed.");
        return 0;
    }
    puts(server_reply);
    printf("\n");

    // keep communicating with server
    while (max_burger > 0)
    {
        printf("Ordering a burger\n");
        if (send(sock, message, strlen(message), 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
        memset(server_reply, 0, sizeof(server_reply));
        // Receive a reply from the server
        if (recv(sock, server_reply, 2000, 0) < 0)
        {
            puts("recv failed");
            break;
        }

        puts("Server reply :");
        if (strcmp(server_reply, "0") == 0)
        {
            puts(out_of_stock);
            break;
        }
        else
        {
            puts(server_reply);
            memset(server_reply, 0, sizeof(server_reply));
            if (recv(sock, server_reply, 2000, 0) < 0)
            {
                puts("recv failed");
                break;
            }
            if (strcmp(server_reply, "1") == 0)
            {
                count++;
                max_burger--;
                printf("Eating.... Burger Number: %d, I can eat %d more\n", count, max_burger);
                time_to_eat = rand() % (3 + 1 - 1) + 1;
                sleep(time_to_eat);
                printf("Finished eating in %d seconds\n", time_to_eat);
            }
        }
    }
    close(sock);
    puts("I am leaving.");
    return 0;
}
int create_socket()
{
    int sock;
    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    return sock;
}
int connection(int sock, char *ip)
{
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT_NUMBER);
    int count = 3; // Attempts
    while (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        count--;
        perror("connect failed. Error");
        if (count == 0)
            return 1;
        puts("Attempting to reconnect");
        sleep(2);
    }
    return 0;
}
int main(int argc, char *argv[])
{
    srand(time(NULL));
    if (argc > 1)
    {
        ips = argv[1];
        if (argc > 2)
        {
            PORT_NUMBER = atoi(argv[2]);
            if (argc > 3)
                max_burger = atoi(argv[3]);
        }
    }

    int sock = create_socket();
    if (connection(sock, ips) == 1)
        return 1;
    if (action(sock) == 1)
        return 1;
    return 0;
}