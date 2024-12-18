#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <sys/uio.h>
#include <sys/wait.h>

#define true 1;
#define false 0; 

long get_file_size(FILE* file){
    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    return size;
}

void writeProcessMemory(int pid, long* buffer, long buffer_size,unsigned long address){
    int entry = 0x1000;
    ptrace(PTRACE_ATTACH, pid, NULL,NULL);
    wait(NULL);
    for(int i = 0; i < buffer_size; i += 8,buffer++){
       ptrace(PTRACE_POKETEXT, pid, address + i, *buffer); 
    }   
    
    // Get old register state.
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, pid, NULL, &regs);
 
    // Modify the instruction pointer to point to our payload
    regs.rip = address + entry + 2;

    // Modify the registers
    ptrace(PTRACE_SETREGS, pid, NULL, &regs); 
    ptrace(PTRACE_CONT, pid, NULL,NULL);
}

unsigned long findAddress(char* mapping, long buffer_size){
    int line_length = 0;
    char* copy = mapping;
    while(1){
        char* line_break = strstr(copy,"\n");
        if(line_break){
            line_length = line_break - copy;
            char* sad = (char*) malloc(line_length+1);
            if(sad){
                memset(sad,0,line_length);
                strncpy(sad,copy,line_length);
                char* is_rwx = strstr(sad,"rwx");
                if(is_rwx){
                    char* minus = strstr(copy,"-");
                    char* space = strstr(copy," ");
                    if(minus && space){
                        int start_size = minus - copy;
                        int end_size = space - minus - 1;
                        char* start = (char*) malloc(start_size + 1);
                        char* end = (char*) malloc(end_size + 1);
                        if(start && end){
                            memset(start,0,start_size + 1);
                            memset(end,0,end_size + 1);
                            strncpy(start,copy,start_size);
                            strncpy(end,copy+start_size+1,end_size);
                            printf("%s\n",start);
                            printf("%s\n",end);
                            unsigned long start_addr = strtoul(start,NULL,16);
                            unsigned long end_addr = strtoul(end,NULL,16);
                            if(end_addr - start_addr >= buffer_size){
                                return start_addr;
                            }
                            memset(start,0,start_size);
                            memset(end,0,end_size);
                            free(start);
                            free(end);
                            start = NULL;
                            end = NULL;
                        }
                    }
                }
                memset(sad,0,line_length);
                free(sad);
                sad = NULL;
                copy = line_break + 1;
                if(!copy[0]){
                    break;
                }
            }

        }
    }
    return 0;
}



int main(){
    int pid = 3674;
    FILE* file = fopen("a.out","r");
    if(file){
        long size = get_file_size(file);
        char* input = (char*) malloc(size);
        if(input){
            memset(input,0,size);
            int read = fread(input, 1, size, file);
            if(read){
                char proc[21];
                proc[20] = 0;
                sprintf(proc,"cat /proc/%d/maps", pid);
                FILE* maps = popen(proc,"r");
                if(maps){
                    printf("%s\n",proc);
                    char* map_file = (char*) malloc(65536);
                    if(map_file){
                        memset(map_file,0,65536);
                        read = fread(map_file,1,65535,maps);
                        if(read){
                            printf("%s\n",map_file);
                            unsigned long addr = findAddress(map_file,size);
                            if(addr){
                                printf("found address: %lx\n",addr);      
                                writeProcessMemory(pid,(long*)input,size,addr);   
                            }
                        }
                        else{
                            perror("sad");
                        }
                    }                    
                }
            }
            memset(input, 0, size);
            free(input);
            input = NULL;
        }
    }

}
