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
  ((glthreadptr)->right==0 && (glthreadptr)->left==0)



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

#define BASE(glthreadptr) ((glthreadptr)->right)


#define ITERATE_GLTHREAD_BEGIN(glthreadptrstart, glthreadptr)                                      \
{                                                                                                  \
    glthread_t *_glthread_ptr = NULL;                                                              \
    glthreadptr = BASE(glthreadptrstart);                                                          \
    for(; glthreadptr!= NULL; glthreadptr = _glthread_ptr){                                        \
        _glthread_ptr = (glthreadptr)->right;

#define ITERATE_GLTHREAD_END(glthreadptrstart, glthreadptr)                                        \
        }}

/*
 *
 *The glthreadptr points to the start of the metadata (not the data).
 *The offset tells you how far back the actual data is located relative to the glthreadptr.The macro uses 
 * this distance (offset) to calculate the address of the data.
 *
 *
 */


#define GLTHREAD_GET_USER_DATA_FROM_OFFSET(glthreadptr,offset) \
  (void *)((char *)(glthreadptr)-offset)

void delete_glthread_list(glthread_t *base_glthread);

unsigned int get_glthread_list_count(glthread_t *base_glthread);

void
glthread_priority_insert(glthread_t *base_glthread,     
                         glthread_t *glthread,
                         int (*comp_fn)(void *, void *),
                         int offset);



/*
 *
 * #if 0 #endif this is a preprocessor directive in c.It disables the code between #if 0 and #endif, effectively commenting it out.
 * The code is ignored during compilation, so the function declaration here is not active.
 *
 *   Searching for a specific item. So we are pointing to the ll, then inside a large struct finding the actual value we care about, the key that u are looking for 
 *   in the list, and last the comparison_fn that will compare and return void since anthing can be returned. 
 *
 */

#if 0
void *
gl_thread_search(glthread_t *base_glthread,
        void *(*thread_to_struct_fn)(glthread_t *),
        void *key,
        int (*comparison_fn)(void *, void *));

#endif

#endif /* __GLUETHREAD__ */
