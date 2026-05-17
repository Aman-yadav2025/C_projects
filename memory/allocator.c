#include<stdio.h>
#include<string.h>
#include<unistd.h>

typedef struct Block {
    size_t size;
    int is_free;
    struct Block* next;
} Block;

#define BLOCK_SIZE sizeof(Block)

void* global_head = NULL;

Block* find_free_block(Block** last, size_t size){
    Block* current = (Block*)global_head;
    while(current && !(current->is_free && current->size >= size)){
        *last = current;
        current = current->next;
    }
    return current;
}

Block *request_space(Block* last,size_t size){
    Block* block
    block = sbrk(0);
    void *request = sbrk(size + BLOCK_SIZE);
    if(request == (void*)-1) return NULL;
    if(last) last->next = block;
    block->size = size;
    block->is_free = 0;
    block->next = NULL;
    return block;
}

void my_malloc(size_t size){
    if(size <= 0) return NULL;
    return NULL;
}

void my_free(void* ptr){
    if(ptr == NULL) return;
    return;
}

int main(){

}