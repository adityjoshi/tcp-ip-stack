#ifndef __UTILS__
#define __UTILS__



typedef enum {
FALSE, 
TRUE
}  bool_t ;

/*
 * apply_mask is used to find the subnet mast by performing the and opertation and storing it in the str_max
 *
 * */
void apply_mask(char *prefix, char mask, char *str_max) ;


void layer2_fill_with_broadcast_mac(char *mac_array);


#endif /* __UTILS__ */
