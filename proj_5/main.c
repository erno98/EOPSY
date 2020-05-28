#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

// --- CONFIG ---

// Female Barbers
#define N1 4

// Male Barbers
#define N2 4

// Unisex Barbers
#define N3 4

// Waiting Room Chairs
#define M 4

// Clients
#define NC 20

// Timings
#define HAIRCUT_TIME 3
#define HAIRGROWTH_TIME 10

// --- READABILITY ---
#define FEMALE 0
#define MALE 1
#define UNISEX 2

// --- KEYS ---
#define KEY_CLIENT 0x1111
#define KEY_BARBER 0x1122
#define KEY_WROOM  0x1133
#define KEY_MEM    0x1144

// semaphore handling
#define UP(SEMN)   {SEMN, +1, SEM_UNDO}
#define DOWN(SEMN) {SEMN, -1, SEM_UNDO}

// structure for holding the semaphores in shared memory
struct semaphores
{
    int clients;
    int barbers;
    int waiting_room;
};

void semaphores_get(struct semaphores* sems);
void sem_wroom_down();
void sem_wroom_up();
void sem_client_down(int gender);
void sem_client_up(int gender);
void sem_barber_down(int gender);
void sem_barber_up(int gender);

// structure for shared memory handling
struct shared_memory
{
    int seats_taken;
    int waiting[2];
};
struct shared_memory* memory_attach();
void memory_detach(struct shared_memory* mem);

// miscellaneous
#define PERROR_EXIT(STR) {perror(STR);exit(1);}
void interrupt_handler();
void release_ids();
const char* sex[3] = {"female", "male", "unisex"};

// process handling
pid_t* children;
int children_amount;
void kill_children();

// main functions
void barber(int gender, int id);
void client(int gender, int id);

int main(int argc, char** argv)
{
    printf("Barber process started\n");

    // create shared memory
    int shmid = shmget(KEY_MEM, sizeof(struct shared_memory), 0660 | IPC_CREAT);
    if(shmid < 0)
        PERROR_EXIT("shmget error");

    struct shared_memory* shared = shmat(shmid, NULL, 0);

    if(shared == (void*) -1)
        PERROR_EXIT("shmat error");

    shared->seats_taken = 0;
    shared->waiting[FEMALE] = 0;
    shared->waiting[MALE] = 0;

    // create semaphores
    struct semaphores sems;
    sems.clients      = semget(KEY_CLIENT, 2, 0660 | IPC_CREAT);
    sems.barbers      = semget(KEY_BARBER, 3, 0660 | IPC_CREAT);
    sems.waiting_room = semget(KEY_WROOM,  1, 0660 | IPC_CREAT);

    if(sems.clients < 0 || sems.barbers < 0 || sems.waiting_room < 0)
        PERROR_EXIT("semget error");

    // initialize the semaphores
    union semun
    {
        int val;
        ushort *array;
    } arg;

    // waiting room mutex
    arg.val = 1;
    if(semctl(sems.waiting_room, 0, SETVAL, arg) < 0)
        PERROR_EXIT("semctl error for waiting room mutex");

    // barbers
    arg.val = N1;
    if(semctl(sems.barbers, FEMALE, SETVAL, arg) < 0)
        PERROR_EXIT("semctl error when setting female barbers [N1] ");

    arg.val = N2;
    if(semctl(sems.barbers, MALE, SETVAL, arg) < 0)
        PERROR_EXIT("semctl error when setting male barbers [N2]");

    arg.val = N3;
    if(semctl(sems.barbers, UNISEX, SETVAL, arg) < 0)
        PERROR_EXIT("semctl error when setting unisex barbers [N3]");

    // clients
    arg.val = 0;
    if(semctl(sems.clients, FEMALE, SETVAL, arg) < 0)
        PERROR_EXIT("semctl setting creating female client");

    arg.val = 0;
    if(semctl(sems.clients, MALE, SETVAL, arg) < 0)
        PERROR_EXIT("semctl error setting male client");

    // making the barbers
    signal(SIGINT, interrupt_handler);
    printf("Barber creation beginning...\n");
    children = (pid_t*) malloc((N1+N2+N3+NC)*sizeof(pid_t));
    children_amount = 0;

    pid_t pid;
    int barbers = N1+N2+N3; // total number of free barbers
    for(int i=0; i<barbers; i++)
    {
        pid = fork();
        // fail
        if(pid < 0)
        {
            kill_children();
            exit(1);
        }
        // parent
        if(pid > 0)
        {
            children[children_amount] = pid;
            children_amount++;
        }
        // child
        if(pid == 0)
        {
            signal(SIGINT, SIG_DFL);
            if(i < N1)
                barber(FEMALE, i);
            else if(i < N1+N2)
                barber(MALE, i-N1);
            else
                barber(UNISEX, i-N1-N2);
            return 0;
        }
    }
    int gender;
    // creating clients
    for(int i=0; i<NC; i++)
    {
        pid = fork();
        // fail
        if(pid < 0)
        {
            kill_children();
            exit(1);
        }
        // parent
        if(pid > 0)
        {
            children[children_amount] = pid;
            children_amount++;
        }
        // child
        if(pid == 0)
        {
            srand(time(NULL));
            signal(SIGINT, SIG_DFL);
            while(1)
            {
                // randomize gender on entry
                gender = rand()%2;
                client(gender, i);
                sleep(HAIRGROWTH_TIME);
            }
            return 0;
        }
    }
    int finished = 0;
    int code;
    while(finished < children_amount)
    {
        pid = wait(&code);
        finished++;
    }
    
    printf("Closing the barber...\n");
    //detaching the memory
    kill_children();
    memory_detach(shared);
    release_ids();
    return 0;
}

