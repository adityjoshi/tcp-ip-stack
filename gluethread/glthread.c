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
}
