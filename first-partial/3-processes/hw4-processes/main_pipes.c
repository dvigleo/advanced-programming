#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/wait.h>
#include <string.h>

#define H 15

typedef struct {
    int pid;
    float average;
    char * histogram;
} proc_t;

float get_average(int, int);

void create_histogram(proc_t *, int);
void print_table(proc_t *, int);

void print_help();
void free_memory(proc_t *, int);

int max = 0;

int main(int argc, char * const * argv) {
    /* Obtaining the number of processes from the command line */
    char * input = NULL;
    int argument, start = 0, index; // variables for short arguments
    int n = 0; // number of processes to create
    while((argument = getopt (argc, argv, "n:h")) != -1)
    
    switch(argument) {
        case 'n':
            input = optarg;
            if(isdigit(*input)) {
                n = atoi(input);
                start = 1;
            } else {
                fprintf(stderr, "Opción -%s require un número entero como argumento\n", optarg);
            }
            break;
        case 'h':
            print_help();
            break;
        case '?':
            if(optopt == 'n')
                fprintf(stderr, "Opción -%c require un número entero como argumento\n", optopt);
            else if(isprint (optopt))
                fprintf(stderr, "Opción desconocida '-%c\n", optopt);
            else
                fprintf(stderr, "Opción desconocida '\\x%x'\n", optopt);
            return 1;
        default:
            abort();
    }
    
    for (index = optind; index < argc; index++)
        printf ("El argumento %s no es una opción válida \n", argv[index]);
    
    if(start == 1) {
        pid_t pid;
        proc_t * processes = (proc_t *) malloc(sizeof(proc_t) * n);
        
        int state;
        int fd[2];
        pipe(fd);
        int i = 0;
        int accum = i;
        
        while(i < n) {
            pid = fork();
            
            if(pid == -1) {
                printf("Error. %d procesos hijos creados\n", i);
            } else if(pid == 0) { 
                close(fd[0]);// Closing reading end
                float average = get_average(getppid(), getpid());
                printf("Soy el proceso hijo con PID = %d, mi promedio es = %.2f\n", getpid(), average);
                int pid = getpid();
                write(fd[1], &pid, sizeof(int));
                write(fd[1], &average, sizeof(float));
                close(fd[1]);
                return((int)average);
            } else {
                if (waitpid(pid, &state, 0) != -1) {
                    wait(NULL);
                    close(fd[1]); // Closing writing end;
                    int pid; 
                    float average;
                    read(fd[0], &pid, sizeof(int));
                    read(fd[0], &average, sizeof(float));
                    proc_t * proc = processes + i;
                    proc->pid = pid;
                    proc->average = average;
                    if(average > max)
                        max = average;
                    close(fd[0]);
                }
            }
            i++;
        }
        create_histogram(processes, n);
        print_table(processes, n);
        free_memory(processes, n);
    } 
    return 0;
}

float get_average(int ppid, int pid) {
    return (float)(ppid + pid) / 2;
}

void create_histogram(proc_t * processes, int n) {
    proc_t * p = processes;
    proc_t * final = processes + n;
    int n_chars = 0;
    char * c;
    for(; p < final; ++p) {
        n_chars = ((int)p->average * H) / max;
        p->histogram = (char *) malloc(sizeof(char) * n_chars);
        strcpy(p->histogram, "*");
        c = p->histogram + 1;
        for(; c < p->histogram + n_chars; ++c) {
            strcpy(c, "*");
        }
    }
}

void print_table(proc_t * processes, int n) {
    proc_t * p = processes;
    proc_t * final = processes + n;
    printf("\nPID Hijo\tPromedio\tHistograma\n");
    for(; p < final; ++p) {
        printf("%d\t\t %.2f\t\t%s\n", p->pid, p->average, p->histogram);
    }
}

void print_help() {
    printf("\nUse: ./a.out [-n value] [-h]\n");
    printf("Opciones:\n");
    printf("\t-n : Crear n procesos indicados como valor entero\n\th : Ayuda\n");
}

void free_memory(proc_t * processes, int n) {
    proc_t * p = processes;
    proc_t * final = processes + n;
    for(; p < final; ++p) {
        p->pid = 0;
        p->average = 0;
        free(p->histogram);
    }    
    free(processes);
    printf("Memory freed correctly\n");
}