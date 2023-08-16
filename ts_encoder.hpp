#ifndef _MUX_MEDIA_HPP_
#define _MUX_MEDIA_HPP_

#include <stdio.h>
#include <iostream>
#include <stdint.h>
#include <map>
#include <list>
#include <vector>

#include "pes.hpp"
#include "pmt.hpp"


using namespace std;

class TsEncoder{
public:
    typedef std::vector<uint8_t> byteArray;
    typedef std::vector<byteArray> TS_PACK_QUEUE;
	TsEncoder(std::string videoCodecType="h264", std::string audioCodecType="aac");
	~TsEncoder();
	TS_PACK_QUEUE encode_video(char* data, int len, uint64_t pts, bool keyFrame);
	TS_PACK_QUEUE encode_audio(char* data, int len, uint64_t pts);	
    void initpat(void);
    void initpmt(void);
    void initts(void);
    void print_data(uint8_t *buf, uint32_t len)
    {
        int i = 0;
        for(i = 0; i < len; i++)
        {
            if(i%16 == 0)
                printf("\n");
            printf("%02x ",buf[i]);
        }
        printf("\n");
    }

    static unsigned short pmt_pid;
    static unsigned short video_pid;
    static unsigned short audio_pid;

private:
	TS_PACK_QUEUE video_ts_pack_queue;
    TS_PACK_QUEUE audio_ts_pack_queue;
    PES_PACK pes_pack;
    PAT_PACK pat_pack;
    PMT_PACK pmt_pack;
    TS_PACK ts_pack;
    unsigned char video_type;
    unsigned char audio_type;
    unsigned short p_pid;
    unsigned short v_pid;
    unsigned short a_pid;
};

#endif
