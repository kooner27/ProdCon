#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

#include <chrono>  // Include for time measurement
#include <cstdlib>
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <vector>

#include "tands.c"

using namespace std;
using namespace std::chrono;

int QUEUE_SIZE = 16;

sem_t semEmpty;
sem_t semFull;

pthread_mutex_t mutexQueue;
pthread_mutex_t mutexCout;

queue<int> taskQueue;

high_resolution_clock::time_point start_time;

/// summary stats
map<int, int> completed_tasks;

int num_T_commands = 0;  // Counter for 'T' commands
int num_asks = 0;        // Counter for asks for work
int num_receives = 0;    // Counter for work assignments
int num_completes = 0;   // Counter for completed tasks
int num_S_commands = 0;  // Counter for 'S' commands

int num_consumers;  // Number of consumer threads
int finished;
pthread_cond_t condAllConsumersFinished;  // Condition variable to wait for all consumers to finish
pthread_mutex_t mutexFinished;            // Mutex to protect the num_consumers variable

void logEvent(const string& message) {
    pthread_mutex_lock(&mutexCout);
    cout << message << endl;
    pthread_mutex_unlock(&mutexCout);
}

double get_time_difference(const high_resolution_clock::time_point& start) {
    high_resolution_clock::time_point end = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(end - start);
    return time_span.count();
}

void* producer(void* args) {
    string input;

    while (getline(cin, input)) {
        char task_type = input[0];  // T or S
        string task_number_str = input.substr(1);
        int num = atoi(task_number_str.c_str());  // int

        if (task_type == 'T') {
            logEvent(to_string(get_time_difference(start_time)) + " ID= " + to_string(0) +
                     " Q= " + to_string(taskQueue.size()) + " Work " + to_string(num));
            num_T_commands += 1;

            sem_wait(&semEmpty);                // decrement number of empty spots in Q
            pthread_mutex_lock(&mutexQueue);    // lock the Q
            taskQueue.push(num);                // push the task
            pthread_mutex_unlock(&mutexQueue);  // unlock the Q
            sem_post(&semFull);                 // increment number of full spots in Q
        } else {
            // Handle 'S' task if needed
            logEvent(to_string(get_time_difference(start_time)) + " ID= " + to_string(0) + " Sleep " + to_string(num));
            num_S_commands += 1;
            Sleep(num);
        }
    }

    // Signal that all consumers should finish
    pthread_mutex_lock(&mutexFinished);                 // conditional variable
    finished = 1;                                       // we are done producing work
    pthread_cond_broadcast(&condAllConsumersFinished);  // send signal
    pthread_mutex_unlock(&mutexFinished);

    return nullptr;
}

void* consumer(void* args) {
    int id = *((int*)args);  // Consumer ID

    while (1) {
        // LOG ASK
        logEvent(to_string(get_time_difference(start_time)) + " ID= " + to_string(id) + " Ask");
        num_asks += 1;

        int y;

        // Remove from the queue
        sem_wait(&semFull);  // P is decrement. means there are tasks
        pthread_mutex_lock(&mutexQueue);
        y = taskQueue.front();
        taskQueue.pop();
        pthread_mutex_unlock(&mutexQueue);
        sem_post(&semEmpty);

        // LOG RECEIVE
        logEvent(to_string(get_time_difference(start_time)) + " ID= " + to_string(id) +
                 " Q= " + to_string(taskQueue.size()) + " Receive " + to_string(y));
        num_receives += 1;
        // Consume and call the Trans function
        Trans(y);  // Call Trans with the integer y

        // LOG COMPLETE
        logEvent(to_string(get_time_difference(start_time)) + " ID= " + to_string(id) + " Complete " + to_string(y));
        num_completes += 1;
        completed_tasks[id] += 1;

        // Check if all consumers have finished, and exit if so
        pthread_mutex_lock(&mutexFinished);  // lock b4 reading
        if (finished == 1) {                 // producer is finished
            pthread_cond_signal(&condAllConsumersFinished);
            pthread_mutex_unlock(&mutexFinished);
            break;
        }
        pthread_mutex_unlock(&mutexFinished);
    }

    return nullptr;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <num_consumers> <logfile_number>" << endl;
        return 1;
    }

    // get nthreads, lognum, create log, start clock
    int nthreads = atoi(argv[1]);
    int logfile_number = atoi(argv[2]);
    string logFileName = "prodcon." + to_string(logfile_number) + ".log";
    freopen(logFileName.c_str(), "w", stdout);
    start_time = high_resolution_clock::now();

    // init n threads, queue mutex, empty spot full spot sem,
    pthread_t threads[nthreads];  // 1 producer + num_consumers consumers
    pthread_mutex_init(&mutexQueue, NULL);
    QUEUE_SIZE = 2 * nthreads;
    sem_init(&semEmpty, 0, QUEUE_SIZE);
    sem_init(&semFull, 0, 0);
    num_consumers = nthreads;

    // finished variable mutex and conitional signal
    pthread_mutex_init(&mutexFinished, NULL);
    pthread_cond_init(&condAllConsumersFinished, NULL);

    // create producer
    int i;
    pthread_create(&threads[0], NULL, producer, nullptr);

    // Create an array of unique IDs for consumers
    vector<int> consumer_ids(nthreads);
    for (i = 1; i <= nthreads; i++) {
        consumer_ids[i - 1] = i;
        pthread_create(&threads[i], NULL, consumer, &consumer_ids[i - 1]);  // Pass the consumer ID
    }

    // Wait for all consumer threads to finish
    pthread_mutex_lock(&mutexFinished);
    while (finished == 0) {
        pthread_cond_wait(&condAllConsumersFinished, &mutexFinished);  // release and wait
    }
    pthread_mutex_unlock(&mutexFinished);  // unneccessary i think but good practice to unlock

    // done so calculate transactions per second = num Ts / time_dif
    double tps = static_cast<double>(num_T_commands) / get_time_difference(start_time);

    // join all threads
    for (i = 0; i < 1 + nthreads; i++) {
        pthread_join(threads[i], nullptr);
    }

    // destroy
    sem_destroy(&semEmpty);
    sem_destroy(&semFull);
    pthread_mutex_destroy(&mutexQueue);
    pthread_mutex_destroy(&mutexFinished);
    pthread_cond_destroy(&condAllConsumersFinished);

    // summary
    // Print summary
    cout << "Summary:" << endl;
    cout << " Work " << num_T_commands << endl;     // Producer: # of 'T' commands"
    cout << " Ask " << num_asks << endl;            // Consumer: # of asks for work" << endl;
    cout << " Receive " << num_receives << endl;    // Consumer: # work assignments" << endl;
    cout << " Complete " << num_completes << endl;  // Consumer: # completed tasks" << endl;
    cout << " Sleep " << num_S_commands << endl;    // Producer: # of 'S' commands" << endl;

    // Print completed task count for each thread
    for (const auto& pair : completed_tasks) {
        cout << " Thread " << pair.first << " " << pair.second << endl;
    }
    cout << "Transactions per second: " << tps << endl;

    return 0;
}
