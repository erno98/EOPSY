# PROJECT 2 - PROCESSES AND SIGNALS


## tsig.c
### Key idea
Program in C that tests synchronization mechanisms and signals.  
It has two main components: main program and modification using signals. The program itself operates on mechanism called forking, to which the method **fork()** has been utilized. It forks **n** given processes from the main process, each taking 10 seconds to complete. Information is constantly being fed to the console, regarding both the parent and child processes.  
For the signal part, if the directive **WITH_SIGNALS** is used, then custom termination and keyboard interrupt are implemented.  


After the process has been interrupted, the program continues to eventually print a summmary of it, i.e. number of performed processes.  

Note, that none of the processes actually do something. There's a 10 second sleep inside of the child processes, and 1 second sleep between each fork. Any functionality can be however implemented by user.

### Debugging tips
To simulate the fork() failure, simply set the fork_status to -1 for given number of iteration (line 123, inside of the *if* condition), as such:
```
  if(i==33){ ...   //the program will simulate fail of the fork() method at 33rd process
```

## run_tsig.sh
Shell script that compiles and runs the tsig.c program. **Do note**, that it has to be placed in the same directory as tsig.c. Compilation is done using GCC.


Tested on: Ubuntu 18.04.3 LTS, last test: 23/04/2020  
Compiler used: GCC 7.5.0
___
Created for course of Operating Systems, Warsaw University of Technology, Faculty of Electronics and Information Technology, 2020. Made by Ernest Pokropek
