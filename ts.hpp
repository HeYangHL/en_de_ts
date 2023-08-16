#ifndef _TS_HPP
#define _TS_HPP


#include <iostream>
#include <fstream>
#include <stdint.h>
#include <vector>
#include "pmt.hpp"
#include "PCR.hpp"



#pragma pack (1)
typedef struct TS_Header{
    unsigned sync_byte                      :8;//同步字节，固定为0x47
    unsigned transport_error_indicator      :1;//错误指示符
    unsigned payload_unit_start_indicator   :1;//负载单元起始标识符
    unsigned transport_priority             :1;//传输优先级，0低优先级 1高优先级
    unsigned pid                            :13;//pid值
    unsigned transport_scrambling_cntrol    :2;//传输加扰控制 00：未加密
    unsigned adaptation_field_control       :2;//是否包含自适应区
    unsigned continuity_counter             :4;//递增计数器
    unsigned adjust_byte                    :8;
}TS_Header;
#pragma pack ()
class TS_PACK
{
public:
    TS_Header ts_head;
    #define PAT 0
    #define PMT 1
    #define MEDIA 2
    #define VIDEO 3
    #define AUDIO 4
    TS_PACK(){printf("this is ts huidiaohanshu!\n");};
    ~TS_PACK();
    void ts_pack(uint8_t *pes, int len, int type, bool keyFrame);
    void init_pat_ts(TS_Header *pat_ts_head);
    void init_pmt_ts(TS_Header *pmt_ts_head);
    void init_media_ts(TS_Header *media_ts_head);
    void init_video_ts(TS_Header *media_ts_head);
    void init_audio_ts(TS_Header *media_ts_head);
    void send_pes_ts_pack(uint8_t *pes, int type, int len, TS_Header pes_head, bool keyFrame);
    void Sumup_Counter(int type);
    bool Is_I_Frame(uint8_t *pes, int len);
    void get_pts_time(long long *pts_time);
    
    typedef std::vector<uint8_t> ts_data;
    typedef std::vector<ts_data> TS_QUEUE;
    TS_QUEUE get_video_queue();
    TS_QUEUE get_audio_queue();
    unsigned short get_pmt_pid();
    unsigned short get_video_pid();
    unsigned short get_audio_pid();
    void clear_queue();
    void set_ts_pid(unsigned short p_pid, unsigned short v_pid, unsigned short a_pid);
    unsigned short get_ts_pid(void){return _pmt_pid;}
    uint8_t *get_pts_val(){return pts_val;}
    ts_data get_pat_ts_pack(){return pat_ts_pack;}
    ts_data get_pmt_ts_pack(){return pmt_ts_pack;}
private:
    uint8_t ts_pack_head[5];
    TS_QUEUE video_queue;
    TS_QUEUE audio_queue;
    ts_data pat_ts_pack;
    ts_data pmt_ts_pack;
    unsigned short _pmt_pid;
    unsigned short _video_pid;
    unsigned short _audio_pid;
    uint8_t pts_val[5] = {0};
    unsigned int  video_index;
    unsigned int  audio_index;
    unsigned int  psi_index;
};


#endif
