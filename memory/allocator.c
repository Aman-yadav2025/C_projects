#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>

typedef struct Block {
    size_t size;
    int is_free;
    struct Block* next;
} Block;

#define BLOCK_SIZE sizeof(Block)

void* global_head = NULL;

Block* find_free_block(Block** last, size_t size){
    Block* current = (Block*)global_head;
    // if current is null or current is not free or current size is less than size, then loop
    while(current && !(current->is_free && current->size >= size)){
        *last = current;
        current = current->next;
    }
    return current;
}

Block *request_space(Block* last,size_t size){
    Block* block;
    // sbrk(0) returns the current program break, which is the end of the heap. We can use this to get the address of the new block.
    block = sbrk(0);//gets the address of the top of the heap
    void *request = sbrk(size + BLOCK_SIZE);
    if(request == (void*)-1) return NULL;
    if(last) last->next = block;
    block->size = size;
    block->is_free = 0;
    block->next = NULL;
    return block;
}

void* my_malloc(size_t size){
    if(size <= 0) return NULL;
    Block* block;
    if(global_head == NULL){
        block = request_space(NULL, size);
        if(block == NULL) return NULL;
        global_head = block;
    }else{
        Block* last = (Block*)global_head;
        block = find_free_block(&last, size);
        if(block == NULL){
            block = request_space(last,size);
            if(block == NULL) return NULL;
        }
        else{
            block->is_free =0;
        }
    }
    return (void*)(block+1);
}

void my_free(void* ptr){
    if(ptr == NULL) return;
    Block* block = (Block*)ptr -1;
    block->is_free = 1;
    return;
}

void debug_heap() {
    Block* current = (Block*)global_head;
    printf("   [Heap Map]: ");
    if (!current) {
        printf("Empty\n");
        return;
    }
    while (current) {
        printf("[%luB | %s] -> ", current->size, current->is_free ? "FREE" : "USED");
        current = current->next;
    }
    printf("NULL\n");
}

int main(){
    printf("Testing custom memory allocator...\n");
    printf("Metadata header size: %lu bytes\n", BLOCK_SIZE);
    
    int* arr = (int*)my_malloc(10 * sizeof(int));
    if(arr == NULL){
        printf("Memory allocation failed\n");
        return 1;
    }
    for(int i = 0; i < 10; i++){
        arr[i] = i;
    }
    printf("Allocated array: ");
    for(int i = 0; i < 10; i++){
        printf("%d ", arr[i]);
    }
    printf("\n");
    debug_heap();

    char* str = (char*)my_malloc(20 * sizeof(char));
    if(str == NULL){
        printf("Memory allocation failed\n");
        return 1;
    }
    strcpy(str, "Hello, World!");
    printf("Allocated string: %s\n", str);
    debug_heap();

    printf("\nFreeing the integer array...");
    my_free(arr);
    debug_heap();

    printf("\nAllocating 12 bytes (should fit inside the newly freed Block 1)...");
    double* dbl = (double*)my_malloc(sizeof(double)); // A double is 8 bytes
    *dbl = 3.14159;
    debug_heap();

    // Clean up remaining memory
    my_free(str);
    my_free(dbl);
    
    printf("\nAll memory freed successfully.\n");
    return 0;
}