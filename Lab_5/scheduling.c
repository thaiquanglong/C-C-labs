#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#define PROCESS_NAME_SIZE 24
#define QUEUE_EMPTY NULL

#pragma pack(push, 1)
typedef struct _PCB
{
        char Priority;
        char Name[PROCESS_NAME_SIZE];
        int ProcessID;
        char IsActive;
        int CPUBurstTime;
        int MemoryBase;
        long MemoryLimit;
        int NoOfFiles;
} PCB;
#pragma pack(pop)
///// defining queue
typedef struct node
{
        PCB *value;
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

int enqueue(queue *q, PCB *value)
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

PCB *dequeue(queue *q)
{
        if (q->head == NULL)
                return QUEUE_EMPTY;

        node *tmp = q->head;

        PCB *result = tmp->value;

        q->head = q->head->next;
        if (q->head == NULL)
        {
                q->tail = NULL;
        }

        free(tmp);

        return result;
}
////////
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
int *algros;
int num_cpu;
float *load;
int *num_proc_each;
PCB **PCB_each;
queue *ready_queue;
int fixed_time = 10;
int *check_queue;
int running = 1;

int GetNumProcesses(char *sFileName, int *nSize)
{
        FILE *pFile = NULL;
        if ((pFile = fopen(sFileName, "rb+")) == NULL)
        {
                printf("Error in opening the file:%s\n", sFileName);
                return -1;
        }
        else
        {
                // calculate the size of the file
                fseek(pFile, 0, SEEK_END);
                *nSize = ftell(pFile) / 50;
                fclose(pFile);
                return 0;
        }
}

//-----------------------------------------
// Read the content of a bin file into an array
//-----------------------------------------
int GetProcesses(char *sFileName, PCB *process, int index)
{
        FILE *pFile = NULL;
        if ((pFile = fopen(sFileName, "rb+")) == NULL)
        {
                printf("Error in opening the file:%s\n", sFileName);
                return -1;
        }
        else
        {
                fseek(pFile, index * 50, SEEK_SET);
                fread(&process->Priority, sizeof(process->Priority), 1, pFile);
                fread(&process->Name, sizeof(process->Name), 1, pFile);
                fread(&process->ProcessID, sizeof(process->ProcessID), 1, pFile);
                fread(&process->IsActive, sizeof(process->IsActive), 1, pFile);
                fread(&process->CPUBurstTime, sizeof(process->CPUBurstTime), 1, pFile);
                fread(&process->MemoryBase, sizeof(process->MemoryBase), 1, pFile);
                fread(&process->MemoryLimit, sizeof(process->MemoryLimit), 1, pFile);
                fread(&process->NoOfFiles, sizeof(process->NoOfFiles), 1, pFile);
                return 0;
        }
}

//-----------------------------------------
// print the content of a process
//-----------------------------------------
int PrintProcess(PCB *Process)
{
        printf("Priority: %d \n", Process->Priority);
        printf("Name: %s \n", Process->Name);
        printf("ProcessID: %d \n", Process->ProcessID);
        printf("IsActive: %d \n", Process->IsActive);
        printf("CPUBurstTime: %d \n", Process->CPUBurstTime);
        printf("MemoryBase: %d \n", Process->MemoryBase);
        printf("MemoryLimit:%d \n", Process->MemoryLimit);
        printf("NoOfFiles: %d \n", Process->NoOfFiles);
        return 0;
}

long GetMemory(PCB **Process, int size)
{
        long memory = 0;
        for (int i = 0; i < size; i++)
                memory += (*Process)[i].MemoryLimit;
        return memory;
}

int GetFiles(PCB **Process, int size)
{
        int files = 0;
        for (int i = 0; i < size; i++)
                files += (*Process)[i].NoOfFiles;
        return files;
}

int GetAllProcess(char *sFileName, PCB **Process, int size)
{
        for (int i = 0; i < size; i++)
        {
                if (GetProcesses(sFileName, &((*Process)[i]), i) == -1)
                        return -1;
        }
        return 0;
}

int comparator_priority(const void *p, const void *q)
{
        return ((PCB *)q)->Priority - ((PCB *)p)->Priority;
}

queue Get_priority_queue(PCB *arr, int size)
{
        queue ready;
        qsort(arr, size, sizeof(PCB), comparator_priority);
        init_queue(&ready);
        for (int i = 0; i < size; i++)
                enqueue(&ready, &(arr[i]));
        return ready;
}

int comparator_CPU_burst(const void *p, const void *q)
{
        return ((PCB *)p)->CPUBurstTime - ((PCB *)q)->CPUBurstTime;
}

queue Get_burst_queue(PCB *arr, int size)
{
        queue ready;
        qsort(arr, size, sizeof(PCB), comparator_CPU_burst);
        init_queue(&ready);
        for (int i = 0; i < size; i++)
                enqueue(&ready, &(arr[i]));
        return ready;
}

queue Get_FCFS_queue(PCB *arr, int size)
{
        queue ready;
        init_queue(&ready);
        for (int i = 0; i < size; i++)
                enqueue(&ready, &(arr[i]));
        return ready;
}
void normal_execution(int num, int queue_index)
{
        PCB *process;
        if (algros[queue_index] != 3)
        {
                while ((process = dequeue(&ready_queue[queue_index])) != QUEUE_EMPTY)
                {
                        printf("CPU %d executing: %s, Prority: %d\n", num, process->Name, process->Priority);

                        usleep((process->CPUBurstTime) * 100000);
                        printf("CPU %d finished executing: %s in %d miliseconds\n", num, process->Name, process->CPUBurstTime * 100);
                }
        }
        else
        {
                int try = num_cpu - 2;
        LOOP:
                while ((process = dequeue(&ready_queue[queue_index])) != QUEUE_EMPTY)
                {
                        try = num_cpu - 2;
                        int sleeptime = fixed_time;
                        if ((process->CPUBurstTime) < sleeptime)
                                sleeptime = process->CPUBurstTime;
                        printf("CPU %d executing: %s, Prority: %d\n", num, process->Name, process->Priority);
                        usleep((sleeptime)*100000);
                        process->CPUBurstTime -= sleeptime;
                        if (process->CPUBurstTime > 0)
                        {
                                enqueue(&ready_queue[queue_index], process);
                                printf("Process: %s ran for %d miliseconds on cpu %d and pushed back to queue\n", process->Name, sleeptime * 100, num);
                        }
                        if (process->CPUBurstTime <= 0)
                                printf("CPU %d finished executing: %s in %d miliseconds\n", num, process->Name, sleeptime * 100);
                }
                if (try > 0)
                {
                        usleep((fixed_time)*100000);
                        try -= 1;
                        goto LOOP;
                }
        }
}

void balance_execution(int num, int queue_index)
{
        PCB *process;
        if (algros[queue_index] != 3)
        {
                if ((process = dequeue(&ready_queue[queue_index])) != QUEUE_EMPTY)
                {
                        printf("CPU %d is helping CPU %d\n", num, queue_index);
                        printf("CPU %d executing: %s, Prority: %d\n", num, process->Name, process->Priority);

                        usleep((process->CPUBurstTime) * 100000);
                        printf("CPU %d finished executing: %s in %d miliseconds\n", num, process->Name, process->CPUBurstTime * 100);
                }
        }
        else
        {
                if ((process = dequeue(&ready_queue[queue_index])) != QUEUE_EMPTY)
                {
                        printf("CPU %d is helping CPU %d\n", num, queue_index);
                        int sleeptime = fixed_time;
                        if ((process->CPUBurstTime) < sleeptime)
                                sleeptime = process->CPUBurstTime;
                        printf("CPU %d executing: %s, Prority: %d\n", num, process->Name, process->Priority);
                        usleep((sleeptime)*100000);
                        process->CPUBurstTime -= sleeptime;
                        if (process->CPUBurstTime > 0)
                        {
                                enqueue(&ready_queue[queue_index], process);
                                printf("Process: %s ran for %d miliseconds on cpu %d and pushed back to queue\n", process->Name, sleeptime * 100, num);
                        }
                        if (process->CPUBurstTime <= 0)
                                printf("CPU %d finished executing: %s in %d miliseconds\n", num, process->Name, sleeptime * 100);
                }
        }
}

void *executing(void *argp)
{
        int num = *((int *)argp);
        normal_execution(num, num);
        check_queue[num] = 0;
        puts("///////");
        printf("CPU %d is load balancing\n", num);
        puts("////////");
        while (running == 1)
        {
                for (int i = 0; i < num_cpu; i++)
                {
                        int temp = check_queue[i];
                        if (temp == 1)
                        {
                                balance_execution(num, i);
                        }
                }
        }
}

void *create_cpus(void *argp)
{
        int size = *((int *)argp);
        pthread_t *p1 = malloc(sizeof(pthread_t) * size);

        int *num = malloc(sizeof(int) * size);
        for (int i = size - 1; i >= 0; i--)
        {
                num[i] = i;
                pthread_create(&p1[i], NULL, &executing, (void *)&num[i]);
        }
        for (int i = 0; i < size; i++)
        {
                pthread_join(p1[i], NULL);
        }
}
PCB **Get_Arrays_PCBs(PCB *temp, int num_cpu, int size)
{

        PCB **array_PCB = (PCB **)malloc(num_cpu * sizeof(PCB *));
        int num = 0;
        for (int i = 0; i < num_cpu; i++)
        {
                num_proc_each[i] = (int)(size * load[i]);
                array_PCB[i] = (PCB *)malloc(num_proc_each[i] * sizeof(PCB));
                for (int j = num, index = 0; j < (num + num_proc_each[i]); j++, index++)
                {
                        array_PCB[i][index] = temp[j];
                }
                num += num_proc_each[i];
        }
        return array_PCB;
}

void set_up_processor(int num_proc)
{
        for (int index = 0; index < num_proc; index++)
        {
                if (algros[index] == 1)
                {
                        ready_queue[index] = Get_priority_queue(PCB_each[index], num_proc_each[index]);
                        printf("CPU %d loaded: Priority Scheduling\n", index);
                }
                if (algros[index] == 2)
                {
                        ready_queue[index] = Get_burst_queue(PCB_each[index], num_proc_each[index]);
                        printf("CPU %d loaded: Shortest Job First\n", index);
                }
                if (algros[index] == 3)
                {
                        ready_queue[index] = Get_FCFS_queue(PCB_each[index], num_proc_each[index]);
                        printf("CPU %d loaded: Round Robin\n", index);
                }
                if (algros[index] == 4)
                {
                        ready_queue[index] = Get_FCFS_queue(PCB_each[index], num_proc_each[index]);
                        printf("CPU %d loaded: First Come First Served\n", index);
                }
        }
}
void *supervisor(void *vargp)
{
        int loop = 1;
        while (loop == 1)
        {
                int temp;
                pthread_mutex_lock(&myMutex);
                for (int i = 0; i < num_cpu; i++)
                {
                        temp = check_queue[i];
                        if (temp == 1)
                                break;
                        if (i == (num_cpu - 1) && temp == 0)
                                loop = 0;
                }
                pthread_mutex_unlock(&myMutex);
        }
        running = 0;
}
int set_up(int argc, char *argv[])
{
        num_cpu = (argc / 2) - 1;
        float check = 0;
        // get the
        algros = (int *)malloc(sizeof(int) * num_cpu);
        load = malloc(sizeof(float) * num_cpu);
        num_proc_each = (int *)malloc(sizeof(int) * num_cpu);
        for (int i = 0; i < num_cpu; i++)
        {
                algros[i] = atoi(argv[2 + 2 * i]);
                load[i] = atof(argv[3 + 2 * i]);
                if (algros[i] == 0 || algros[i] > 4)
                {
                        puts("Scheduling algrorithim goes from 1 to 4");
                        puts("1-Priority Scheduling");
                        puts("2-Shortest Job First");
                        puts("3-Round Robin");
                        puts("4-First Come First Served");
                        return -1;
                }
                if (algros[i] < 0 || load[i] < 0.0)
                {
                        puts("The program dont accept negatives");
                        return -1;
                }
                check += load[i];
        }
        if (check > 1.00000011920928955078125)
        {
                printf("\n sum of load has to be smaller or equal to 1.0 \n", argv[0]);
                return -1;
        }
        return 0;
}
//-----------------------------------------
// main function
//-----------------------------------------
int main(int argc, char *argv[])
{
        if (argc < 2 || argc % 2 != 0)
        {
                printf("\n Usage: %s <filename> <algro> <load> \n", argv[0]);
                return 1;
        }
        if (argc > 2)
        {
                if (set_up(argc, argv) != 0)
                        return 1;
        }
        if (argc == 2)
        {
                char *c_arr[4];
                char *arg1 = "4";
                char *arg2 = "1";
                c_arr[2] = arg1;
                c_arr[3] = arg2;
                if (set_up(4, c_arr) != 0)
                        return 1;
        }

        char *sFileName = argv[1];
        int size = 0;
        int memory;
        int files;
        // get size
        if (GetNumProcesses(sFileName, &size) == -1)
                return 1;

        // get processes from file
        PCB *temp = malloc(sizeof(PCB) * size);
        if (GetAllProcess(sFileName, &temp, size) == -1)
                return 1;
        PCB_each = Get_Arrays_PCBs(temp, num_cpu, size);
        // get memrory
        memory = GetMemory(&temp, size);
        files = GetFiles(&temp, size);
        ready_queue = malloc(sizeof(queue) * num_cpu);
        set_up_processor(num_cpu);
        check_queue = (int *)malloc(sizeof(int) * num_cpu);
        for (int i = 0; i < num_cpu; i++)
                check_queue[i] = 1;

        printf("Number of processes: %d\n", size);
        printf("Memory Allocated: %d \n", memory);
        printf("Number of files: %d \n", files);
        puts("");

        pthread_t super;
        pthread_t cpus;
        if (pthread_create(&super, NULL, &supervisor, NULL) != 0)
        {
                return 1;
        }
        if (pthread_create(&cpus, NULL, &create_cpus, (void *)&num_cpu) != 0)
        {
                return 2;
        }
        if (pthread_join(super, NULL) != 0)
        {
                return 3;
        }
        if (pthread_join(cpus, NULL) != 0)
        {
                return 4;
        }
        puts("All processes executed");
        return 0;
}
