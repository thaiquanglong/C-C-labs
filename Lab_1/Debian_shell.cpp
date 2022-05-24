/*
Coded for the Kali/Debian Linux/GNU distrubution
*/
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <cstdlib>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;

char *remove_quote(char *str, int end)
{
    str[end - 1] = '\0';
    str = str + 1;
    return str;
}
char *strmbtok(char *input, char *delimit, char *openblock, char *closeblock)
{
    static char *token = NULL;
    char *lead = NULL;
    char *block = NULL;
    int iBlock = 0;
    int iBlockIndex = 0;

    if (input != NULL)
    {
        token = input;
        lead = input;
    }
    else
    {
        lead = token;
        if (*token == '\0')
        {
            lead = NULL;
        }
    }

    while (*token != '\0')
    {
        if (iBlock)
        {
            if (closeblock[iBlockIndex] == *token)
            {
                iBlock = 0;
            }
            token++;
            continue;
        }
        if ((block = strchr(openblock, *token)) != NULL)
        {
            iBlock = 1;
            iBlockIndex = block - openblock;
            token++;
            continue;
        }
        if (strchr(delimit, *token) != NULL)
        {
            *token = '\0';
            token++;
            break;
        }
        token++;
    }
    return lead;
}
void commandToken(char *command, char **arrCommand)
{
    char *temp;
    char acOpen[] = {"\"\'[<{"};
    char acClose[] = {"\"\']>}"};
    char delimiter[] = {" "};
    temp = strmbtok(command, delimiter, acOpen, acClose);
    int k = 0;
    while (temp != NULL)
    {
        if (temp[0] == '\'' || temp[0] == '\"')
        {
            temp = remove_quote(temp, strlen(temp));
        }
        arrCommand[k] = temp;
        temp = strmbtok(NULL, delimiter, acOpen, acClose);
        k++;
    }
}

int myExecvp(char **argv)
{
    pid_t pid;
    int status;
    int childStatus;
    pid = fork();
    if (pid == 0)
    {
        childStatus = execvp(*argv, argv);
        if (childStatus < 0)
        {
            cout << "ERROR:wrong input" << endl;
        }
        exit(0);
    }
    else if (pid < 0)
    {
        cout << "somthing went wrong!" << endl;
    }
    return waitpid(pid, &status, 0);
}
int cd(char *path)
{
    if (*path != '/' && *path != '.')
    {
        char tmp[256];
        getcwd(tmp, 256);
        string temp1;
        temp1 += tmp;
        temp1 += "/";
        temp1 += path;
        const char *cptr = temp1.c_str();
        strncpy((char *)cptr, temp1.c_str(), temp1.size());
        return chdir(cptr);
    }
    return chdir(path);
}
void print_manual()
{
    cout << "Manual" << endl;
    cout << endl;
    printf("exit [n]: %s \n", "terminates the shell, either by calling the exit()");
    cout << endl;
    printf("prompt  [new_prompt]: %s \n %s \n",
           "will change the current shell prompt to the new_prompt.  The  default  prompt  should  be  cwushell>",
           "                    Typing  prompt  should  restore the default shell prompt");
    cout << endl;
    printf("cpuinfo –switch: %s \n %s \n %s \n %s \n",
           "will print on the screen different cpu related information based on the switch. ",
           "                -c – will print the cpu clock (e.g. 3.2 GHz)",
           "                -t –will print the cpu type (e.g. Intel)",
           "                -n – will print the number of cores (e.g. 8).");
    cout << endl;
    printf("meminfo -switch: %s \n %s \n %s \n %s \n",
           "will print on the screen different memory related information based on the switch.",
           "                -t – will print the total RAM memory available in the system in bytes",
           "                -u –will print the used RAM memory and",
           "                -c – will print the size of the L2 cache/core in bytes");
    cout << endl;
    printf("%s \n",
           "all other existing shell commands (internal and external Linux commands e.g. ls, cat, pwd, etc.)");
    cout << endl;
}
void print_cpuinfo()
{
    cout << endl;
    printf("cpuinfo –switch: %s \n %s \n %s \n %s \n",
           "will print on the screen different cpu related information based on the switch. ",
           "                -c – will print the cpu clock (e.g. 3.2 GHz)",
           "                -t –will print the cpu type (e.g. Intel)",
           "                -n – will print the number of cores (e.g. 8).");
    cout << endl;
}
void print_meminfo()
{
    cout << endl;
    printf("meminfo -switch: %s \n %s \n %s \n %s \n",
           "will print on the screen different memory related information based on the switch.",
           "                -t – will print the total RAM memory available in the system in bytes",
           "                -u –will print the used RAM memory and",
           "                -c – will print the size of the L2 cache/core in bytes");
    cout << endl;
}
void commands(char **argv)
{
    char **temp = new char *[3];
    if (strcmp(*argv, "cpuinfo") == 0)
    {
        temp[0] = (char *)"grep";
        temp[2] = (char *)"/proc/cpuinfo";
        temp[3] = (char *)NULL;
        if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "-help") == 0))
        {
            print_cpuinfo();
            delete[] temp;
            return;
        }
        if (strcmp(argv[1], "-c") == 0)
        {
            temp[1] = (char *)"MHz";
        }
        else if (strcmp(argv[1], "-t") == 0)
        {
            temp[1] = (char *)"model name";
        }
        else if (strcmp(argv[1], "-n") == 0)
        {
            temp[1] = (char *)"cpu cores";
        }
    }
    if (strcmp(*argv, "meminfo") == 0)
    {
        temp[0] = (char *)"grep";
        temp[2] = (char *)"/proc/meminfo";
        if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "-help") == 0))
        {
            print_meminfo();
            delete[] temp;
            return;
        }
        if (strcmp(argv[1], "-t") == 0)
        {
            temp[1] = (char *)"MemTotal";
            temp[3] = (char *)NULL;
        }
        else if (strcmp(argv[1], "-u") == 0)
        {
            temp[0] = (char *)"free";
            temp[1] = (char *)"-b";
            temp[2] = (char *)NULL;
            myExecvp(temp);
            delete[] temp;
            return;
        }
        else if (strcmp(argv[1], "-c") == 0)
        {
            temp[0] = (char *)"echo";
            temp[1] = (char *)"L2 cache/core: ";
            temp[2] = (char *)NULL;
            myExecvp(temp);
            temp[0] = (char *)"cat";
            temp[1] = (char *)"/sys/devices/system/cpu/cpu0/cache/index2/size";
            temp[2] = (char *)NULL;
            myExecvp(temp);
            delete[] temp;
            return;
        }
    }
    myExecvp(temp);
    delete[] temp;
}
int main()
{
    char **arrCommand = new char *[50];
    char str[250];
    string prompt = "cwushell-> ";
    while (true)
    {
        delete[] arrCommand;
        arrCommand = new char *[50];
        std::cout << prompt;
        if (cin.getline(str, 250) && strlen(str) != 0)
        {
            string tempStr(str);
            commandToken(str, arrCommand);
            if (tempStr == "exit")
                exit(0);
            if (tempStr == "manual")
            {
                print_manual();
                continue;
            }
            if (tempStr == "cpuinfo")
            {
                print_cpuinfo();
                continue;
            }
            if (tempStr == "meminfo")
            {
                print_meminfo();
                continue;
            }
            if (tempStr == "prompt")
            {
                prompt = "cwushell-> ";
                continue;
            }
            if (tempStr == "ls")
            {
                arrCommand[1] = (char *)NULL;
                myExecvp(arrCommand);
                continue;
            }
            if ((strcmp(arrCommand[0], "cpuinfo") == 0) || (strcmp(arrCommand[0], "meminfo") == 0))
            {
                commands(arrCommand);
                continue;
            }
            if (strcmp(arrCommand[0], "exit") == 0)
                exit(atoi(arrCommand[1]));
            if (strcmp(arrCommand[0], "cd") == 0)
            {
                if (cd(arrCommand[1]) < 0)
                {
                    perror(arrCommand[1]);
                }
                continue;
            }
            if (strcmp(arrCommand[0], "prompt") == 0)
            {
                prompt = arrCommand[1];
                prompt += "-> ";
                continue;
            }
            myExecvp(arrCommand);
        }
    }

    return 0;
}
