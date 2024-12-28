# GlueThread Implementation

## Introduction

The GlueThread implementation provides a set of functions for managing a doubly linked list of nodes, utilizing the `glthread_t` structure. It includes operations such as adding nodes, removing nodes, counting nodes, and inserting nodes with priority. This implementation supports dynamic insertion, deletion, and traversal, making it useful for managing data in various applications.

## Functions

### 1. **`void init_glthread(glthread_t *glthread)`**
   - Initializes the `glthread` by setting its `left` and `right` pointers to `NULL`.
   
### 2. **`void glthread_add_next(glthread_t *curr_glthread, glthread_t *new_glthread)`**
   - Adds `new_glthread` as the next node after `curr_glthread` in the linked list.
   - If `curr_glthread` is the last node, `new_glthread` will be appended.

### 3. **`void glthread_add_before(glthread_t *curr_glthread, glthread_t *new_glthread)`**
   - Inserts `new_glthread` before `curr_glthread` in the list.
   - If `curr_glthread` is the first node, `new_glthread` will be placed at the beginning.

### 4. **`void remove_glthread(glthread_t *curr_glthread)`**
   - Removes the `curr_glthread` from the list by updating the neighboring nodes (`left` and `right` pointers).
   - Handles cases when the node is at the beginning or end of the list.

### 5. **`void delete_glthread_list(glthread_t *base_glthread)`**
   - Deletes all nodes in the linked list starting from `base_glthread`.
   - Iterates through the list and removes each node.

### 6. **`void glthread_add_last(glthread_t *base_glthread, glthread_t *new_glthread)`**
   - Adds `new_glthread` to the end of the list starting from `base_glthread`.
   - If the list is empty, it adds the node to the beginning.

### 7. **`unsigned int get_glthread_list_count(glthread_t *base_glthread)`**
   - Returns the total number of nodes in the list starting from `base_glthread`.
   - Iterates through the list and counts the nodes.

### 8. **`void glthread_priority_insert(glthread_t *base_glthread, glthread_t *glthread, int (*comp_fn)(void *, void *), int offset)`**
   - Inserts `glthread` into the list based on a priority comparison function (`comp_fn`).
   - The list is traversed, and the correct position for `glthread` is found according to the comparison function.

## Macros

### 1. **`IS_GLTHREAD_LIST_EMPTY(glthreadptr)`**
   - Checks if a list is empty by verifying both `left` and `right` pointers of the node are `NULL`.

### 2. **`GLTHREAD_GET_USER_DATA_FROM_OFFSET(glthreadptr, offset)`**
   - Retrieves the user data from a node by applying an offset to the `glthread` pointer.
   
### 3. **`ITERATE_GLTHREAD_BEGIN(glthreadptrstart, glthreadptr)` and `ITERATE_GLTHREAD_END(glthreadptrstart, glthreadptr)`**
   - Macros for iterating through the list starting from `glthreadptrstart`.

## Preprocessor Directive

### **`#if 0 ... #endif`**
   - The `gl_thread_search` function is commented out using this directive.
   - This function is intended for searching within the list using a comparison function and key.

---

The `glthread.c` file provides the implementation of the functions defined in `glthread.h`, allowing for flexible and efficient manipulation of doubly linked lists. It includes basic operations for node insertion, removal, traversal, and list management with additional support for priority-based insertion.
