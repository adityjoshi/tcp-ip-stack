#include "utils.h"
#include <stdint.h>
#include <string.h>
#include <sys/_endian.h>
#include <arpa/inet.h>


void apply_mask(char *prefix, char mask, char *str_prefix) {

uint32_t binary_prefix = 0 ;
/*
* 0x is for hexa decimal format and f is 1111 so basically its 255.255.255.255
*/
uint32_t subnetMask = 0xffffffff; 

if (mask == 32) {
strncpy(str_prefix,prefix,16);
str_prefix[15] = '\0';
return ; 
}

inet_pton(AF_INET, prefix, &binary_prefix);
binary_prefix = htonl(binary_prefix);
}
