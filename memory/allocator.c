#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>

//make a header so that we can store the size of the block and whether it is free or not, and a pointer to the next block
typedef struct Block {
    size_t size;
    int is_free;
    struct Block* next;
} Block;

#define BLOCK_SIZE sizeof(Block)

// global head of the linked list of blocks
void* global_head = NULL;

Block* find_free_block(Block** last, size_t size){
    Block* current = (Block*)global_head;
    // if current is null or current is not free or current size is less than size, then loop
    while(current && !(current->is_free && current->size >= size)){
        *last = current;
        current = current->next;
    }
    Block* split;
    if(current && current->size > size + BLOCK_SIZE){//check for left over space
        split = (Block*)((char*)current + BLOCK_SIZE + size);//convert the current pointer to char to have exact byte address
        split->size = current->size - size - BLOCK_SIZE;
        split->is_free =1;
        split->next = current->next;//pointers adjustment for the split block
        current->size = size;
        current->next = split;
    }
    return current;
}

Block *request_space(Block* last,size_t size){
    Block* block;
    // sbrk(0) returns the current program break, which is the end of the heap. We can use this to get the address of the new block.
    block = sbrk(0);//gets the address of the top of the heap
    void *request = sbrk(size + BLOCK_SIZE);//increase the size of the heap to add the new block
    if(request == (void*)-1) return NULL;//fail safe
    if(last) last->next = block; // if there is a last block, set its next pointer to the new block
    block->size = size;
    block->is_free = 0;
    block->next = NULL;//set the next pointer of the new block to null
    return block;//returns memory address
}

void* my_malloc(size_t size){
    if(size <= 0) return NULL;//fail safe
    Block* block;
    if(global_head == NULL){//checks for the first time if there is no block in the heap, if there is no block, then we need to request space for the first block
        block = request_space(NULL, size);//request space last pointer is NULL and size is requested size
        if(block == NULL) return NULL;//fail safe
        global_head = block;//update head new block is the head of the linked list
    }else{
        Block* last = (Block*)global_head;//already existing heap
        block = find_free_block(&last, size);//find free space in the existing heap
        if(block == NULL){//heap is full
            block = request_space(last,size);//request space 
            if(block == NULL) return NULL;//fail safe
        }
        else{
            block->is_free =0;//occupy the block
        }
    }
    return (void*)(block+1);//returns memory address where data will be stored
}

void my_free(void* ptr){
    if(ptr == NULL) return;//fail safe
    Block* block = (Block*)ptr -1;//move back from the storage area to the header to get the block metadata
    block->is_free = 1;//free the block size of the block remains the same, but it is now marked as free
    return;
}

void debug_heap() {
    Block* current = (Block*)global_head;//temp pointer to traverse the heap
    printf("   [Heap Map]: ");
    if (!current) {//check if the heap is empty
        printf("Empty\n");
        return;
    }
    while (current) {//check the size and free status of each block in the heap and print it out
        printf("[%luB | %s] -> ", current->size, current->is_free ? "FREE" : "USED");
        current = current->next;
    }
    printf("NULL\n");
}

int main(){
    printf("Testing custom memory allocator...\n");
    printf("Metadata header size: %lu bytes\n", BLOCK_SIZE);
    
    int* arr = (int*)my_malloc(10 * sizeof(int));//allocate an array of 10 integers
    if(arr == NULL){//fail safe check
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
    debug_heap();//print the heap map after allocation

    char* str = (char*)my_malloc(20 * sizeof(char));//allocate a string of 20 characters
    if(str == NULL){//fail safe check
        printf("Memory allocation failed\n");
        return 1;
    }
    strcpy(str, "Hello, World!");//assign a string to the allocated memory
    printf("Allocated string: %s\n", str);
    debug_heap();//print the heap map after allocation

    printf("\nFreeing the integer array...");
    my_free(arr);//free the integer array
    debug_heap();//print the heap map after freeing the integer array

    printf("\nAllocating 12 bytes (should fit inside the newly freed Block 1)...");//utilize the freed space from the integer array allocation
    double* dbl = (double*)my_malloc(sizeof(double)); // A double is 8 bytes
    *dbl = 3.14159;
    debug_heap();//print the heap map after allocating the double

    // Clean up remaining memory
    my_free(str);
    my_free(dbl);
    
    printf("\nAll memory freed successfully.\n");
    return 0;
}