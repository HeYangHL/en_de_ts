#include "ts.hpp"
#include "pes.hpp"

#include <stdio.h>

using namespace std;


void TS_PACK::set_ts_pid(unsigned short p_pid, unsigned short v_pid, unsigned short a_pid)
{
    
    _pmt_pid = p_pid;
    _video_pid = v_pid;
    _audio_pid = a_pid;
}



TS_PACK::~TS_PACK()
{
    for(auto item : video_queue){
        item.clear();
    }
    video_queue.clear();
    
    for(auto item : audio_queue){
        item.clear();
    }
    audio_queue.clear();

}


bool TS_PACK::Is_I_Frame(uint8_t * pes, int len)
{
    int i = 0;

    for(i = 0; (i+3) < len; i++)
    {
        if(pes[i] == 0x00 && pes[i+1] == 0x00 && pes[i+2] == 0x00 && pes[i+3] == 0x01)
        {            
            if((pes[i+4]&0x1F) == 5)
            {
                return true;
            }
            if((pes[i+4]&0x1F) == 1)
                break;
        }

            
    }
    return false;
}

void TS_PACK::init_pat_ts(TS_Header * media_ts_head)
{
	media_ts_head->sync_byte = 0x47;
	media_ts_head->transport_error_indicator = 0;
	media_ts_head->payload_unit_start_indicator = 0x1;
	media_ts_head->transport_priority = 0x0;
	media_ts_head->pid = 0x0;
	media_ts_head->transport_scrambling_cntrol = 0;
	media_ts_head->adaptation_field_control = 0x1;
	media_ts_head->continuity_counter = (psi_index&0xF);
	media_ts_head->adjust_byte = 0;
}

void TS_PACK::init_pmt_ts(TS_Header * media_ts_head)
{

	media_ts_head->sync_byte = 0x47;
	media_ts_head->transport_error_indicator = 0;
	media_ts_head->payload_unit_start_indicator = 0x1;
	media_ts_head->transport_priority = 0x0;
	media_ts_head->pid = _pmt_pid;
	media_ts_head->transport_scrambling_cntrol = 0;
	media_ts_head->adaptation_field_control = 0x1;
	media_ts_head->continuity_counter = (psi_index&0xF);
	media_ts_head->adjust_byte = 0;
}

void TS_PACK::init_video_ts(TS_Header * media_ts_head)
{  

	media_ts_head->sync_byte = 0x47;
	media_ts_head->transport_error_indicator = 0;
	media_ts_head->payload_unit_start_indicator = 0x0;
	media_ts_head->transport_priority = 0;
    media_ts_head->pid = _video_pid; 
	media_ts_head->transport_scrambling_cntrol = 0;
	media_ts_head->adaptation_field_control = 0x1;
//    std::cout << "video index : " << media_index << std::endl;
//	media_ts_head->continuity_counter = (media_index & 0x000F);
	//    media_ts_head->adjust_byte = 0;
//	std::cout << "continuity_counter : " << media_ts_head->continuity_counter << std::endl;
}

void TS_PACK::init_audio_ts(TS_Header * media_ts_head)
{
	media_ts_head->sync_byte = 0x47;
	media_ts_head->transport_error_indicator = 0;
	media_ts_head->payload_unit_start_indicator = 0x0;
	media_ts_head->transport_priority = 0;
    media_ts_head->pid = _audio_pid;
	media_ts_head->transport_scrambling_cntrol = 0;
	media_ts_head->adaptation_field_control = 0x1;
//	media_ts_head->continuity_counter = (media_index & 0x000F);
	//    media_ts_head->adjust_byte = 0;
}

void TS_PACK::get_pts_time(long long *pts_time)
{

//    printf("ts pts :\n");
//    printf("%02x %02x %02x %02x %02x\n", pts_val[0], pts_val[1] ,pts_val[2],pts_val[3],pts_val[4]);
    long long PTS = (pts_val[4]>>1) | (pts_val[3]<<7) | (pts_val[2]>>1)<<15 | pts_val[1]<<22 | (pts_val[0]>>1)<<30;
    long long TIMEUS = PTS * 100LL / 9LL;
    *pts_time = TIMEUS;

    return;
}

