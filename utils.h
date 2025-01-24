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


void layer2_add_broadcast_address(char *mac_array);


#define IS_MAC_BROADCAST_ADDR(mac)   \
  (mac[0] == 0xff && mac[1] ==  0xff && mac[2] ==  0xff && mac[3] ==  0xff && mac[4] ==  0xff && mac[5] ==  0xff);
#endif /* __UTILS__ */
