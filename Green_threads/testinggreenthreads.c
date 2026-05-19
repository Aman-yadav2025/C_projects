#include<stdio.h>
#include<stdlib.h>
#include<ucontext.h>

void green_thread_function(){
    printf("Hello from the green thread!\n");
    printf("Green thread is executing some function.\n");
    printf("Green thread is exiting.\n");
}

int main(){
    printf("Main thread is starting.\n");
    ucontext_t main_context, green_thread_context;

    // Initialize the green thread context
    getcontext(&green_thread_context);
    size_t stack_size = 1024 * 64; // 64 KB stack
    void* thread_stack = malloc(stack_size);
    if (thread_stack == NULL) {
        perror("Failed to allocate stack");
        return 1;
    }
    green_thread_context.uc_stack.ss_sp = thread_stack;// Set the stack pointer for the green thread
    green_thread_context.uc_stack.ss_size = stack_size;//set stack size for the green thread
    green_thread_context.uc_stack.ss_flags = 0; // No special flags

    green_thread_context.uc_link = &main_context; // Return to main context after thread finishes
    //set a function to execute when the green thread is scheduled
    //0 is indicating that the function takes no arguments
    makecontext(&green_thread_context, green_thread_function, 0);
    //swap the current context with the green thread context, 
    //this get the current state of the main and save it in main_context, then switch to green_thread_context
    swapcontext(&main_context, &green_thread_context);
    printf("Main thread is resuming after green thread has finished.\n");

    free(thread_stack); // Clean up the allocated stack
    return 0;
}