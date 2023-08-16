#include "ts_encoder.hpp"

unsigned short TsEncoder::pmt_pid = 0x1000;
unsigned short TsEncoder::video_pid = 0x100;
unsigned short TsEncoder::audio_pid = 0x101;

void TsEncoder::initpat(void)
{
    pat_pack.set_pmt_pid(p_pid);
}
void TsEncoder::initpmt(void)
{
    pmt_pack.set_pmt_pid(v_pid, a_pid, video_type, audio_type);
}

void TsEncoder::initts(void)
{
    ts_pack.set_ts_pid(p_pid, v_pid, a_pid);
}


TsEncoder::TsEncoder(std::string videoCodecType, std::string audioCodecType)
{
    p_pid = pmt_pid++;
    v_pid = video_pid++;
    a_pid = audio_pid++;
    if(videoCodecType == "h264")
        video_type = 0x1b;
    if(videoCodecType == "h265")
        video_type = 0x24;
    if(audioCodecType == "aac")
        audio_type = 0xf;
    
    initpat();
    initpmt();
    initts();
}
TsEncoder::~TsEncoder()
{
    for(auto item : video_ts_pack_queue){
        item.clear();
    }
    video_ts_pack_queue.clear();

    for(auto item : audio_ts_pack_queue){
        item.clear();
    }
    audio_ts_pack_queue.clear();
}

TsEncoder::TS_PACK_QUEUE TsEncoder::encode_video(char* data, int len, uint64_t pts, bool keyFrame)
{
    uint8_t pat_data[188] = {0};
    uint32_t pat_len = 0;
    uint8_t pmt_data[188] = {0};
    uint32_t pmt_len = 0;
    uint8_t pes_data[1024*1024] = {0};
    uint32_t pes_len = 0;
    byteArray pat_ts;
    byteArray pmt_ts;
    int i = 0;
    
    pat_ts.clear();
    pmt_ts.clear();
    
    pat_pack.InitPat(pat_data, &pat_len);
    ts_pack.ts_pack(pat_data, pat_len, PAT, false);
    pat_ts = ts_pack.get_pat_ts_pack();
  
    pmt_pack.InitPmt(pmt_data, &pmt_len);
    ts_pack.ts_pack(pmt_data, pmt_len, PMT, false);
    pmt_ts = ts_pack.get_pmt_ts_pack();
    
    
    pes_pack.Set_Pes_Pack((unsigned char *)data, pts, len, VIDEO, pes_data, &pes_len);
    ts_pack.ts_pack((uint8_t *)pes_data, pes_len, VIDEO, keyFrame);
    
    pes_pack.set_pts_dts(pts, ts_pack.get_pts_val());
    video_ts_pack_queue = ts_pack.get_video_queue();

    video_ts_pack_queue.insert(video_ts_pack_queue.begin(), pmt_ts);
    video_ts_pack_queue.insert(video_ts_pack_queue.begin(), pat_ts);
    
    return video_ts_pack_queue;
}

TsEncoder::TS_PACK_QUEUE TsEncoder::encode_audio(char* data, int len, uint64_t pts)
{
    uint8_t pat_data[188] = {0};
    uint32_t pat_len = 0;
    uint8_t pmt_data[188] = {0};
    uint32_t pmt_len = 0;
    uint8_t pes_data[1024*1024] = {0};
    uint32_t pes_len = 0;
    int i = 0;
    uint8_t *pts_test = NULL;

    pes_pack.Set_Pes_Pack((unsigned char *)data, pts, len, AUDIO, pes_data, &pes_len);
    ts_pack.ts_pack((uint8_t *)pes_data, pes_len, AUDIO, false);

    pes_pack.set_pts_dts(pts, ts_pack.get_pts_val());
//    pts_test = ts_pack.get_pts_val();
//    printf("audio pts :\n");
//    printf("%02x %02x %02x %02x %02x\n", pts_test[0], pts_test[1], pts_test[2], pts_test[3], pts_test[4], pts_test[5]);
//    audio_ts_pack_queue = pes_pack.get_ts_pack()->get_audio_queue();
    audio_ts_pack_queue = ts_pack.get_audio_queue();


    return audio_ts_pack_queue;
}

