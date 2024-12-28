# GlueThread Header

## Introduction

The Glue Thread header defines a double-linked list structure designed for flexible and efficient data management. It includes utility functions and macros for manipulating threads (nodes) within the list. This header is versatile and supports advanced operations like priority insertion and data offset handling, making it ideal for various programming tasks.

## Features

- Bidirectional navigation of nodes.
- Support for dynamic node insertion and removal.
- Macros for common operations and data manipulation.
- Customizable priority-based insertion.

## Implementation Details

### Structures and Typedefs

- **`glthread_t`**: 
  - A structure representing a node in the linked list.
  - Contains `left` and `right` pointers for bidirectional navigation.

### Functions

1. **`void glthread_add_next(glthread_t *base_glthread, glthread_t *new_glthread)`**:
   - Adds `new_glthread` as the next node to `base_glthread`.

2. **`void glthread_add_before(glthread_t *base_glthread, glthread_t *new_glthread)`**:
   - Inserts `new_glthread` before `base_glthread` in the list.

3. **`void remove_glthread(glthread_t *glthread)`**:
   - Removes a specific node (`glthread`) from the list.

4. **`void init_glthread(glthread_t *glthread)`**:
   - Resets the `left` and `right` pointers of the node to `NULL`.

5. **`void glthread_add_last(glthread_t *base_glthread, glthread_t *new_glthread)`**:
   - Appends `new_glthread` to the end of the list starting from `base_glthread`.

6. **`void delete_glthread_list(glthread_t *base_glthread)`**:
   - Deletes all nodes in the list starting from `base_glthread`.

7. **`unsigned int get_glthread_list_count(glthread_t *base_glthread)`**:
   - Returns the total number of nodes in the list starting from `base_glthread`.

8. **`void glthread_priority_insert(glthread_t *base_glthread, glthread_t *glthread, int (*comp_fn)(void *, void *), int offset)`**:
   - Inserts `glthread` into the list based on a priority comparison function.

### Macros

1. **`IS_GLTHREAD_LIST_EMPTY(glthreadptr)`**:
   - Checks if a list is empty by verifying both `left` and `right` pointers are `NULL`.

2. **`GLTHREAD_TO_STRUCT(fn_name, structure_name, field_name)`**:
   - Retrieves the parent structure containing a `glthread_t` field.
   - Useful for navigating from a node to its enclosing structure.

3. **`BASE(glthreadptr)`**:
   - Points to the `right` pointer of a node, indicating the next node.

4. **`ITERATE_GLTHREAD_BEGIN(glthreadptrstart, glthreadptr)` and `ITERATE_GLTHREAD_END(glthreadptrstart, glthreadptr)`**:
   - Macros for iterating through the list starting from `glthreadptrstart`.
   - Allows for clean and concise traversal of the list.

5. **`GLTHREAD_GET_USER_DATA_FROM_OFFSET(glthreadptr, offset)`**:
   - Calculates the user data address based on the node pointer and an offset value.

### Preprocessor Directive

- **`#if 0 ... #endif`**:
   - The `gl_thread_search` function is commented out using this directive.
   - Intended for searching within the list using a comparison function and key.

`
The glthread.h file provides the basic building blocks for creating and manipulating doubly linked lists using the glthread_t structure.
It includes function declarations for adding, removing, and initializing nodes in the list, as well as placeholders for common operation macros.
`
