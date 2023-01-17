#ifndef ring_BUFFER_H
#define ring_BUFFER_H
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief usage:
 * 1.create table of cells for data
 * 2.create rb_ring_buffer object by  rb_create()
 *   passing address of that table and size one cell and number of cells
 * 3.to insert content to the end 
 *   call  rb_get_back_hook   (like hook on hook conveyor belt)
 *   and get pointer to cell of table
 *   fill in cell pointed by that pointer
 * 4.to get fists inserted content 
 *   call rb_get_front_hook
 *   and get pointer to cell with that content
 * 5.on the end, call rb_destroy()  to free allocated memory on ring_buffer
 */


/**
 * @brief ring buffer for pointers
 * 
 * Structure for managing access to addresses of data place.
 * 
 * Table of data (desired type) must exest before use this constructor. 
 *  
 * Pointer require casting before use data (read/write).
 * Not makes checking of overflow during writting or underflow during reading.
 * In case of overflow, content of oldest element is overritten by pushed element.
 * In case of undeflow readed is not appropriate elemment.
 */
typedef struct ring_buffer_TT {    
    int front;
    int back;    
    size_t size;
    size_t type_size;
    void *b[];  //FAM
} ring_buffer_T;

/**
 * @brief ring_buffer for pointers to user data
 * 
 * User must provide table of data, and ring buffer serverves to store pointers to 
 * cells of that table. 
 * 
 * Buffer constructor calculate pointers to cells based on pointer to table (1 arg)
 * size of one cell provided in second arg.
 * Third arg determined capacity of buffer, and must be equal to number of 
 * cell user data table.
 *
 * 
 * @param void_data  - pointer to table of cell
 * @param data_type_size - size one cel in bytes
 * @param number_of_data - number of cell
 * @return ring_buffer_T* - pointer to newly created ring_buffer_T
 */
ring_buffer_T* rb_create(void* data_,size_t data_type_size, size_t number_of_data);
void rb_destroy(ring_buffer_T* prb);

void* rb_get_back_hook(ring_buffer_T *prb);
void* rb_get_front_hook(ring_buffer_T *prb);
void rb_log(ring_buffer_T *prb, const char * msg);


#endif //ring_BUFFER_H