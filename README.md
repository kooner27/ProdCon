Here is a `README.md` file for your Producer-Consumer solution in C++:

---

# Producer-Consumer Solution in C++ 🚀

A multithreaded implementation of the **Producer-Consumer Problem**, a classic concurrency challenge in operating systems.\
This solution is designed as a genereal-purpose framework for efficiently solving any producer-consumer related tasks.\
It can be easily modified for your use-case, whenever you want to get the most out of your cpu for any task.

---

## Features

- **Scalable**: Configurable number of consumer threads.
- **Efficient Synchronization**: Uses semaphores and mutexes to coordinate producer and consumer threads.
- **Dynamic Queue Management**: Supports dynamic task assignment and completion.
- **Performance Metrics**: Calculates transactions per second (TPS) and provides detailed task execution summaries.
- **Log Generation**: Outputs detailed logs for debugging and performance monitoring.

---

## Getting Started

### Prerequisites

- A C++ compiler supporting `pthread` (e.g., GCC or Clang).
- Basic understanding of concurrency concepts (e.g., threads, semaphores, and mutexes).

### Compilation

Compile the program using the following command:

```bash
g++ -pthread -o prodcon prodcon.cpp tands.c
```

### Input Format

The program reads input tasks from `stdin`. Each line should contain:

- `T<number>`: A task requiring a unit of work. For example, `T10` means a task with a workload of 10.
- `S<number>`: A sleep command, where the producer sleeps for the specified number of seconds. For example, `S5`.

See the "inputexample" file for some example input.

### Running the Program

Execute the program with the following syntax:

```bash
./prodcon <num_consumers> <logfile_number>
```

- `<num_consumers>`: Number of consumer threads.
- `<logfile_number>`: A unique identifier for the log file.

Example:

```bash
./prodcon 4 1
```

This will start the producer-consumer system with 4 consumers and output logs to `prodcon.1.log`.

Since the program reads from standard in you can also run it like:

```
./prodcon <num_consumers> <logfile_number> < inputexample.txt
```

---

## Output

The program generates detailed logs in the format:

```
<timestamp> ID= <thread_id> Q= <queue_size> <event> <task_number>
```

### Log Examples

- `0.001 ID= 0 Q= 0 Work 10`: Producer adds a task with workload 10.
- `0.005 ID= 1 Q= 0 Ask`: Consumer 1 asks for a task.
- `0.006 ID= 1 Q= 0 Receive 10`: Consumer 1 receives task 10.
- `0.010 ID= 1 Q= 0 Complete 10`: Consumer 1 completes task 10.

### Summary

At the end of execution, a summary is printed:

```
Summary:
 Work <number_of_tasks_produced>
 Ask <number_of_asks>
 Receive <number_of_tasks_received>
 Complete <number_of_tasks_completed>
 Sleep <number_of_sleeps>
 Thread <thread_id> <tasks_completed_by_thread>
 Transactions per second: <tps>
```

---

## How It Works

### Producer

- Continuously reads tasks from `stdin` and adds them to a shared queue.
- Uses semaphores to manage available space in the queue.

### Consumer

- Requests tasks from the queue, processes them, and logs completion.
- Uses semaphores to manage tasks availability in the queue.

### Synchronization

- **Semaphores**:
  - `semEmpty`: Tracks available space in the queue.
  - `semFull`: Tracks available tasks in the queue.
- **Mutexes**:
  - `mutexQueue`: Protects access to the shared task queue.
  - `mutexFinished`: Coordinates task completion signals.

---

## Enhancements

- **Extend the queue size dynamically**: Optimize for more intensive workloads.
- **Improve load balancing**: Implement smarter work allocation strategies.
- **Add priority-based tasks**: Enable prioritization of critical tasks.
- **Measure system performance**: Incorporate advanced profiling tools.

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---

## Example Execution

Command:

```bash
./prodcon 3 1 <inputexample
```

Output:

```
Summary:
 Work 8
 Ask 8
 Receive 8
 Complete 8
 Sleep 2
 Thread 1 3
 Thread 2 3
 Thread 3 2
Transactions per second: 79.3245

```

---
