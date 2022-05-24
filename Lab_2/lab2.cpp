#include <stdio.h>     
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string>       
#include <iostream>     
#include <sstream>

void cull(int p, int read_d, int write_d) {
    int n;
    while (read(read_d, &n, sizeof(n))) {
        if (n % p != 0) {
            write(write_d, &n, sizeof(n));
        }
    }
}

int sink(int read_d, char* path) {
    int p, dup_write, fd;
    if (read(read_d, &p, sizeof(p))) {
        int cull_pipe[2];
        pipe(cull_pipe);

        if (fork()) {
            close(cull_pipe[1]);
            fd = open(path, O_WRONLY|O_TRUNC); 
            int out = dup(1);
            dup_write = dup2(fd,1);
            printf("%d\n", p);
            sink(cull_pipe[0], path);

        } else {
            close(cull_pipe[0]);
            cull(p, read_d, cull_pipe[1]);
            close(cull_pipe[1]);
        }
    }
    return dup_write;
}
int get_prime(char* path){
    int pd[2], fd, sz;
    int Pid;
    pipe(pd);

    if (Pid = fork()) {
        close(pd[1]);
        sink(pd[0], path);
    } else {
        close(pd[0]);
        printf("%s \n",path);
        fd = open(path, O_RDONLY); 
        if (fd < 0) { 
                perror("r1"); exit(1); 
        } 
        struct stat st;
        stat(path, &st);
        int size = st.st_size;
        char *c = (char *) calloc(size+1, sizeof(char));       
        sz = read(fd, c, size); 
        printf("called read(% d, c, %d). returned that"
                " %d bytes were read.\n", fd, sz, sz); 
        c[sz] = '\0'; 
        std::stringstream ss(c);
        std::string to;
        if (c != NULL){
                while(std::getline(ss,to,'\n')){
                        int num = std::stoi(to);
                        write(pd[1], &num, sizeof(int));
                }
        }
        close(pd[1]);
    }
    return wait(&Pid);
}
int main(int argc, char *argv[]) {
    int num = get_prime(argv[1]);
}