TS_PACK::TS_QUEUE TS_PACK::get_video_queue()
{
    return video_queue;
}
TS_PACK::TS_QUEUE TS_PACK::get_audio_queue()
{
    return audio_queue;
}

void TS_PACK::clear_queue()
{
    for(auto item : video_queue){
        item.clear();
    }
    video_queue.clear();

    for(auto item : audio_queue){
        item.clear();
    }
    audio_queue.clear();
}

void TS_PACK::send_pes_ts_pack(uint8_t *pes, int type, int len, TS_Header pes_head, bool keyFrame)
{

	int i = 0;
    int a_flag = 0;
    int v_i_flag = 0;
    char adaptation = 0x0;

    PCR_DATA pcr_ts;
    long long pts_time = 0;

    ts_data ts_pack;

    get_pts_time(&pts_time);

    clear_queue();

	while(len != 0)
	{
		if(type == PAT || type == PMT)
		{
            pat_ts_pack.clear();
            pmt_ts_pack.clear();
			if(len > 183)
			{
#if 0                
				fin.write((const char *)ts_pack_head, 5);
				fin.write((const char *)pes, 183);
#endif
                if(type == PAT)
                {
                    for(i = 0; i < 5; i++)
                        pat_ts_pack.push_back(ts_pack_head[i]);
                    pat_ts_pack.insert(ts_pack.end(), pes, (pes+183));
                }
                if(type == PMT)
                {
                    for(i = 0; i < 5; i++)
                        pmt_ts_pack.push_back(ts_pack_head[i]);
                    pmt_ts_pack.insert(ts_pack.end(), pes, (pes+183));
                }
				len -= 183;
			}
			else if(len <= 183)
			{
#if 0
				fin.write((const char *)ts_pack_head, 5);
				fin.write((const char *)pes, len);
				for(i = 0; i < (183-len); i++)
				{
					fin.put(0xff);
				}
				len = 0;
#endif 

                if(type == PAT)
                {
                    for(i = 0; i < 5; i++)
                        pat_ts_pack.push_back(ts_pack_head[i]);
                    pat_ts_pack.insert(pat_ts_pack.end(), pes, (pes+len));
#if 0                    
                    printf("pat_ts_pack ts pack : size = %d\n", pat_ts_pack.size());
                    for(i = 0; i < pat_ts_pack.size(); i++)
                    {
                        if(i%16 == 0)
                            printf("\n");
                        printf("%02x ", pat_ts_pack.at(i));
                    }
                    printf("\n");
#endif
                }
                if(type == PMT)
                {
                    for(i = 0; i < 5; i++)
                        pmt_ts_pack.push_back(ts_pack_head[i]);
                    pmt_ts_pack.insert(pmt_ts_pack.end(), pes, (pes+len));
                }
                len = 0;
			}
		}
		else if(type == VIDEO)
		{        
            ts_pack_head[3] &= 0xf0;
            pes_head.continuity_counter = (video_index & 0x000F);

			if(len >= 184)
			{
            #if 1
                if(keyFrame && (v_i_flag == 0))
                {
                    ts_pack.clear();
                    pes_head.adaptation_field_control = 0;
                    ts_pack_head[3] &= (pes_head.adaptation_field_control) << 4;
                    pes_head.payload_unit_start_indicator = 0x1;
                    ts_pack_head[1] |= (pes_head.payload_unit_start_indicator)<< 6;
                    pes_head.adaptation_field_control = 0x3;
                    ts_pack_head[3] |= (pes_head.adaptation_field_control) << 4;
                    ts_pack_head[3] |= pes_head.continuity_counter;
                    v_i_flag = 1;
#if 0                    
                    fin.write((const char *)ts_pack_head, 4);
                    std::cout << "this is I frame !" << std::endl;
                    fin.put(0x07);
                    fin.put(0x50);
                    pcr_ts.Get_Pcr_Val(pts_time);
                    fin.write((const char *)pcr_ts.PCR_val, 6);
#endif
                    //change by yh 2022/1/3
//                    ts_pack.insert(ts_pack.end(), &ts_pack_head[0], &ts_pack_head[4]);
                    for(i = 0; i < 4; i++)
                        ts_pack.push_back(ts_pack_head[i]);
                    ts_pack.push_back(0x07);
                    ts_pack.push_back(0x50);
                    pcr_ts.Get_Pcr_Val(pts_time);
//                    ts_pack.insert(ts_pack.end(), &pcr_ts.PCR_val[0], &pcr_ts.PCR_val[6]);
                    for(i = 0; i < 6; i++)
                        ts_pack.push_back(pcr_ts.PCR_val[i]);
#if 0
				    fin.write((const char *)pes, 176);
#endif
                    //change by yh 2022/1/3
                    ts_pack.insert(ts_pack.end(), pes, (pes+176));
                    
                    video_queue.push_back(ts_pack);
                    
				    len -= 176;
                    pes += 176; 
                    
                }
                else
                {
                    ts_pack.clear();
                    pes_head.adaptation_field_control = 0;
                    ts_pack_head[3] &= (pes_head.adaptation_field_control) << 4;
                    pes_head.payload_unit_start_indicator = 0x0;
                    ts_pack_head[1] &= (pes_head.payload_unit_start_indicator)<< 6;
                    ts_pack_head[1] |= (pes_head.transport_priority) << 5;
	                ts_pack_head[1] |= (pes_head.pid) >> 8;
                    pes_head.adaptation_field_control = 0x1;
                    ts_pack_head[3] |= (pes_head.adaptation_field_control) << 4;

                    ts_pack_head[3] |= pes_head.continuity_counter;


                    if(v_i_flag == 0)
                    {
                        pes_head.payload_unit_start_indicator = 0x1;
                        ts_pack_head[1] |= (pes_head.payload_unit_start_indicator)<< 6;
                        v_i_flag = 1;
                    }

                    //change by yh 2022/1/3                    
#if 0                    
				    fin.write((const char *)ts_pack_head, 4);
				    fin.write((const char *)pes, 184);
#endif
//                    ts_pack.insert(ts_pack.end(), &ts_pack_head[0], &ts_pack_head[4]);
                    for(i = 0; i < 4; i++)
                        ts_pack.push_back(ts_pack_head[i]);
                    ts_pack.insert(ts_pack.end(), pes, (pes+184));
                    
                    video_queue.push_back(ts_pack);

				    len -= 184;
                    pes += 184; 
                }
             #endif
             
			}
			else if(len < 184)
			{
                ts_pack.clear();
                pes_head.adaptation_field_control = 0;
                ts_pack_head[3] &= (pes_head.adaptation_field_control) << 4;
                
                pes_head.payload_unit_start_indicator = 0x0;
                ts_pack_head[1] &= (pes_head.payload_unit_start_indicator)<< 6;
                ts_pack_head[1] |= (pes_head.transport_priority) << 5;
	            ts_pack_head[1] |= (pes_head.pid) >> 8;
                
                pes_head.adaptation_field_control = 0x3;
                ts_pack_head[3] |= (pes_head.adaptation_field_control) << 4;
                ts_pack_head[3] |= pes_head.continuity_counter;
                if(v_i_flag == 0)
                {
                    pes_head.payload_unit_start_indicator = 0x1;
                    ts_pack_head[1] |= (pes_head.payload_unit_start_indicator)<< 6;
                    v_i_flag = 1;
                }

                //change by yh 2022/1/3
//				fin.write((const char *)ts_pack_head, 4);
//                ts_pack.insert(ts_pack.end(), ts_pack_head[0], ts_pack_head[3]);
                for(i = 0; i < 4; i++)
                        ts_pack.push_back(ts_pack_head[i]);

                
                adaptation = (char)(184 - len -1);

//change by yh 2022/1/3
#if 0                
                fin.put(adaptation);
                if(adaptation != 0x0)
                    fin.put(0x0);
#endif
                ts_pack.push_back(adaptation);
                if(adaptation != 0x0)
                    ts_pack.push_back(0x0);

				for(i = 0; i < (182-len); i++)
				{   
                    ////change by yh 2022/1/3
//                  fin.put(0xFF);                    
				    ts_pack.push_back(0xff);
				}
                //change by yh 2022/1/3
//                fin.write((const char *)pes, len);
                ts_pack.insert(ts_pack.end(), pes, (pes+len));
                
                video_queue.push_back(ts_pack);

				len = 0;
			}

            
		}
        else if(type == AUDIO)
        {
            ts_pack_head[3] &= 0xf0;
            pes_head.continuity_counter = (audio_index & 0x000F);
            

//			ffmpeg.print_data((unsigned char *)pes, 32);
			if(len >= 184)
			{
                ts_pack.clear();
                if(a_flag == 0)
                {
                    pes_head.adaptation_field_control = 0;
                    ts_pack_head[3] &= (pes_head.adaptation_field_control) << 4;
                    pes_head.payload_unit_start_indicator = 0x1;
                    ts_pack_head[1] |= (pes_head.payload_unit_start_indicator)<< 6;
                    pes_head.adaptation_field_control = 0x3;
                    ts_pack_head[3] |= (pes_head.adaptation_field_control) << 4;
                    ts_pack_head[3] |= pes_head.continuity_counter;
                    a_flag = 1;

                    //change by yh 2022/1/3
#if 0
                    fin.write((const char *)ts_pack_head, 4);
                    fin.put(0x01);
                    fin.put(0x40);
				    fin.write((const char *)pes, 182);
#endif
//                    ts_pack.insert(ts_pack.end(), ts_pack_head[0], ts_pack_head[3]);
                    for(i = 0; i < 4; i++)
                        ts_pack.push_back(ts_pack_head[i]);
                    ts_pack.push_back(0x01);
                    ts_pack.push_back(0x40);
                    ts_pack.insert(ts_pack.end(), pes, (pes+182));
                    
                    audio_queue.push_back(ts_pack);

				    len -= 182;
                    pes += 182;
                }
                else{
                    ts_pack.clear();
                    pes_head.adaptation_field_control = 0;
                    ts_pack_head[3] &= (pes_head.adaptation_field_control) << 4;
                
                    pes_head.payload_unit_start_indicator = 0x0;
                    ts_pack_head[1] &= (pes_head.payload_unit_start_indicator)<< 6;
                    ts_pack_head[1] |= (pes_head.transport_priority) << 5;
	                ts_pack_head[1] |= (pes_head.pid) >> 8;
                    
                    pes_head.adaptation_field_control = 0x1;
                    ts_pack_head[3] |= (pes_head.adaptation_field_control) << 4;
                    ts_pack_head[3] |= pes_head.continuity_counter;

                    //change by yh 2022/1/3
#if 0
                    fin.write((const char *)ts_pack_head, 4);
				    fin.write((const char *)pes, 184);
#endif
//                    ts_pack.insert(ts_pack.end(), ts_pack_head[0], ts_pack_head[3]);
                    for(i = 0; i < 4; i++)
                        ts_pack.push_back(ts_pack_head[i]);
                    ts_pack.insert(ts_pack.end(), pes, (pes+184));
                    
                    audio_queue.push_back(ts_pack);
                    
				    len -= 184;
                    pes += 184;
                }
			}
			else if(len < 184)
			{
                ts_pack.clear();
                pes_head.adaptation_field_control = 0;
                ts_pack_head[3] &= (pes_head.adaptation_field_control) << 4;
                
                pes_head.payload_unit_start_indicator = 0x0;
                ts_pack_head[1] &= (pes_head.payload_unit_start_indicator)<< 6;
                ts_pack_head[1] |= (pes_head.transport_priority) << 5;
	            ts_pack_head[1] |= (pes_head.pid) >> 8;
                
                pes_head.adaptation_field_control = 0x3;
                ts_pack_head[3] |= (pes_head.adaptation_field_control) << 4;
                ts_pack_head[3] |= pes_head.continuity_counter;

                //change by yh 2022/1/3 
#if 0
				fin.write((const char *)ts_pack_head, 4);
                adaptation = (char)(184 - len -1);
                fin.put(adaptation);
                if(adaptation != 0)
                    fin.put(0x0);
                
				for(i = 0; i < (182-len); i++)
				{
					fin.put(0xff);
				}
                fin.write((const char *)pes, len);
#endif
//                ts_pack.insert(ts_pack.end(), ts_pack_head[0], ts_pack_head[3]);
                for(i = 0; i < 4; i++)
                        ts_pack.push_back(ts_pack_head[i]);
                adaptation = (char)(184 - len -1);
                ts_pack.push_back(adaptation);
                if(adaptation != 0)
                    ts_pack.push_back(0x0);
                for(i = 0; i < (182-len); i++)
                    ts_pack.push_back(0xFF);
                ts_pack.insert(ts_pack.end(), pes, (pes+len));
                
                audio_queue.push_back(ts_pack);
                
				len = 0;
			}
        }
        Sumup_Counter(type);
	}

	return;
}

