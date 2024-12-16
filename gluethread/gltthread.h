#ifndef __GLUETHREAD__
#define __GLUETHREAD__


typedef struct _glthread {
  struct _glthread *left;
  struct _glthread *right;
} glthread_t;


void glthread_add_next(glthread_t *base_glthread, glthread_t *new_glthread);

void glthread_add_before(glthread_t *base_glthread, glthread_t *new_glthread);

void remove_glthread(glthread_t *glthread);

/*
 it basically resets the right and left node to the null ; 
 */
void init_glthread(glthread_t *glthread);


void glthread_add_last(glthread_t *base_glthread, glthread_t *new_glthread);

/*
 *
 *
 *
 *    MACROS creation for the common operations 
 *
 *
 *
 *
 */

#define IS_GLTHREAD_LIST_EMPTY(glthreadptr) \
  ((glthreadptr)->right==0 && (glthreadptr)->left==0);



/*
 *
 *   This marco is used for the finding of the parent structure so suppose if we ever call the 
 *   pointer glthread_t it will tell us inside which big struct it is getting used 
 *
 *   We are subtracting the address of the field glthread_node inside the larger structure (not the smaller structure) 
 *   from the pointer to the smaller structure (glthreadptr). This gives us the address of the entire larger structure.
 *
 */

#define GLTHREAD_TO_STRUCT(fn_name,structure_name,field_name) \
  static inline structure_name * fn_name(glthread_t *glthreadptr) { \
    return (structure_name *)((char *)(glthreadptr) - (char *)&(((structure_name *)0)->field_name)) ; \
  }


#endif /* __GLUETHREAD__ */
