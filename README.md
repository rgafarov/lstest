# Project lstest

## Problem description:
1. Use two threads to increment an integer. Thread A increments when even and Thread B increments when odd (for the integer problem we can have it specify up to a number provided on the command line)
2. What are some of the difficulties in adding more threads? Please show the difficulties with code.
3. Extra credit – Design an improved solution to the above that can scale with many threads


## Proposing solution
Used basic primitives for resource access synchronisation(spinlock and mutex). Event based model of synchronisation is looking more complex and can be reviewed/implemented on demand. Number of threads is not limited by implementation. Order of resource increment defined by number of threads and order of the current thread. Example below shows growing threads conflicts overhead with growing number of threads. Frequency of unsuccessful resource acquisition is growing with the number of threads.


## Basic requirements
- CMake (https://cmake.org/)
- ninja (https://ninja-build.org/)


## Build
Run build.sh from root directory. Executable binary will be placed in ./build subfolder.


## Example of execution (was run in docker based on MacOS)
```
~/lstest $ ./build/lstest 1000
lstest started (CPU cores: 4)
execution time: 10ms (custom spinlock, threads = 4, count = 1000)
execution time: 3649ms (custom spinlock, threads = 8, count = 1000)
execution time: 147ms (std spinlock, threads = 4, count = 1000)
execution time: 10078ms (std spinlock, threads = 8, count = 1000)
execution time: 72ms (mutex, threads = 4, count = 1000)
execution time: 157ms (mutex, threads = 8, count = 1000)
lstest stopped
```
