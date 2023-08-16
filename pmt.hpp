#ifndef _PMT_HPP_
#define _PMT_HPP_

#include <vector>
#include <iostream>
#include "pat.hpp"
#include <string.h>



typedef struct TS_PMT_Stream
{
    unsigned stream_type     : 8; //指示特定PID的节目元素包的类型。该处PID由elementary PID指定
    unsigned reserved_5      :3;
    unsigned elementary_PID  : 13; //该域指示TS包的PID值。这些TS包含有相关的节目元素
    unsigned reserved_6      :4;    
    unsigned ES_info_length  : 12; //前两位bit为00。该域指示跟随其后的描述相关节目元素的byte数
}TS_PMT_Stream;


typedef struct TS_PMT
{
    unsigned table_id                   : 8; //固定为0x02, 表示PMT表
    unsigned section_syntax_indicator    : 1; //固定为0x01
    unsigned zero                       : 1; //0x01
    unsigned reserved_1                 : 2; //0x03
    unsigned section_length : 12;//首先两位bit置为00，它指示段的byte数，由段长度域开始，包含CRC 
    unsigned program_number             : 16;// 指出该节目对应于可应用的Program map PID
    unsigned reserved_2                 : 2; //0x03
    unsigned version_number             : 5; //指出TS流中Program map section的版本号
    unsigned current_next_indicator  : 1; //当该位置1时，当前传送的Program map section可用
     //当该位置0时，指示当前传送的Program map section不可用，下一个TS流的Program map section有效
    unsigned section_number            : 8; //固定为0x00
    unsigned last_section_number      : 8; //固定为0x00
    unsigned reserved_3               : 3; //0x07
    unsigned PCR_PID                   : 13; //指明TS包的PID值，该TS包含有PCR域，
      //该PCR值对应于由节目号指定的对应节目，如果对于私有数据流的节目定义与PCR无关，这个域的值将为0x1FFF。
    unsigned reserved_4            : 4;  //预留为0x0F
    unsigned program_info_length  : 12; //前两位bit为00。该域指出跟随其后对节目信息的描述的byte数。
    
    std::vector<TS_PMT_Stream> PMT_Stream;  //每个元素包含8位, 指示特定PID的节目元素包的类型。该处PID由elementary PID指定
    unsigned CRC_32                    : 32; 
} TS_PMT; 

class PMT_PACK{
public:
    TS_PMT set_pmt_pack;
    void InitPmt(uint8_t *buf, uint32_t *len);

    typedef std::vector<uint8_t> pmt_data;
    int parse_pmt(unsigned char *ts_data, unsigned short *video_pid, unsigned short *audio_pid, unsigned char *video_type, unsigned char *audio_type);
    void set_pmt_pid(unsigned short v_pid, unsigned short a_pid, unsigned char v_type, unsigned char a_type);
    void parse_set_pmt_pid(std::vector<unsigned short> pmt_pid){_pmt_pid = pmt_pid;}

private:
    pmt_data pmt_ts_pack;
    unsigned short _video_pid;
    unsigned short _audio_pid;
    std::vector<unsigned short> _pmt_pid;
    unsigned char _video_type;
    unsigned char _audio_type;
};


#endif
