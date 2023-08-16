#ifndef _PCR_HPP_
#define _PCR_HPP_

#include <stdio.h>
#include <iostream>
#include <stdint.h>
#include <string.h>


typedef struct PCR_PACK{
	unsigned PCR_base	:33;
	unsigned reserved	:6;
	unsigned PCR_ext	:9;
}PCR_PACK;

class PCR_DATA{
public:
    PCR_DATA(){};
    ~PCR_DATA(){};
	static PCR_PACK pcr_pack;
    static int PCR_index;
    static uint8_t PCR_val[6];
    void Get_Pcr_Val(long long pts_time);
    void Init_Pcr(void);
};


#endif
