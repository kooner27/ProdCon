#include <iostream>
#include <string>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <queue>
#include "tands.c"
#include <chrono> // Include for time measurement
#include <map>
#include <vector>

using namespace std;
using namespace std::chrono;

int QUEUE_SIZE = 16;

sem_t semEmpty;
sem_t semFull;

pthread_mutex_t mutexQueue;
pthread_mutex_t mutexCout; // Mutex for protecting cout

queue<int> taskQueue;

high_resolution_clock::time_point start_time;

/// summary stats
map<int, int> completed_tasks;

int num_T_commands = 0; // Counter for 'T' commands
int num_asks = 0; // Counter for asks for work
int num_receives = 0; // Counter for work assignments
int num_completes = 0; // Counter for completed tasks
int num_S_commands = 0; // Counter for 'S' commands

double get_time_difference(const high_resolution_clock::time_point& start) {
    high_resolution_clock::time_point end = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(end - start);
    return time_span.count();
}

void logEvent(const string& message) {
    pthread_mutex_lock(&mutexCout);
    cout << message << endl;
    pthread_mutex_unlock(&mutexCout);
}

void* producer(void* args) {
    string input;

    while (getline(cin, input)) {
        char task_type = input[0]; // T or S
        string task_number_str = input.substr(1);
        int num = atoi(task_number_str.c_str()); // int

        if (task_type == 'T') {
            num_T_commands +=1;
            sem_wait(&semEmpty);
            pthread_mutex_lock(&mutexQueue);
            taskQueue.push(num);
            pthread_mutex_unlock(&mutexQueue);
            sem_post(&semFull);
        } else {
            // Handle 'S' task if needed
            num_S_commands +=1;
            Sleep(num);
        }
    }
}

void* consumer(void* args) {
    int id = *((int*)args); // Consumer ID

    while (1) {
        // LOG ASK
        logEvent(to_string(get_time_difference(start_time)) + " ID= " + to_string(id) + " Ask");
        num_asks+=1;

        int y;
        // Remove from the queue
        sem_wait(&semFull);
        pthread_mutex_lock(&mutexQueue);
        y = taskQueue.front();
        taskQueue.pop();
        pthread_mutex_unlock(&mutexQueue);
        sem_post(&semEmpty);

        // LOG RECEIVE
        logEvent(to_string(get_time_difference(start_time)) + " ID= " + to_string(id) + " Q= " + to_string(taskQueue.size()) + " Receive " + to_string(y));
        num_receives += 1;
        // Consume and call the Trans function
        Trans(y); // Call Trans with the integer y

        // LOG COMPLETE
        logEvent(to_string(get_time_difference(start_time)) + " ID= " + to_string(id) + " Complete " + to_string(y));
        num_completes+=1;
        completed_tasks[id]+=1;
    }
}

int main(int argc, char* argv[]) {
    freopen("log", "w", stdout);
    start_time = high_resolution_clock::now();

    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <num_consumers>" << endl;
        return 1;
    }

    int nthreads = atoi(argv[1]);

    srand(time(NULL));
    pthread_t threads[nthreads]; // 1 producer + num_consumers consumers
    pthread_mutex_init(&mutexQueue, NULL);
    pthread_mutex_init(&mutexCout, NULL); // Initialize the mutex for cout
    QUEUE_SIZE = 2 * nthreads;
    sem_init(&semEmpty, 0, QUEUE_SIZE);
    sem_init(&semFull, 0, 0);

    int i;
    vector<int> consumer_ids(nthreads);
    pthread_create(&threads[0], NULL, producer, nullptr);

    for (i = 0; i < nthreads; i++) {
        consumer_ids[i] = i + 1; // Start consumer IDs from 1
        pthread_create(&threads[i + 1], NULL, consumer, &consumer_ids[i]); // Pass the consumer ID
    }

    for (i = 0; i < nthreads + 1; i++) {
        pthread_join(threads[i], nullptr);
    }

    sem_destroy(&semEmpty);
    sem_destroy(&semFull);
    pthread_mutex_destroy(&mutexQueue);
    pthread_mutex_destroy(&mutexCout); // Destroy the mutex for cout

    // summary
        // Print summary
    cout << "Summary:" << endl;
    cout << " Work " << num_T_commands << endl; // Producer: # of 'T' commands" 
    cout << " Ask " << num_asks << endl;         // Consumer: # of asks for work" << endl;
    cout << " Receive " << num_receives << endl; // Consumer: # work assignments" << endl;
    cout << " Complete " << num_T_commands << endl; // Consumer: # completed tasks" << endl;
    cout << " Sleep " << num_S_commands << endl; // Producer: #
    return 0;
}
