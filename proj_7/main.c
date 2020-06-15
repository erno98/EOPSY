#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define N 5 // number of philosophers
#define LEFT (i+N-1)%N  // philosopher on the left of 'i'
#define RIGHT (i+1)%N   // philosopher on the right of 'i'

// define the states
#define THINKING 0
#define HUNGRY 1
#define EATING 2

// helper variables for time of the processes
#define THINKING_TIME 2
#define EATING_TIME 1
#define SIMULATION_TIME 10

// function definition
void grab_forks(int i);
void put_away_forks(int i);
void test(int i);

// mutexes
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t s[N] = PTHREAD_MUTEX_INITIALIZER;

int state[N];  // current state of philosopher

// here I assumed that there is same numbers of meals as philosophers
// they have to eat it with two forks however
// they're philosophers not engineers, after all
int meals[N];  // meals for the philosophers

// thread (function) for philosopher
void* philosopher(void* num);


//  ------------------------------ main function

int main(int argc, char** argv){
    
	// initialize mutexes to locked state
	for(int i=0; i<N; i++) {
		pthread_mutex_lock(s+i);
	}

	int* num = (int*) malloc(N*sizeof(int));

	// start the philosopher threads
	pthread_t philosophers[N];
	for(int i=0; i<N; i++){
		num[i] = i;
        // check if thread was created correctly
		if(pthread_create(&philosophers[i], NULL, philosopher, (void*) (num+i))){
            // thread couldn't be created, exit with code 1
			fprintf(stderr, "[Philosopher: %d] failed to create. Capitalists are happy.", i);
			exit(1);
		}	
	}

	// start the simulation
	sleep(SIMULATION_TIME);

	for(int i=0; i<N; i++){
		// cancel all philosopher threads, make them leave
		pthread_cancel(philosophers[i]);
		pthread_join(philosophers[i], NULL);
		printf("[Philosopher: %d] Left the table.\n", i);
	}

    // lock mutex
	pthread_mutex_lock(&m);

    // info about meals eaten
		for(int i=0; i<N; i++){
			printf("[Philosopher: %d] Has eaten %d meals.\n", i, meals[i]);
		}

    // unlock the mutex
	pthread_mutex_unlock(&m);

    // free the memory
	free(num);

	return 0;
}

// ------------------------------ function definition

void grab_forks(int i){
    // function for putting away forks from philosopher
    // given (int) i which is the 'index' of the philosopher
    
	// lock the mutex
	pthread_mutex_lock(&m);

	state[i] = HUNGRY; // change the state to HUNGRY (1)
	printf("[Philosopher: %d] Became hungry, trying to get the forks.\n", i);
	test(i); // check if forks available

    // unlock the mutex
	pthread_mutex_unlock(&m);

	// lock the mutex - take the forks
	pthread_mutex_lock(&s[i]);
}


void put_away_forks(int i){
    // function for putting away forks from philosopher
    // given (int) i which is the 'index' of the philosopher

    // lock the mutex
	pthread_mutex_lock(&m);

	// finished eating
	++meals[i];  // increase the number of available meals
	state[i] = THINKING;  // change philosopher state to thinking
	printf("[Philosopher: %d] Finished eating, putting away forks.\n", i);

	// check philosophers nearby (if they can eat)
	test(LEFT);
	test(RIGHT);

    // unlock the mutex
	pthread_mutex_unlock(&m);
}

void test(int i){
    // function to check whether given philosopher can eat
    // three conditions:
    //      philosopher is hungry
    //      philosopher on the left is not eating
    //      philosopher on the right is not eating
    // given (int) i which is the 'index' of the philosopher

	if(state[i] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING){
        // philosopher can eat

        // change state to eating
		state[i] = EATING;

        // unlock the mutex
		pthread_mutex_unlock(&s[i]);
	}
}


void* philosopher(void* num){
    // function (thread) to act as philosopher

	int* i = (int*) num;
	printf("[Philosopher: %d] Came to the table.\n", *i);
	while(1){

		// first, initialize the philosopher to think
		printf("[Philosopher: %d] thinking...\n", *i);
		sleep(THINKING_TIME);

		// then, philosopher becomes hungry - grab forks
		grab_forks(*i);

		// finally philosopher has forks and can eat
		printf("[Philosopher: %d] eating...\n", *i);
		sleep(EATING_TIME);

		// philosopher stopped eating, can put away the forks and think again
		put_away_forks(*i);

	}

}

