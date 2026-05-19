#include<stdio.h>
#include<stdlib.h>
#include<ucontext.h>

#define MAX_THREADS 3
#define STACK_SIZE 1024*64 // 64 KB stack size for each thread

typedef enum{//enum is like a custom data type that can take one of the specified values
    READY, // for compilar this crossponds to 0
    RUNNING, // this to 1
    FINISHED, // this to 2
    BLOCKED   // this to 3
} Thread_state;

typedef struct ThreadControlBlock{//this is the structure storing info of the thread
    int id;//unique id 
    ucontext_t context;//the context of the thread
    Thread_state state;//the state of the thread
    void* stack;//pointer to the thread's stack
} TCB;

TCB thread_table[MAX_THREADS];//globally created thread table to store TCBs of all threads
int current_thread = -1;//index of the currently running thread
int thread_count = 0;//number of threads created


//Initiate the main thread

void thread_init(){
    thread_table[0].id =0;
    thread_table[0].state = RUNNING;
    thread_table[0].stack = NULL; // main thread uses the existing stack

    current_thread = 0;
    thread_count = 1;
    printf("Main thread initialized with ID: %d\n", thread_table[0].id);
}

int thread_create(void (*func)()){
    if(thread_count >= MAX_THREADS){
        printf("Error: Maximum thread limit reached.\n");
        return -1;
    }

    int new_thread_id = thread_count;
    thread_table[new_thread_id].id = new_thread_id;
    thread_table[new_thread_id].state = READY;

    getcontext(&thread_table[new_thread_id].context);//get the current context as a base for the new thread
    thread_table[new_thread_id].stack = malloc(STACK_SIZE);//allocate stack for the new thread
    if(thread_table[new_thread_id].stack == NULL){//check if stack allocation was successful
        printf("Error: Failed to allocate stack for thread %d.\n", new_thread_id);
        return -1;
    }
    thread_table[new_thread_id].context.uc_stack.ss_sp = thread_table[new_thread_id].stack;//set the stack pointer for the new thread
    thread_table[new_thread_id].context.uc_stack.ss_size = STACK_SIZE;//set the stack
    thread_table[new_thread_id].context.uc_stack.ss_flags = 0; // No special flags
    thread_table[new_thread_id].context.uc_link = &thread_table[0].context; // Return to main context after thread finishes

    makecontext(&thread_table[new_thread_id].context,func,0);//set the function to execute when the thread is scheduled
    thread_count++;
    return new_thread_id;
}

void thread_yield(){
    int old_thread = current_thread;

    int next_thread = (current_thread + 1) % thread_count;//simple round-robin scheduling
    while(thread_table[next_thread].state != READY){
        if(next_thread == current_thread){
            printf("No other threads are ready to run.\n");
            return;
        }
        next_thread = (next_thread + 1) % thread_count;
    }
    if(thread_table[old_thread].state == RUNNING){
        thread_table[old_thread].state = READY;//mark the current thread as ready
    }
    thread_table[next_thread].state = RUNNING;//mark the next thread as running
    current_thread = next_thread;
    swapcontext(&thread_table[old_thread].context, &thread_table[next_thread].context);//switch to the next thread's context
    return;
}

void thread_exit(){
    thread_table[current_thread].state = FINISHED;//mark the current thread as finished

    thread_yield();//yield to allow other threads to run}
}
void thread_A_function(){
    printf("Thread A is running.\n");
    thread_yield();//yield to allow other threads to run
    printf("Thread A is resuming.\n");
    thread_yield();
    printf("Thread A is finishing.\n");
    thread_exit();//exit the thread when done
    return;
}

void thread_B_function(){
    printf("Thread B is running.\n");
    thread_yield();//yield to allow other threads to run
    printf("Thread B is finishing.\n");
    thread_exit();//exit the thread when done
    return;
}

int main(){
    thread_init();//initialize the main thread
    thread_create(thread_A_function);//create thread A
    thread_create(thread_B_function);//create thread B

    while(1){
        int all_finished = 1;
        for(int i=1;i<thread_count;i++){
            if(thread_table[i].state != FINISHED){
                all_finished = 0;
                break;
            }
        }
        if (all_finished){
            break;
        }
        thread_yield();//yield to allow threads to run
    }
    for(int i=1;i<thread_count;i++){
        free(thread_table[i].stack);//clean up allocated stacks
    }
    printf("Main thread is resuming after threads have finished.\n");
    return 0;
}