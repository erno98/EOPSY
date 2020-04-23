#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <signal.h>

#include <string.h>

#define NUM_CHILD 4
#define WITH_SIGNALS

// --------------------------- SIGNALS
#ifdef WITH_SIGNALS

int interrupted = 0; //global variable for notifying about the keyboard interrupt

void custom_interrupt(){
    printf("parent[%d]:\tReceived keyboard interrupt, terminating.\n", getpid());
    interrupted = 1;
}

void custom_terminate(){
    printf("child[%d]:\tReceived SIGTERM signal, terminating.\n", getpid());
}

#endif

// --------------------------- INFORMATION 

void print_child_status(int iteration){
    printf("parent[%d]:\tFork no. %d created: child[%d]\n",  getppid(), iteration+1, getpid());
}

void print_parent_status(){
    printf("parent[%d]:\tHas started\n", getpid());
}

void print_fork_error(int iteration){
    printf("parent[%d]:\tFailed to create fork no. %d. Exiting.\n", getpid(), iteration+1);
    // note, that here the id of the parent is the id of the process, since the fork failed
}

void child_process(){

    #ifdef WITH_SIGNALS
        // ignore the interrupt signal
        signal(SIGINT, SIG_IGN);
        signal(SIGTERM, custom_terminate);  
    #endif

        printf("child[%d]:\tProcess started forked from parent[%d]\n", getpid(),  getppid());
        unsigned int elapsed = sleep(10);
        if(elapsed != 0){
            printf("child[%d]:\tProcess interrupted\n", getpid());

        } else{
            printf("child[%d]:\tProcess completed\n", getpid());
        }

}

// --------------------------- USAGE

void terminate_children(pid_t *pids, int how_many){
    // function sending SIGTERM to given child processes
    // params:  pids -> pointer to array of pid_t with processes to terminate
    //          how_many -> number of child processes
    // returns: void
    for (int i=0; i<how_many; i++){
        // termination
        printf("child[%d]:\tReceived SIGTERM signal, terminating\n", pids[i]);
        kill(pids[i], SIGTERM);
    }

    printf("parent[%d]:\tTerminating children finished.\n", getpid());

}

// --------------------------- MAIN

int main(){

    #ifdef WITH_SIGNALS
    
        // ignore all signals
        for(int i=0; i<_NSIG; i++){
            signal(i+1, SIG_IGN);
        }
        // restoring default handler for SIGCHLD
        signal(SIGCHLD, SIG_DFL);  

        // custom keyboard interrput handler
        signal(SIGINT, custom_interrupt);  
    
    #endif

    pid_t fork_status;
    int i = 0;
    int children_num = 0;

    // print info about parent process before creating children
    print_parent_status();

    // array to hold the ids of succesfuly created child processes
    int child_processes[NUM_CHILD];

    do {

        #ifdef WITH_SIGNALS
            if(interrupted == 1){
                // interrupt occurred
                terminate_children(child_processes, i);
                printf("parent[%d]:\tInterrputed, process cancelled.\n", getpid());
                break;
            }
        #endif

        //create child process
        // debug purposes
        if(i==55){
            fork_status = -1;
        } else{
            fork_status = fork();
        }


        // do in child process
        if(fork_status == 0){
            // fork created correctly
            child_process();
            return 0;

        // do in parent process
        } else if(fork_status > 0){
            child_processes[i] = fork_status;
            ++children_num;
            sleep(1);

        // fork failure
        } else{
            print_fork_error(i);
            printf("parent[%d]:\tsending SIGTERM signal\n", getpid());
            terminate_children(child_processes, i);
            return 1;
        }

        i++;    

    } while ((fork_status != 0) && (i < NUM_CHILD));

    // number of the finished processes (not failed)
    int finished_processes_num = 0;

    // count of exit code of the process
    int exit_codes[children_num]; // array to store exit codes ! not displayed anywhere !
    int exit_codes_num = 0; // number of exit codes received
    int exit_code; // exit code to be received from wait


    while(finished_processes_num < children_num){

        int process_id = wait(&exit_code); // waiting for child to complete the process
        for(int i=0; i<children_num; i++){
            if(child_processes[i] == process_id){
                exit_codes_num++;
                exit_codes[i] = exit_code;
                break;
            }
        }

        ++finished_processes_num;

    }

    if(fork_status > 0){
        printf("parent[%d]:\t%d exit codes returned.\n", getpid(), exit_codes_num);
        printf("parent[%d]:\tNo more processes to synchronize.\n", getpid());
    }

    // restore the old service handlers
    #ifdef WITH_SIGNALS
        for(int i=0; i<_NSIG; i++){
            signal(i+1, SIG_DFL);
        }
    #endif

}
