#include "ts_decoder.hpp"

int TsDecoder::decode(TsDecoder::PACK_T i_pkt, TsDecoder::PACK_T &o_frame, uint64_t &o_pts, std::string &o_codecType)
{
    unsigned int frame_len;
    unsigned char ts[188] = {0};

    int i = 0;
    int ret = 0;
    int count = 0;
#if 1
    // if (i_pkt.at(0) != 0x47)
    // {
    if ((ts_counts < 6) && (startflag == 0))
    {

        memcpy(ts_data + (ts_counts * 188), i_pkt.data(), 188);
        ts_counts++;
        return 0;
    }
    if ((ts_counts >= 6) && (startflag == 1))
    {
        parse_counts--;
        memcpy(ts_data + ((ts_counts - 1) * 188), i_pkt.data(), 188);
    }
    if ((ts_counts >= 6) && (startflag == 0))
    {
        for (i = 0; i < 188; i++)
        {
            if (ts_data[i] == 0x47)
            {
                for (int j = 1; j < 6; j++)
                {
                    if (((i + (j * 188)) < 1128) && (ts_data[i + j * 188] == 0x47))
                        count++;
                }
                if (count == 5)
                {
                    startflag = 1;
                    offset = i;
                    break;
                }
            }
        }
        if (i == 188)
        {
            ts_counts = 0;
            memset(ts_data, 0, sizeof(ts_data));
            return -1;
        }
#if 0
        if (i >= 0 && i < 188)
        {
            printf("get 188 ts data!\n");
            memcpy(ts, &ts_data[offset], 188);
            memmove(ts_data, &ts_data[188], (sizeof(ts_data) - 188));
        }
#endif
    }
    else if ((ts_counts >= 6) && (startflag == 1))
    {
        while(ts_counts - parse_counts)
        {
            memcpy(ts, &ts_data[offset], 188);
            memmove(ts_data, &ts_data[188], (sizeof(ts_data) - 188));
            parse_counts++;
            frame_len = parse_ts_pack(ts, o_frame, o_pts, o_codecType);
            if(frame_len > 0)
                return frame_len;
        }

    }

    // }
    // else if(i_pkt.at(0) == 0x47)
    // {
    //     for (i = 0; i < i_pkt.size(); i++)
    //         ts[i] = i_pkt.at(i);
    // }
    
#endif
#if 0    
    ret = pat_pack.parse_pat(i_pkt, unsigned &_pmt_pid);
    if(ret == 0)
    {
        pmt_pack.parse_set_pmt_pid(_pmt_pid);
        ret = pmt_pack.parse_pmt(unsigned char * ts_data, &_video_pid, &_audio_pid, &_video_type, &_audio_type);
        if(ret == 0)
        {
            pes_pack.parse_set_pes_pid(_video_pid, _audio_pid);
            pes_pack.parse_pes_head(ts);
        }
    }
#endif
    // frame_len = parse_ts_pack(ts, o_frame, o_pts, o_codecType);

    return frame_len;
}
int TsDecoder::parse_ts_pack(unsigned char *ts, TsDecoder::PACK_T &o_frame, uint64_t &o_pts, std::string &o_codecType)
{
    int ret = 0;
    std::vector<uint8_t> return_frame;

    if (pat_flag == 0)
    {
        // sleep(2);
        ret = pat_pack.parse_pat(ts, &_pmt_pid);
        if (ret == 0)
        {
            // for (auto item : _pmt_pid)
            // {
            //     printf("pmt pid : %d\n", item);
            // }
            // sleep(10);
            pat_flag = 1;
            return 0;
        }
    }
    if (pat_flag == 1 && pmt_flag == 0)
    {
        pmt_pack.parse_set_pmt_pid(_pmt_pid);
        ret = pmt_pack.parse_pmt(ts, &_video_pid, &_audio_pid, &_video_type, &_audio_type);
        if (ret == 0)
        {
            // printf("pmt : %02x %02x %02x %02x\n", ts[0], ts[1], ts[2], ts[3]);
            // printf("video pid = %02x, audio pid = %02x, type : %02x, %02x\n", _video_pid, _audio_pid, _video_type, _audio_type);
            // sleep(2);
            pmt_flag = 1;
            return 0;
        }
    }
    if (pat_flag == 1 && pmt_flag == 1)
    {
        // printf("%02x %02x %02x %02x\n", ts[0], ts[1], ts[2], ts[3]);
        pes_pack.parse_set_pes_pid(_video_pid, _audio_pid, _video_type, _audio_type);
        ret = pes_pack.parse_pes_head(ts);
        if (ret < 0)
            return -1;
    }
    if (pes_pack.get_video_media_flag() == 2)
    {
        
        o_frame.clear();
        media_frame = pes_pack.get_video_media_frame();

        return_frame.insert(return_frame.end(), media_frame.begin(), media_frame.end());
        o_frame.insert(o_frame.end(), return_frame.begin(), return_frame.end());
        o_pts = pes_pack.get_return_pts();
        pes_pack.set_video_media_flag(1);
        if (pes_pack.get_return_type() == 0x1b)
            o_codecType = "h264";
        if (pes_pack.get_return_type() == 0x24)
            o_codecType = "h265";
        if (pes_pack.get_return_type() == 0x0f)
            o_codecType = "aac";

        pes_pack.clear_video_media_frame();

        return return_frame.size();
    }
    else if (pes_pack.get_audio_media_flag() == 2)
    {
        // printf("start write media data to file!\n");
        o_frame.clear();
        media_frame = pes_pack.get_audio_media_frame();

        return_frame.insert(return_frame.end(), media_frame.begin(), media_frame.end());
        o_frame.insert(o_frame.end(), return_frame.begin(), return_frame.end());
        o_pts = pes_pack.get_return_pts();
        pes_pack.set_audio_media_flag(1);
        // printf("media type : %02x\n", pes_pack.get_return_type());
        if (pes_pack.get_return_type() == 0x1b)
            o_codecType = "h264";
        if (pes_pack.get_return_type() == 0x24)
            o_codecType = "h265";
        if (pes_pack.get_return_type() == 0x0f)
            o_codecType = "aac";

        pes_pack.clear_audio_media_frame();
        // printf("return frame size : %d\n", return_frame.size());
        return return_frame.size();
    }

    return 0;
}