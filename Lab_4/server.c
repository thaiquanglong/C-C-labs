#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#define QUEUE_EMPTY INT_MIN

///// defining queue
typedef struct node
{
        int value;
        struct node *next;
} node;

typedef struct
{
        node *head;
        node *tail;
} queue;

void init_queue(queue *q)
{
        q->head = NULL;
        q->tail = NULL;
}

int enqueue(queue *q, int value)
{
        node *newnode = malloc(sizeof(node));
        if (newnode == NULL)
                return 1;

        newnode->value = value;
        newnode->next = NULL;

        if (q->tail != NULL)
        {
                q->tail->next = newnode;
        }
        q->tail = newnode;

        if (q->head == NULL)
        {
                q->head = newnode;
        }
        return 0;
}

int dequeue(queue *q)
{
        if (q->head == NULL)
                return QUEUE_EMPTY;

        node *tmp = q->head;

        int result = tmp->value;

        q->head = q->head->next;
        if (q->head == NULL)
        {
                q->tail = NULL;
        }

        free(tmp);

        return result;
}
////////

struct arg_struct
{
        pthread_t arg1;
        int *arg2;
};

// the thread function
void *connection_handler(void *);

pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t myMutex2 = PTHREAD_MUTEX_INITIALIZER;
queue order_queue;
int burger_to_make = 25;
int num_chef = 2;
int PORT_NUMBER = 54321;
int restau_open = 1;
int create_socket()
{
        int socket_desc;
        socket_desc = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_desc == -1)
        {
                printf("Could not create socket");
        }
        puts("Socket created");
        return socket_desc;
}
int bind_server(int sock)
{
        struct sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(PORT_NUMBER);
        if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
                // print the error message
                perror("bind failed. Error");
                return 1;
        }
        puts("bind done");
        return 0;
}
void *cooking(void *argp)
{
        int num = *((int *)argp);
        int order_number;
        int time_to_cook = 0;
        printf("Chef Number %d clocked in\n", num);
        while ((order_number = dequeue(&order_queue)) != QUEUE_EMPTY || burger_to_make > 0)
        {
                if (order_number != QUEUE_EMPTY)
                {
                        printf("Chef Number %d is cooking for customer ID: %d\n", num, order_number);
                        time_to_cook = rand() % (4 + 1 - 2) + 2;
                        sleep(time_to_cook); // cook burger
                        write(order_number, "1", strlen("1"));
                        printf("Chef Number %d finished cooking in %d seconds\n", num, time_to_cook);
                        printf("Chef Number %d deliver to customer ID: %d\n", num, order_number);
                }
        }
        restau_open = 0;
}
void *create_chef(void *argp)
{
        int size = *((int *)argp);
        pthread_t *p1 = malloc(sizeof(pthread_t) * size);

        struct arg_struct *arg = malloc(sizeof(struct arg_struct) * size);
        int *num = malloc(sizeof(int) * size);
        for (int i = 0; i < size; i++)
        {
                num[i] = i + 1;
                pthread_create(&p1[i], NULL, &cooking, (void *)&num[i]);
        }
        for (int i = 0; i < size; i++)
        {
                pthread_join(p1[i], NULL);
        }
}
int create_threads(int socket_desc)
{
        struct sockaddr_in client;
        int client_sock, c, *new_sock;

        // Accept and incoming connection
        puts("Waiting for incoming connections...");
        c = sizeof(struct sockaddr_in);
        while ((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c)))
        {
                puts("Connection accepted");
                pthread_t sniffer_thread;
                new_sock = malloc(1);
                *new_sock = client_sock;
                struct arg_struct args1;
                args1.arg1 = sniffer_thread;
                args1.arg2 = new_sock;
                if (pthread_create(&sniffer_thread, NULL, connection_handler, (void *)&args1) < 0)
                {
                        perror("could not create thread");
                        return 1;
                }

                puts("Handler assigned");
        }

        if (client_sock < 0)
        {
                perror("accept failed");
                return 1;
        }
        return 0;
}
/*
 * This will handle connection for each client
 * */
void *connection_handler(void *argp)
{
        struct arg_struct *args = argp;
        // Get the socket descriptor
        int *socket_desc = args->arg2;
        int sock = *socket_desc;
        pthread_t sniffer_thread = args->arg1;
        int read_size;
        char client_message[2000];
        char *order_confirmation = "Order received.";
        // Send some messages to the client
        if (restau_open != 1)
        {
                write(sock, "0", strlen("0"));
                if (restau_open == 0)
                {
                        puts("We ran out of burgers!!!");
                }
                printf("Customer number %d left.\n", sock);
                fflush(stdout);
                return 0;
        }
        char *message = "Welcome to Krustys! I am your waiter\nWould you like some burgers?";
        write(sock, message, strlen(message));

        // Receive a message from client
        memset(client_message, 0, sizeof(client_message));
        while ((read_size = recv(sock, client_message, 2000, 0)) > 0)
        {
                // Send the message back to client
                if (strcmp(client_message, "1") == 0)
                {
                        pthread_mutex_lock(&myMutex);
                        if (burger_to_make > 0)
                        {
                                write(sock, order_confirmation, strlen(order_confirmation));
                                burger_to_make--;
                                enqueue(&order_queue, sock);
                                pthread_mutex_unlock(&myMutex);
                        }
                        else
                        {
                                pthread_mutex_unlock(&myMutex);
                                write(sock, "0", strlen("0"));
                        }
                }
                memset(client_message, 0, sizeof(client_message));
        }

        if (read_size == 0)
        {
                if (restau_open == 0)
                {
                        puts("We ran out of burgers!!!");
                }
                printf("Customer number %d left.\n", sock);
                fflush(stdout);
        }
        else if (read_size == -1)
        {
                perror("recv failed");
        }

        // Free the socket pointer
        free(socket_desc);
        pthread_join(sniffer_thread, NULL);
        return 0;
}

int main(int argc, char *argv[])
{
        srand(time(NULL));
        if (argc > 1)
        {
                burger_to_make = atoi(argv[1]);
                if (argc > 2)
                {
                        num_chef = atoi(argv[2]);
                        if (argc > 3)
                                PORT_NUMBER = atoi(argv[3]);
                }
        }
        // make queue for orders
        init_queue(&order_queue);

        // Create socket
        int socket_desc = create_socket();

        if (bind_server(socket_desc) == 1)
                return 1;

        // Listen
        listen(socket_desc, 10);
        pthread_t chefs;
        if (pthread_create(&chefs, NULL, &create_chef, (void *)&num_chef) != 0)
        {
                return 1;
        }
        if (create_threads(socket_desc) == 1)
                return 1;

        if (pthread_join(chefs, NULL) != 0)
        {
                return 2;
        }
        return 0;
}
