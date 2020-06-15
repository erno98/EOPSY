### main.c -> main program to run
### run.sh -> shell script to run main

1. Would it be sufficient just to add to the old algorithm from task5 additional mutex variable to organize critical sections in functions grab_forks() and put_away_forks() for making changes to values of two mutexes indivisably?  If not, why?

It wouldn't be sufficient to just add mutexes. In case, when philosopher X enters mutex and tries to grab the forks, and can't since someone did it already, then philosopher Y wants to put away the forks being used, but can't, since philosopher X is in the mutex and waiting. A teeny-tiny deadlock happening there, sadly. In the previous task we had the possibility of blocking two forks at the same time.

2. Why m mutex is initialized with 1 and mutexes from the array s are initialized with 0's?

Mutex m is initialized to 1 to make philosopher able to grab/put away forks. If it was locked (0), then no philosopher would be able to enter the critical section and that would result in a deadlock. Array s is initialized to 0 in order to block philosopher after attempt of grabbing forks, i.e. making it only 'unlockable' by the neighbours of it.
