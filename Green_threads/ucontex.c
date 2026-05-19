#include<stdio.h>
#include<ucontext.h>

int main(){
    ucontext_t context;// context structure
    int has_jumped =0;

    printf("STEP 1: Before getcontext\n");//getting context
    getcontext(&context);//snapshot of current cpu state

    printf("STEP 2 : check for has jumped\n");//execution starts here
    if(has_jumped == 0){
        has_jumped = 1;
        printf("STEP 3: After getcontext, before setcontext\n");//modifying the context
        setcontext(&context);//restoring the context
    }
    printf("STEP 4: After setcontext\n");
    return 0;
}