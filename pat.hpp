#ifndef _PAT_HPP_
#define _PAT_HPP_

#include <vector>
#include <iostream>
#include "ts.hpp"
#include <string.h>
#include <unistd.h>


typedef struct TS_PAT_Program
{
 unsigned program_number    :16;   //节目号
 unsigned reserved_3        : 3; // 保留位
 unsigned program_map_PID   :13;   //节目映射表的PID，节目号大于0时对应的PID，每个节目对应一个
}TS_PAT_Program;


typedef struct TS_PAT
{
    unsigned table_id                     : 8; //固定为0x00 ，标志是该表是PAT
    unsigned section_syntax_indicator     : 1; //段语法标志位，固定为1
    unsigned zero                         : 1; //0
    unsigned reserved_1                   : 2; // 保留位
     unsigned section_length               : 12; //表示这个字节后面有用的字节数，包括CRC32
    unsigned transport_stream_id        : 16; //该传输流的ID，区别于一个网络中其它多路复用的流
    unsigned reserved_2                   : 2;// 保留位
    unsigned version_number               : 5; //范围0-31，表示PAT的版本号
    unsigned current_next_indicator       : 1; //发送的PAT是当前有效还是下一个PAT有效
    unsigned section_number               : 8; //分段的号码。PAT可能分为多段传输，第一段为00，以后每个分段加1，最多可能有256个分段
    unsigned last_section_number          : 8;  //最后一个分段的号码
 
    std::vector<TS_PAT_Program> program;
    unsigned network_PID                    : 13; //网络信息表（NIT）的PID,节目号为0时对应的PID为network_PID
    unsigned CRC_32                        : 32;  //CRC32校验码
} TS_PAT; 

class PAT_PACK{

public:
    PAT_PACK(){};
    ~PAT_PACK(){};
    TS_PAT set_pat_pack;
    void InitPat(uint8_t* buf, uint32_t *len);
    int MakeTable(uint32_t *crc32_table);
    uint32_t crc32(uint8_t *buffer, uint32_t size, uint32_t *crc32_table);

    typedef std::vector<uint8_t> pat_data;
    // int parse_pat(unsigned char *ts_data, unsigned short *pmt_pid);
    int parse_pat(unsigned char *ts_data, std::vector<unsigned short> *pmt_pid);
    void set_pmt_pid(unsigned short pid);
    
private:
    pat_data pat_ts_pack;
    unsigned short _pmt_pid;

};
 



#endif
