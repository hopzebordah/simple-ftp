#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_THREADS 2

void *my_thread_function(void *thread_argument);

int current_threads = 0;

int main(void) {
    for (long i=0; i<4; i++) {
        printf("current_threads: %d, attempting to create thread: %ld\n", current_threads, i);
        if (current_threads < MAX_THREADS) {
            printf("creating thread: %ld\n", i);
            pthread_t thread_id;
            pthread_create(&thread_id, NULL, my_thread_function, (void *)i);
        } else {
            printf("thread creation denied for thread: %ld\n", i);
        }
    }

    return 0;
}

void *my_thread_function(void *thread_argument) {
    current_threads++;
    printf("Hello from thread %ld\n", (long)thread_argument);
    sleep(250);
    current_threads--;
    pthread_exit(NULL);
}