void TS_PACK::Sumup_Counter(int type)
{
	if(type == VIDEO)
	{
		if(video_index < 15)
		{
//			std::cout << "video index : " << video_index << std::endl;
			video_index++;
		}
		else
			video_index = 0;
	}
    else if(type == AUDIO)
    {
        if(audio_index < 15)
		{
//			std::cout << "video index : " << audio_index << std::endl;
			audio_index++;
		}
		else
			audio_index = 0;
    }
	else if(type == PMT)
	{
		if(psi_index < 15)
		{
//			std::cout << "psi index : " << psi_index << std::endl;
			psi_index++;
		}
		else
			psi_index = 0;
	}
}

void TS_PACK::ts_pack(uint8_t *pes, int len, int type, bool keyFrame)
{
	TS_Header media_ts_head;

//    printf("ts pid pmt : %02x, video : %02x audio : %02x\n", _pmt_pid, _video_pid, _audio_pid);

	switch(type)
	{
		case PAT:
			init_pat_ts(&media_ts_head);
			break;
		case PMT:
			init_pmt_ts(&media_ts_head);
			break;
		case VIDEO:
			init_video_ts(&media_ts_head);
			break;
		case AUDIO:
			init_audio_ts(&media_ts_head);
			break;
		default:
			break;
	}
#if 1
    memset(ts_pack_head, 0, 5);
	ts_pack_head[0] = media_ts_head.sync_byte;
	ts_pack_head[1] |= (media_ts_head.transport_error_indicator) << 7;
	ts_pack_head[1] |= (media_ts_head.payload_unit_start_indicator)<< 6;
	ts_pack_head[1] |= (media_ts_head.transport_priority) << 5;
	ts_pack_head[1] |= (media_ts_head.pid) >> 8;
	ts_pack_head[2] |= (media_ts_head.pid);
	ts_pack_head[3] |= (media_ts_head.transport_scrambling_cntrol) << 6;
	ts_pack_head[3] |= (media_ts_head.adaptation_field_control) << 4;
	ts_pack_head[3] |= media_ts_head.continuity_counter;
//    std::cout << "pack continuity_counter : " << media_ts_head.continuity_counter << std::endl;

#endif



    send_pes_ts_pack(pes, type, len, media_ts_head, keyFrame);

#if 1

    #endif
}

