#include <stdio.h>
#include <malloc.h>
#include "ring_buffer.h"

#define LOGGER_BUFFER_SIZE (1025)
extern char *ptr_logger_buffer;

ring_buffer_t* rb_create(void* void_data,size_t data_type_size, size_t number_of_data){
    ring_buffer_t *prb = 
        (ring_buffer_t*) malloc(sizeof(ring_buffer_t) + number_of_data * sizeof(void*));
                           
    prb->size = number_of_data;
    prb->type_size = data_type_size;
    prb->front = 0;
    prb->back = 0;
    char *char_data = (char*)void_data;
    for(int i = 0; i < prb->size; i++){
        /* 
        *  prb->b  store address of box in data table (unknown type /void_data/)
        *  prb->type_size - size of one unit of data in bytes 
        *  char_data - pointer to one byte (type char*) pointng to data table
        */
        prb->b[i] = (void*) (char_data + (i * prb->type_size));
    }                        
    return prb;
}
void rb_destroy(ring_buffer_t* prb){
    if(prb != NULL){
        free(prb);
    }
}
/**
 * @brief returns a pointer to one 'box' in the data table, 
 * which is a pointer to place to write
 * @param prb 
 * @return void* pointer to one box in data table (need casting)
 */
void* rb_get_back_hook(ring_buffer_t *prb){
    void *el = prb->b[prb->back];
    prb->back++;
    if(prb->back == prb->front)
        rb_log(prb, "ring buffer overflow");    
    prb->back %= prb->size; 
    return el;
}
/**
 * @brief returns a pointer to one 'box' in the data table, 
 * which is a pointer to the data to read 
 * @param prb 
 * @return void* pointer to one box in data table (need casting)
 */
void* rb_get_front_hook(ring_buffer_t *prb){                        
    void *el = prb->b[prb->front];                        
    if(prb->front == prb->back)
        rb_log(prb, "ring buffer underflow");
    prb->front++;                        
    prb->front %= prb->size;                        
    return el;
}

void rb_log(ring_buffer_t *prb, const char * msg){
    fprintf(stderr, "ring_buffer: %p: %s", prb, msg);
}