#include "gltthread.h"
#include "stdlib.h"

void init_glthread(glthread_t *glthread) {
 glthread -> left  = NULL;
 glthread -> right = NULL;
} 

void glthread_add_next(glthread_t *curr_glthread, glthread_t *new_glthread) {
 if(!curr_glthread->right) {
    curr_glthread->right = new_glthread;
    new_glthread->left = curr_glthread;
return ;
}
glthread_t *temp = curr_glthread->right;
curr_glthread->right = new_glthread; 
new_glthread->left = curr_glthread; 
new_glthread->right = temp; 
temp->left=new_glthread; 
}

void glthread_add_before(glthread_t *curr_glthread, glthread_t *new_glthread) {
if (!curr_glthread->left) {
new_glthread->left=NULL;
curr_glthread->left = new_glthread; 
new_glthread->right = curr_glthread;
return; 
}
glthread_t *temp = curr_glthread->left;
temp->right = new_glthread; 
new_glthread->left = temp; 
new_glthread->right = curr_glthread ; 
curr_glthread->left=new_glthread; 

}

void remove_glthread(glthread_t *curr_glthread) {
if (!curr_glthread->left) {
curr_glthread->right->left = NULL;
curr_glthread->right = 0 ; 
return;
}
if(!curr_glthread->right) {
curr_glthread->left->right = NULL;
curr_glthread->left = 0 ; 
return ; 
}
curr_glthread->left->right = curr_glthread->right;
curr_glthread->right->left = curr_glthread->left;
curr_glthread->left = 0 ; 
curr_glthread->right = 0 ;
}


void delete_glthread_list(glthread_t *base_glthread) {
glthread_t *glthreadptr = NULL;

ITERATE_GLTHREAD_BEGIN(base_glthread, glthreadptr) {
remove_glthread(glthreadptr);
}ITERATE_GLTHREAD_END(base_glthread, glthreadptr);
}

void glthread_add_last(glthread_t *base_glthread, glthread_t *new_glthread) {
glthread_t *glthreadptr = NULL, *prevglthreadptr = NULL;
 ITERATE_GLTHREAD_BEGIN(base_glthread, glthreadptr){
        prevglthreadptr = glthreadptr;
    } ITERATE_GLTHREAD_END(base_glthread, glthreadptr);
  
    if(prevglthreadptr) 
        glthread_add_next(prevglthreadptr, new_glthread); 
    else
        glthread_add_next(base_glthread, new_glthread);
}

unsigned int get_glthread_list_count(glthread_t *base_glthread) {
unsigned int count = 0 ; 
glthread_t *glthreadptr = NULL;
ITERATE_GLTHREAD_BEGIN(base_glthread,glthreadptr) {
count++;
} ITERATE_GLTHREAD_END(base_glthread, glthreadptr)
return count ; 
}
