#include <stdio.h>
#include <malloc.h>
#include <assert.h>

#include "ring_buffer.h"

ring_buffer_T* rb_create(void* void_data,size_t data_type_size, size_t number_of_data) {
    ring_buffer_T *prb =
        (ring_buffer_T*) malloc(sizeof(ring_buffer_T) + number_of_data * sizeof(void*));

    prb->size = number_of_data;
    prb->type_size = data_type_size;
    prb->front = 0;
    prb->back = 0;
    prb->count = 0;
    cnd_init(&prb->nonempty);
    cnd_init(&prb->nonfull);
    mtx_init(&prb->mtx, mtx_plain);
    char *char_data = (char*)void_data;
    for(size_t i = 0; i < prb->size; ++i) {
        /*
        *  prb->b  store address of box in data table (unknown type /void_data/)
        *  prb->type_size - size of one unit of data in bytes
        *  char_data - pointer to one byte (type char*) pointng to data table
        */
        prb->b[i] = (void*) (char_data + (i * prb->type_size));
    }
    return prb;
}
void rb_destroy(ring_buffer_T** pprb) {
    cnd_destroy(&(*pprb)->nonempty);
    cnd_destroy(&(*pprb)->nonfull);
    mtx_destroy(&(*pprb)->mtx);
    if(*pprb) {
        free(*pprb);
        *pprb = NULL;
    }
}
/**
 * @brief returns a pointer to one 'box' in the data table,
 * which is a pointer to place to write
 * @param prb
 * @return void* pointer to one box in data table (need casting)
 */
void* rb_get_back_hook(ring_buffer_T *prb) {
    mtx_lock(&prb->mtx);
    while(prb->count == prb->size) {
        cnd_wait(&prb->nonfull, &prb->mtx);
    }
    assert(prb->count < prb->size);
    void *el = prb->b[prb->back];
    prb->back++;
    prb->back %= prb->size;
    prb->count++;
    cnd_signal(&prb->nonempty);
    mtx_unlock(&prb->mtx);
    return el;
}
/**
 * @brief returns a pointer to one 'box' in the data table,
 * which is a pointer to the data to read
 * @param prb
 * @return void* pointer to one box in data table (need casting)
 */
void* rb_get_front_hook(ring_buffer_T *prb) {
    mtx_lock(&prb->mtx);
    while(prb->count == 0) {
        cnd_wait(&prb->nonempty, &prb->mtx);
    }
    void *el = prb->b[prb->front];
    prb->front++;
    prb->front %= prb->size;
    assert(0 < prb->count);
    prb->count--;
    cnd_signal(&prb->nonfull);
    mtx_unlock(&prb->mtx);
    return el;
}

