#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[]){
    if(argc != 2){
        fprintf(stderr,"This program only accept one parameter\n");
        return 1;
    }
    char* in = argv[1];
    char* error;
    long int pid = strtol(in,&error,10);
    if(*error != '\0')
    {
        fprintf(stderr, "Please only input number cause pid can only be a number\n");
        return 1;
    }
    if(pid  < 0){
        fprintf(stderr,"Pid should be bigger than 0\n");
    }
    char result_buf[5120];
    int fd = open("/proc/lab2/lab_info",O_RDWR);
    //int fd = open("./test.txt",O_RDWR);
    write(fd,argv[1],strlen(argv[1]));
    close(fd);
    int new_fd = open("/proc/lab2/lab_info", O_RDWR);
    read(new_fd,result_buf,5120);
    puts(result_buf);
    return 0;
}
