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
#define UNSET_BIT(n, pos)       (n = n & ((1 << pos) ^ 0xFFFFFFFF))


void layer2_add_broadcast_address(char *mac_array);


#define IS_MAC_BROADCAST_ADDR(mac)   \
    (mac[0] == 0xFF  &&  mac[1] == 0xFF && mac[2] == 0xFF && \
     mac[3] == 0xFF  &&  mac[4] == 0xFF && mac[5] == 0xFF)
#endif /* __UTILS__ */
