#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>

//make a header so that we can store the size of the block and whether it is free or not, and a pointer to the next block
typedef struct Block {
    size_t size;
    int is_free;
    struct Block* next;
} Block;

#define BLOCK_SIZE sizeof(Block)

// global head of the linked list of blocks
void* global_head = NULL;
pthread_mutex_t malloc_mutex = PTHREAD_MUTEX_INITIALIZER;//mutex to protect the global head and the linked list of blocks from concurrent access

Block* find_free_block(Block** last, size_t size){
    Block* current = (Block*)global_head;
    Block* best_fit = NULL; // Initialize best fit block pointer
    size_t diff = -1; // Initialize difference to maximum possible value size_t = -1 assigns maxmium value beacuse it's unsigned it 
    // meaning it can't be negative and assigning -1 to it cause it to underflow and wrap around to give max value

    while(current){//check current fail safe can't find free memory block
        if(current->is_free){//if current is free
            while(current->next && current->next->is_free){//if next shell is also free merge the two blocks keep merging until we find a block that is not free or we reach the end of the list
                current->size += current->next->size + BLOCK_SIZE;//move to the next block
                current->next = current->next->next;//skip the next block since it is also free
            }
            if(current->size >= size) {
                size_t current_diff = current->size - size;
                if(current_diff == 0){
                    best_fit = current; // Perfect fit
                    break;
                }
                else if(current_diff < diff) {
                    diff = current_diff;
                    best_fit = current; // Better fit found
                }
            }
        }
        *last = current;// update the last pointer to the current block
        current = current->next;//move to the next block
    }
    if(best_fit && best_fit->size > size + BLOCK_SIZE){//check for left over
        Block* split = (Block*)((char*)(best_fit + 1) + size);//jump over to the next header position
        split->size = best_fit->size - size - BLOCK_SIZE;//assign left over size
        split->is_free = 1;
        split->next = best_fit->next;
        best_fit->size = size;
        best_fit->next = split;
    }
    return best_fit;
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
    size = ((size + 7) / 8) * 8;//align the size to 8 bytes for better performance cache in cpu reads memory in chunks of 8 bytes, so aligning the size to 8 bytes can improve performance by reducing fragmentation and ensuring that memory accesses are efficient. This is done by rounding up the requested size to the nearest multiple of 8 bytes, which can be achieved by dividing the size by 8, adding 1, and then multiplying back by 8. This way, we ensure that all allocated blocks are properly aligned in memory.
    pthread_mutex_lock(&malloc_mutex);//lock the mutex to protect the global head and the linked list of blocks from concurrent access
    Block* block;
    if(global_head == NULL){//checks for the first time if there is no block in the heap, if there is no block, then we need to request space for the first block
        block = request_space(NULL, size);//request space last pointer is NULL and size is requested size
        if(block == NULL) {
            pthread_mutex_unlock(&malloc_mutex);//unlock the mutex before returning
            return NULL;
        }//fail safe
        global_head = block;//update head new block is the head of the linked list
    }else{
        Block* last = (Block*)global_head;//already existing heap
        block = find_free_block(&last, size);//find free space in the existing heap
        if(block == NULL){//heap is full
            block = request_space(last,size);//request space 
            if(block == NULL) {
                pthread_mutex_unlock(&malloc_mutex);
                return NULL;
            }//fail safe
        }
        else{
            block->is_free =0;//occupy the block
        }
    }
    pthread_mutex_unlock(&malloc_mutex);//unlock the mutex after allocation is done
    return (void*)(block+1);//returns memory address where data will be stored
}

void* my_calloc(size_t num, size_t size){
    size_t total_size = num * size;
    if(total_size <= 0) return NULL;//fail safe

    void* ptr = my_malloc(total_size);
    if(ptr == NULL) return NULL;//fail safe

    memset(ptr, 0,total_size);
    return ptr;
}

void my_free(void* ptr){
    if(ptr == NULL) return;//fail safe
    pthread_mutex_lock(&malloc_mutex);
    Block* block = (Block*)ptr -1;//move back from the storage area to the header to get the block metadata
    block->is_free = 1;//free the block size of the block remains the same, but it is now marked as free
    pthread_mutex_unlock(&malloc_mutex);
    return;
}

void* my_realloc(void* ptr, size_t size){
    if(ptr == NULL) return my_malloc(size);
    if(size <= 0) {//fail safe
        my_free(ptr);
        return NULL;
    }
    pthread_mutex_lock(&malloc_mutex);
    Block* block = (Block*)ptr -1;//mvoe to the header
    size = ((size + 7) / 8) * 8;
    if(block->size >= size) {
        // --- FIX 3: Added Splitting Optimization here ---
        if (block->size > size + BLOCK_SIZE) {
            Block* split = (Block*)((char*)(block+1) + size);
            split->size = block->size - size - BLOCK_SIZE;
            split->is_free = 1;
            split->next = block->next;
            block->size = size;
            block->next = split;
        }
        pthread_mutex_unlock(&malloc_mutex);
        return ptr;
    }
    size_t old_size = block->size;
    pthread_mutex_unlock(&malloc_mutex);

    void* new_ptr = my_malloc(size);
    if(new_ptr == NULL) {
        pthread_mutex_unlock(&malloc_mutex);
        return NULL;
    }//fail safe
    size_t copy_size = (old_size < size) ? old_size : size;//copy only the minimum of the old and new sizes to avoid overflow
    memcpy(new_ptr, ptr, copy_size);
    my_free(ptr);
    return new_ptr;
}

void debug_heap() {
    pthread_mutex_lock(&malloc_mutex);
    Block* current = (Block*)global_head;//temp pointer to traverse the heap
    printf("   [Heap Map]: ");
    if (!current) {//check if the heap is empty
        printf("Empty\n");
        pthread_mutex_unlock(&malloc_mutex);
        return;
    }
    while (current) {//check the size and free status of each block in the heap and print it out
        printf("[%luB | %s] -> ", current->size, current->is_free ? "FREE" : "USED");
        current = current->next;
    }
    printf("NULL\n");
    pthread_mutex_unlock(&malloc_mutex);
    return;
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