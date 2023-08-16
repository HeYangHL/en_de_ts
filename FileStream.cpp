/*
 * @Author: 威泰视信
 * @Date: 2022-12-07 09:02:14
 * @LastEditors: 威泰视信
 * @LastEditTime: 2022-12-07 15:08:10
 * @FilePath: /ts/source/fileStream/FileStream.cpp
 * @Description: 
 * 
 * Copyright (c) 2022 by 威泰视信, All Rights Reserved. 
 */
#include "FileStream.h"

FileStream::FileStream(std::string file, std::string format)
:_file(file), _format(format){}

FileStream::FileStream(){}

FileStream::~FileStream()
{
    for(auto item : _frame_queue){
        item.clear();
    }
    _frame_queue.clear();
}

int FileStream::loadFile()
{
    std::ifstream in;
	in.open(_file, std::ios::in|std::ios::binary);
	if (!in.is_open()) {
		perror("file open:");
		return -1;
	}

    byteArray frame;
    uint64_t byteCnt = 0;

    if(_format == "h264" || _format == "h265"){    
        byteArray startCode;
        bool newFrame = true;
        uint8_t b;
        while (in.read((char*)&b, 1)){
            byteCnt++;
            if(newFrame){
                frame.clear();
                if(startCode.size()== 3){
                    if(startCode[0]==0x00 && startCode[1]==0x00 && startCode[2]==0x01){
                        frame.assign(startCode.begin(), startCode.end());
                        frame.push_back(b);
                        startCode.clear();
                        newFrame = false;
                        // printf("new frame.\n");
                    }else{
                        startCode.push_back(b);
                    }
                }else if(startCode.size()== 4){
                    if(startCode[0]==0x00 && startCode[1]==0x00 && startCode[2]==0x00 && startCode[3]==0x01){
                        frame.assign(startCode.begin(), startCode.end());
                        frame.push_back(b);
                        startCode.clear();
                        newFrame = false;
                        // printf("new frame.\n");
                    }else{
                        startCode.push_back(b);
                    }
                }else if(startCode.size() > 4){
                    startCode.clear();
                    std::cout << _format <<" file format error." << std::endl;
                    return -1;
                }else{
                    startCode.push_back(b);
                }
            }else{
                frame.push_back(b);
                if (frame.at(frame.size()-4) == 0x00 &&
                    frame.at(frame.size()-3) == 0x00 && 
                    frame.at(frame.size()-2) == 0x00 && 
                    frame.at(frame.size()-1) == 0x01 ){
                    startCode.clear();
                    startCode.assign(frame.end()-4, frame.end());
                    // PRINT(startCode, "startCode");
                    frame.pop_back();
                    frame.pop_back();
                    frame.pop_back();
                    frame.pop_back();
                    // PRINT(frame, "frame data");
                    _frame_queue.push_back(frame);
                    newFrame = true;
                    // printf("%s-%03d: frame[%02x] size:%lu\n", __FILE__, __LINE__, frame.at(4), frame.size());
                    // printf("%s-%03d: frame queue size:%lu\n", __FILE__, __LINE__, _frame_queue.size());
                } else if ( frame.at(frame.size()-3) == 0x00 && 
                            frame.at(frame.size()-2) == 0x00 && 
                            frame.at(frame.size()-1) == 0x01 ){
                    startCode.clear();
                    startCode.assign(frame.end()-3, frame.end());
                    frame.pop_back();
                    frame.pop_back();
                    frame.pop_back();
                    _frame_queue.push_back(frame);
                    newFrame = true;
                    // printf("%s-%03d: frame[%02x] size:%lu\n", __FILE__, __LINE__, frame.at(4), frame.size());
                    // printf("%s-%03d: frame queue size:%lu\n", __FILE__, __LINE__, _frame_queue.size());
                }
            }
        }
        _frame_queue.push_back(frame);
        // printf("%s-%03d: frame[%02x] size:%lu\n", __FILE__, __LINE__, frame.at(4), frame.size());
        // printf("%s-%03d: frame queue size:%lu\n", __FILE__, __LINE__, _frame_queue.size());
        printf("%s-%03d: file:<%s> Loading completed. Total bytes:%llu\n", __FILE__, __LINE__, _file.c_str(), byteCnt);
    }else if(_format == "aac"){
        byteArray adts_head;
        adts_head.resize(7);
        while(in.read((char*)adts_head.data(), 7)){
            byteCnt+=7;
            frame.clear();
            unsigned short framelen = ((adts_head.at(3) & 0x03) << 11) | ((adts_head.at(4) << 3)) | ((adts_head.at(5) & 0xE0) >> 5);
            framelen -= 7; // 取raw长度

            byteArray aac;
            aac.resize(framelen);
            if(in.read((char*)aac.data(), framelen)){
                byteCnt+=framelen;
                frame.insert(frame.end(), adts_head.begin(), adts_head.end());
                frame.insert(frame.end(), aac.begin(), aac.end());
                _frame_queue.push_back(frame);
                // printf("%s-%03d: frame size:%lu\n", __FILE__, __LINE__, frame.size());
                // printf("%s-%03d: frame queue size:%lu\n", __FILE__, __LINE__, _frame_queue.size());
            }
        }
        printf("%s-%03d: file:<%s> Loading completed. Total bytes:%llu\n", __FILE__, __LINE__, _file.c_str(), byteCnt);
    }

    // debugPrintQueue(_frame_queue);

    in.close();
    return 0;
}
int FileStream::loadFile(std::string file, std::string format)
{
    _file = file;
    _format = format;
    return loadFile();
}

FileStream::byteArray FileStream::getFrame(uint64_t interval)
{
    byteArray frame;
//    printf("index = %d, size = %d\n", _rindex, _frame_queue.size());
    frame.assign(_frame_queue.at(_rindex).begin(), _frame_queue.at(_rindex).end());
    _rindex = (_rindex>=(_frame_queue.size()-1))?0:_rindex+1;
    interval<=0?:usleep(interval);
    if(_rindex == 0)
        return (byteArray)0;
    
    return frame;
}