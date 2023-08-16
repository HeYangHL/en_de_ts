/*
 * @Author: 威泰视信
 * @Date: 2022-12-07 09:01:30
 * @LastEditors: 威泰视信
 * @LastEditTime: 2022-12-08 09:00:02
 * @FilePath: /ts/source/fileStream/FileStream.h
 * @Description: 
 * 
 * Copyright (c) 2022 by 威泰视信, All Rights Reserved. 
 */
#ifndef __FILE_STREAM_HEAD_H__
#define __FILE_STREAM_HEAD_H__

#include<iostream>
#include<vector>
#include <fstream>
#include <string>
#include <unistd.h>
#include <stdint.h>


#define H265_FRAME_TYPE(v) ((uint8_t)(v)&0x7E)>>1
#define H264_FRAME_TYPE(v) ((uint8_t)(v)&0x1f)

class FileStream
{
public:
    FileStream(std::string file, std::string format);
    FileStream();
    ~FileStream();

    int loadFile();
    int loadFile(std::string file, std::string format);
   

    typedef std::vector<uint8_t>  byteArray;
    /**
     * @description: 获取一帧数据；
     * @param interval: >0等待时长，<=0直接返回，单位us。注意：并不是等待超时，而是提供的类似帧间隔的机制，目的是控制帧率；
     * @return [*]: 帧数据信息；
     */    
    FileStream::byteArray getFrame(uint64_t interval);

private:
    std::string _file;
    std::string _format;                        /* h264/h265/aac */
    std::vector<byteArray> _frame_queue;        /* 帧队列 */
    int _rindex = 0;                            /* 读索引 */

private:
    inline void PRINT(byteArray frame, std::string msg="data", int size=64){
        printf("%s:\n", msg.c_str());
        for(int i=0; i<((size>frame.size())?frame.size():size); i++){
            printf("0x%02x ", frame[i]);
            if((i+1)%16==0) printf("\n");
        }
        printf("\n");
    }

    inline void debugPrintQueue(std::vector<byteArray> queue){
        for(auto item : queue){
            PRINT(item, "frame data");
        }
    }
};


#endif  //__FILE_STREAM_HEAD_H__