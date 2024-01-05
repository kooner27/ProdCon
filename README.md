## Running Instructions
```shell
g++ sem.cpp -o prodcon
./prodcon <nthreads> <id> <inputexample
```
This will spawn 'nthreads' number of consumers and activity will be logged in prodon.id.log.
Please see the pdf for more details such as example output. In the file sem2.cpp, I am working on an implementation that doesn't rely on pthread conditionals and instead uses only semaphores for everything.

## Additional Info
Readings:\
https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem
