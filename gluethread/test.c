#include "gltthread.h"
#include "stdlib.h"
#include "stdio.h"
#include "memory.h"



typedef struct _person {
int age ; 
int weight ;
glthread_t glthread;
} person_t;



int senior_citizen(person_t *p1, person_t *p2) {
if(p1->age == p2->age) return 0;
if(p1->age < p2->age) return 1 ; 
return - 1; 
}

#define offset(struct_name, fld_name) \
    (unsigned int)&(((struct_name *)0)->fld_name)

GLTHREAD_TO_STRUCT(thread_to_person, person_t, glthread);


