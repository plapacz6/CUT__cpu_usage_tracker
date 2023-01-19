/*************************  TESTS ************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../ring_buffer.h"

extern size_t number_of_data;
extern long double *ptr_vla_a; //to VLA

int test_ring_buffer(){
    
    ring_buffer_T *prb = rb_create(ptr_vla_a, sizeof(int), number_of_data);
                            
    int el;
    int el_test = 100;
    int *ptr_el = NULL;

    for(int i = 0; i < number_of_data /*+ 2*/; i++){

        puts("--- push_back  ---");                    
        for(int j = 0, el = el_test; j < i; j++, el++){
                                        
            ptr_el = (int*)rb_get_back_hook(prb);  
            assert(ptr_el != NULL);                            
            *ptr_el = el;  
        }   
                            
        puts("--- pop_front  ---");
        for(int j = 0, el = el_test; j < i; j++, el++){            
                            
            ptr_el = (int*)rb_get_front_hook(prb);
            assert(ptr_el != NULL); 
            if(j <= number_of_data){
                printf("| i: %d, j: %d ", i, j);
                assert(el == *ptr_el);
            }
            else{
                printf("| i: %d, j: %d ", i, j);
                assert(el != *ptr_el);
            }
        }        
        puts("%================\n");
    }    

    rb_destroy(prb);

    return 0; //ok
}



size_t number_of_data = 20;
long double *ptr_vla_a; //to VLA

int main(){
    long double a[number_of_data][7]; //VLA
    ptr_vla_a = (long double *)&a;

    printf("%s\n", "Tests: ring buffer :");
    if(0 == test_ring_buffer()){
        printf("%s\n", " PASS");
    }
    else{
        printf("%s\n", "  FAIL");  //assert not allow come here
        exit(1);
    }
    return 0;
}