void barber(int gender, int id)
{
    struct shared_memory* shared = memory_attach();
    printf("%s [barber] %d created: Ready for work\n", sex[gender], id);
    int client_gender = gender;
    while(1)
    {
        if(gender == UNISEX)
        {
            sem_wroom_down();
            client_gender = shared->waiting[FEMALE] >= shared->waiting[MALE] ? FEMALE : MALE;
            sem_wroom_up();
        }
        printf("%s [barber] %d: Ready for new customer\n", sex[gender], id);

        //down customers
        sem_client_down(client_gender);

        //down mutex
        sem_wroom_down();

        //decrease waiting
        shared->seats_taken--;
        shared->waiting[client_gender]--;

        //up barbers
        sem_barber_up(client_gender);

        //up mutex
        sem_wroom_up();

        printf("%s [barber] %d: serving customer...\n", sex[gender], id);
        sleep(HAIRCUT_TIME);
    }
    memory_detach(shared);
}

void client(int gender, int id)
{
    struct shared_memory* shared = memory_attach();
    printf("%s [client] %d: Entered the waiting room.\n", sex[gender], id);
    //down mutex
    sem_wroom_down();
    //if waiting < chairs
    if(shared->seats_taken < M)
    {
        //increase waiting
        shared->seats_taken++;
        shared->waiting[gender]++;

        //up customers
        sem_client_up(gender);

        //up mutex
        sem_wroom_up();

        //down barbers
        sem_barber_down(gender);

        //ready to be served by barber
        printf("%s [client] %d: Is to be served by a barber\n", sex[gender], id);
    }
    else
    {
        sem_wroom_up();
        printf("%s [client] %d: waiting room full, exiting\n", sex[gender], id);
    }
    memory_detach(shared);
}

void semaphores_get(struct semaphores* sems)
{
    sems->clients      = semget(KEY_CLIENT, 2, 0660);
    sems->barbers      = semget(KEY_BARBER, 3, 0660);
    sems->waiting_room = semget(KEY_WROOM,  1, 0660);
    if(sems->clients < 0 || sems-> barbers < 0 || sems->waiting_room < 0)
        PERROR_EXIT("semget error when using semaphores_get()");
    return;
}

void sem_wroom_down()
{
   struct sembuf sb = DOWN(0);
   struct semaphores sems;
   semaphores_get(&sems);
   if(semop(sems.waiting_room, &sb, 1) < 0)
        PERROR_EXIT("semop error when using sem_wroom_down()");
   return;
}

void sem_wroom_up()
{
   struct sembuf sb = UP(0);
   struct semaphores sems;
   semaphores_get(&sems);
   if(semop(sems.waiting_room, &sb, 1) < 0)
        PERROR_EXIT("semop error when using sem_wroom_up()");
   return;
}

void sem_client_down(int gender)
{
    struct sembuf sb = DOWN(gender);
    struct semaphores sems;
    semaphores_get(&sems);
    if(semop(sems.clients, &sb, 1) < 0)
        PERROR_EXIT("semop error\n");
    return;
}

void sem_client_up(int gender)
{
    struct sembuf sb = UP(gender);
    struct semaphores sems;
    semaphores_get(&sems);
    if(semop(sems.clients, &sb, 1) < 0)
        PERROR_EXIT("semop error when using sem_client_up()");
    return;
}

void sem_barber_down(int gender)
{
    struct sembuf sb = DOWN(gender);
    struct semaphores sems;
    semaphores_get(&sems);
    if(semop(sems.barbers, &sb, 1) < 0)
        PERROR_EXIT("semop error when using sem_barber_down()");
    return;
}

void sem_barber_up(int gender)
{
    struct sembuf sb = UP(gender);
    struct semaphores sems;
    semaphores_get(&sems);
    if(semop(sems.barbers, &sb, 1) < 0)
        PERROR_EXIT("semop error when using sem_barber_up");
    return;
}


struct shared_memory* memory_attach()
{
    // get memory
    int shmid = shmget(KEY_MEM, sizeof(struct shared_memory), 0660);
    if(shmid < 0)
        PERROR_EXIT("shmget error when using memory_attach()");
    // attach
    struct shared_memory* mem;
    mem = shmat(shmid, NULL, 0);
    if(mem == (void*) -1)
        PERROR_EXIT("shmat error\n");
    return mem;
}

void memory_detach(struct shared_memory* mem)
{
    if(shmdt(mem) < 0)
        PERROR_EXIT("shmdt error when using memory_detach()");
    return;
}

void interrupt_handler()
{
    printf("Program was interrupted. Clearning shared memory and semaphores.\n");
    release_ids();
    kill_children();
    signal(SIGINT, SIG_DFL);
    raise(SIGINT);
    exit(0);
}

void release_ids()
{
    int shmid = shmget(KEY_MEM, sizeof(struct shared_memory), 0660);
    struct semaphores sems;
    semaphores_get(&sems);

    // remove memory
    if(shmctl(shmid, IPC_RMID, 0) < 0)
        PERROR_EXIT("shmctl error\n");

    // remove semaphores
    semctl(sems.clients, 0, IPC_RMID);
    semctl(sems.clients, 0, IPC_RMID);
    semctl(sems.clients, 0, IPC_RMID);
}

void kill_children()
{
    for(int i=0; i<children_amount; i++)
    {
        kill(children[i], SIGTERM);
        printf("[client] id: %d left the barbershop.\n", children[i]);
    }
    free(children);
}
