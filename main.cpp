#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "FileStream.h"
#include "ts_encoder.hpp"
#include "ts_decoder.hpp"


uint64_t v_pts = 0;
uint64_t a_pts = 0;


class FILE_T{
public:
    FileStream file_v;
    FileStream file_a;

};


using namespace std;



int _ra(void *opaque,unsigned char *buf, int len)
{
    FILE_T* file = (FILE_T *)opaque;
    FileStream::byteArray frame;
    int i = 0;
#if 1    
    frame =  file->file_a.getFrame(10);

    for(i = 0; i < frame.size(); i++)
        buf[i] = frame[i];

#endif
        
    return frame.size();
}
int _rv(void *opaque,unsigned char *buf, int len)
{
    FILE_T* file = (FILE_T *)opaque;
    unsigned long int index = 0;
    FileStream::byteArray frame;
    int i = 0;
#if 1
    while(1)
    {
        frame =  file->file_v.getFrame(10);

        for(i = 0; i < frame.size(); i++)
            buf[index + i] = frame.at(i);
        index += frame.size();
        if(frame.at(4) == 0x09 || frame.at(4) == 0x67 || frame.at(4) == 0x68 || frame.at(4) == 0x06)
            continue;
        else
            break;
    }
    #endif
 //   printf("video size = %d\n", index);
    return index;
}



void get_media_pts(uint64_t *video_pts, uint64_t *audio_pts)
{
    (*video_pts) = (v_pts*20000);
	v_pts++;
    
    (*audio_pts) = (a_pts*21333);
	a_pts++;
}

bool is_I_Frame(uint8_t * pes, int len)
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



int main(void)
{

    FILE_T file;
    TsEncoder ts_en("h265", "aac");
    TsEncoder::TS_PACK_QUEUE video_queue;
    TsEncoder::TS_PACK_QUEUE audio_queue;
    unsigned char video_data[1024*1024] = {0};
    unsigned char audio_data[1024*1024] = {0};
    unsigned int video_len = 0;
    unsigned int audio_len = 0;
    uint64_t video_pts = 0;
    uint64_t audio_pts = 0;
    std::vector<TsEncoder::byteArray>::iterator iter;
    std::vector<uint8_t>::iterator it;
    int i = 0;
    ofstream fin;

    //TS协议编码demo
    #if 0
    file.file_v.loadFile("video.h265", "h265");
    file.file_a.loadFile("/home/yanghe/teddy/project_app/mux_ts_API_20221223/build/audio.aac", "aac");
    fin.open("ts_test.ts");


    while(1)
    {
        if(video_pts <= audio_pts)
        {
            video_len = _rv((void *)&file, video_data, 0);
            video_pts = (v_pts*20000);
	        v_pts++;
            if(is_I_Frame(video_data, video_len))
                video_queue = ts_en.encode_video((char *)video_data, (int)video_len, video_pts, true);
            else
                video_queue = ts_en.encode_video((char *)video_data, (int)video_len, video_pts, false);

            for(iter = video_queue.begin(); iter != video_queue.end(); iter++)
            {
                for(i = 0; i < iter->size(); i++)
                {
                    fin.put(iter->at(i));
                }
            }
        }
        if(video_pts > audio_pts)
        {
            audio_len = _ra((void *)&file, audio_data, 0);
            audio_pts = (a_pts*21333);
	        a_pts++;
            audio_queue = ts_en.encode_audio((char *)audio_data, (int)audio_len, audio_pts);
            for(iter = audio_queue.begin(); iter != audio_queue.end(); iter++)
            {
                for(i = 0; i < iter->size(); i++)
                {
                    fin.put(iter->at(i));
                }
            }
        }
        if(video_len <= 0)
            break;

        memset(video_data, 0, sizeof(video_data));
        memset(audio_data, 0, sizeof(audio_data));
        
//        break;
    }
#endif

//TS协议解码demo
#if 1
    unsigned char ts[188] = {0};
    TsDecoder ts_de;
    TsDecoder::PACK_T ts_data;
    TsDecoder::PACK_T frame_data;
    uint64_t pts = 0;
    std::string type;
    int ret = 0;
    ofstream video_fin;
    // ofstream video_h265_fin;
    ofstream audio_fin;
    ifstream fout;

    video_fin.open("video.h264");
    // video_h265_fin.open("video.h265");
    audio_fin.open("audio.aac");
    fout.open("ts_mux.ts");
    
    while(fout.read((char *)ts, 188))
    {
        ts_data.clear();
        
        //printf("%02x %02x %02x %02x\n", ts[0], ts[1], ts[2], ts[3]);

        for(int i = 0; i < 188; i++)
            ts_data.push_back(ts[i]);
        
        ret = ts_de.decode(ts_data, frame_data, pts, type);
        if(ret == 0)
            continue;
        if(ret > 0)
        {
            
            // printf("media pts : %lld, type = %s, size = %d\n", pts, type.c_str(), ret);
            if(type == "h264")
            {
                video_fin.write((char *)&frame_data[0], ret);
            }
            // else if(type == "h265")
            // {
            //     video_h265_fin.write((char *)&frame_data[0], ret);
            // }
            else if(type == "aac")
            {
                audio_fin.write((char *)&frame_data[0], ret);
            }
            // sleep(10);
        }
    }
    fout.close();
    video_fin.close();
    audio_fin.close();
#endif   
#if 0
    int fd = 0;
    struct sockaddr_in sin;
    unsigned char buf[188] = {0};
    TsDecoder::PACK_T ts_data;
    TsDecoder::PACK_T frame_data;
    TsDecoder ts_de;
    ofstream video_fin;
    ofstream audio_fin;
    int ret = 0;
    uint64_t pts = 0;
    std::string type;
    
    video_fin.open("video.h264");
    audio_fin.open("audio.aac");

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0)
    {
        printf("socket error!");
        return -1;
    }
    sin.sin_family = AF_INET;
    sin.sin_port = htons(5000);
    sin.sin_addr.s_addr = inet_addr("");

    int len = sizeof(sin);

    while(1)
    {
        recvfrom(fd, buf, 188, 0, (struct sockaddr *)&sin, (socklen_t *)&len);
        ts_data.clear();
        
        for(int i = 0; i < 188; i++)
            ts_data.push_back(buf[i]);
        ret = ts_de.decode(ts_data, frame_data, pts, type);
        if(ret == 0)
            continue;
        if(ret > 0)
        {
            
            printf("media pts : %lld, type = %s\n", pts, type.c_str());
            if(type == "h264")
            {
                video_fin.write((char *)&frame_data[0], ret);
            }
            if(type == "aac")
            {
                audio_fin.write((char *)&frame_data[0], ret);
            }
        }
    }
#endif    

    return 0;
}
