#include "PCR.hpp"

PCR_PACK PCR_DATA::pcr_pack;
int PCR_DATA::PCR_index = 0;
uint8_t PCR_DATA::PCR_val[6] = {0};

void PCR_DATA::Init_Pcr(void)
{
    PCR_index = 0x041EB0;
    pcr_pack.PCR_base = 0x0;
    pcr_pack.reserved = 0x3F;
    pcr_pack.PCR_ext = 0x0;

    return;
}

void PCR_DATA::Get_Pcr_Val(long long timeUs)
{
    #if 0
    memset(PCR_val, 0, 6);

    PCR_val[0] |= pcr_pack.PCR_base >> 25;
    PCR_val[1] |= pcr_pack.PCR_base >> 17;
    PCR_val[2] |= pcr_pack.PCR_base >> 9;
    PCR_val[3] |= pcr_pack.PCR_base >> 1;
    PCR_val[4] |= pcr_pack.PCR_base << 7;
    PCR_val[4] |= pcr_pack.reserved << 1;
    PCR_val[4] |= pcr_pack.PCR_ext >> 8;
    PCR_val[5] = pcr_pack.PCR_ext;

    printf("PCR base = %02x, index = %02x\n", pcr_pack.PCR_base, PCR_index);

    std::cout << "PCR data is :"  << std::endl;
    
    for(int i = 0; i < 6; i++)
    {
        if(i%16 == 0)
            printf("\n");
        printf("%02x ", PCR_val[i]);
    }
    printf("\n");
    if(pcr_pack.PCR_base == 0)
        pcr_pack.PCR_base = 0x417A8;
    else
        pcr_pack.PCR_base += PCR_index;
    printf("2 pcr base = %02x\n", pcr_pack.PCR_base);
#endif
    float tmp = timeUs / 1000000 + (timeUs % 1000000) / 1000 * 0.001;
    long long pcr = (long long)(tmp * 27000000.0);
    long long pcr_low = pcr % 300LL;

    long long pcr_high = pcr / 300LL;

    PCR_val[0] = pcr_high>>25;
    PCR_val[1] = pcr_high>>17;
    PCR_val[2] = pcr_high>>9;
    PCR_val[3] = pcr_high>>1;
    PCR_val[4] = pcr_high<<7 | pcr_low>>8 | 0x7e;
    PCR_val[5] = pcr_low;

    return;
;}