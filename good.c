#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
  #define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(){
    printf("%d\n",getpid());
    char* sad = (char*) mmap(NULL, 20000, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    if(sad != -1){
        printf("%x\n", sad);
        while(1){
            printf("sad\n");
            sleep(1);
        }
    }else {
       handle_error("sad"); 
    }
}
