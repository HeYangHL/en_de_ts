#include "pmt.hpp"

void PMT_PACK::set_pmt_pid(unsigned short v_pid, unsigned short a_pid, unsigned char v_type, unsigned char a_type)
{
	_video_pid = v_pid;
	_audio_pid = a_pid;
	_video_type = v_type;
	_audio_type = a_type;
}

void PMT_PACK::InitPmt(uint8_t *buf, uint32_t *len)
{
	TS_PMT_Stream v_pmt_stream, a_pmt_stream, pmt_stream;
	//	TS_PACK set_ts_pack;
	PAT_PACK pat_pack;
	//	uint8_t buf[183] = {0};
	uint32_t crc32Table[256] = {0};
	uint32_t crc = 0;
	int i = 0;

	set_pmt_pack.PMT_Stream.clear();

	pmt_stream.stream_type = _video_type;
	pmt_stream.reserved_5 = 0x7;
	pmt_stream.elementary_PID = _video_pid;
	pmt_stream.reserved_6 = 0xf;
	pmt_stream.ES_info_length = 0x0;
	set_pmt_pack.PMT_Stream.push_back(pmt_stream);
	pmt_stream.stream_type = _audio_type;
	pmt_stream.reserved_5 = 0x7;
	pmt_stream.elementary_PID = _audio_pid;
	pmt_stream.reserved_6 = 0xf;
	pmt_stream.ES_info_length = 0x0;
	set_pmt_pack.PMT_Stream.push_back(pmt_stream);

	pat_pack.MakeTable(crc32Table);

	set_pmt_pack.table_id = 0x2;
	set_pmt_pack.section_syntax_indicator = 0x1;
	set_pmt_pack.zero = 0x0;
	set_pmt_pack.reserved_1 = 0x3;
	set_pmt_pack.section_length = (0xd + set_pmt_pack.PMT_Stream.size() * 5);
	set_pmt_pack.program_number = 0x0001; // 频道号，表示当前的PMT关联到得频道
	set_pmt_pack.reserved_2 = 0x3;
	set_pmt_pack.version_number = 0;
	set_pmt_pack.current_next_indicator = 0x1;
	set_pmt_pack.section_number = 0;
	set_pmt_pack.last_section_number = 0;
	set_pmt_pack.reserved_3 = 0x7;
	set_pmt_pack.PCR_PID = 0x100;
	set_pmt_pack.reserved_4 = 0xf;
	set_pmt_pack.program_info_length = 0x0;

	buf[0] = set_pmt_pack.table_id;
	buf[1] |= (set_pmt_pack.section_syntax_indicator) << 7;
	buf[1] |= (set_pmt_pack.zero) << 6;
	buf[1] |= (set_pmt_pack.reserved_1) << 4;
	buf[1] |= (set_pmt_pack.section_length) >> 8;
	buf[2] |= set_pmt_pack.section_length;
	buf[3] |= (set_pmt_pack.program_number) >> 8;
	buf[4] |= set_pmt_pack.program_number;
	buf[5] |= (set_pmt_pack.reserved_2) << 6;
	buf[5] |= (set_pmt_pack.version_number) << 1;
	buf[5] |= (set_pmt_pack.current_next_indicator);
	buf[6] |= (set_pmt_pack.section_number);
	buf[7] |= (set_pmt_pack.last_section_number);
	buf[8] |= (set_pmt_pack.reserved_3) << 5;
	buf[8] |= (set_pmt_pack.PCR_PID) >> 8;
	buf[9] |= (set_pmt_pack.PCR_PID);
	buf[10] |= (set_pmt_pack.reserved_4) << 4;
	buf[10] |= (set_pmt_pack.program_info_length) >> 8;
	buf[11] |= (set_pmt_pack.program_info_length);

	for (i = 0; i < set_pmt_pack.PMT_Stream.size(); i++)
	{
		buf[12 + (5 * i)] |= (set_pmt_pack.PMT_Stream[i].stream_type);
		buf[13 + (5 * i)] |= (set_pmt_pack.PMT_Stream[i].reserved_5) << 5;
		buf[13 + (5 * i)] |= (set_pmt_pack.PMT_Stream[i].elementary_PID) >> 8;
		buf[14 + (5 * i)] |= (set_pmt_pack.PMT_Stream[i].elementary_PID);
		buf[15 + (5 * i)] |= (set_pmt_pack.PMT_Stream[i].reserved_6) << 4;
		buf[15 + (5 * i)] |= (set_pmt_pack.PMT_Stream[i].ES_info_length) >> 8;
		buf[16 + (5 * i)] |= (set_pmt_pack.PMT_Stream[i].ES_info_length);
	}

	crc = pat_pack.crc32((uint8_t *)buf, (0xc + set_pmt_pack.PMT_Stream.size() * 5), crc32Table);
	set_pmt_pack.CRC_32 = crc;

	i--;
	buf[17 + (5 * i)] |= (set_pmt_pack.CRC_32) >> 24;
	buf[18 + (5 * i)] |= (set_pmt_pack.CRC_32) >> 16;
	buf[19 + (5 * i)] |= (set_pmt_pack.CRC_32) >> 8;
	buf[20 + (5 * i)] |= (set_pmt_pack.CRC_32);

	//    printf("pmt head data:\n");
	//   mux_media.print_data(buf, (20+(5*i)+1));

	int pmt_len = set_pmt_pack.section_length + 3;

	for (i = pmt_len; i < 183; i++)
	{
		buf[i] = 0xff;
	}
	*len = 183;
	//	set_ts_pack.ts_pack((uint8_t *)buf, 183, PMT, false);

	return;
}

int PMT_PACK::parse_pmt(unsigned char *ts_data, unsigned short *video_pid, unsigned short *audio_pid, unsigned char *video_type, unsigned char *audio_type)
{
	unsigned char pmt_len[4] = {0};
	unsigned char *pmt_head = NULL;
	TS_Header ts_head_s;
	TS_PMT ts_pmt;
	TS_PMT_Stream pmt_stream;
	PAT_PACK pat_s;
	unsigned int index = 0;
	unsigned int pmt_flag = 0;
	std::vector<unsigned short>::iterator pmt_item;
	int media_message_len = 0;
	int media_message = 0;
	int media_num = 0;

	memset((unsigned char *)&ts_head_s, 0, sizeof(ts_head_s));
	memset((unsigned char *)&ts_pmt, 0, sizeof(ts_pmt));


	ts_head_s.sync_byte = ts_data[0];
	ts_head_s.transport_error_indicator = ts_data[1] >> 7;
	ts_head_s.payload_unit_start_indicator = (ts_data[1] & 0x40) >> 6;
	ts_head_s.transport_priority = (ts_data[1] & 0x20) >> 5;
	ts_head_s.pid |= ts_data[1] << 8;
	ts_head_s.pid |= ts_data[2];
	ts_head_s.transport_scrambling_cntrol = ts_data[3] >> 6;
	ts_head_s.adaptation_field_control = (ts_data[3] & 0x30) >> 4;
	ts_head_s.continuity_counter = (ts_data[3] & 0xF);

	for (pmt_item = _pmt_pid.begin(); pmt_item != _pmt_pid.end(); pmt_item++)
	{
		if (ts_head_s.sync_byte == 0x47 && ts_head_s.pid == *pmt_item)
		{

			if (ts_head_s.adaptation_field_control == 1)
			{

				ts_pmt.table_id = ts_data[5];
				ts_pmt.section_syntax_indicator = ts_data[6] >> 7;
				ts_pmt.zero = (ts_data[6] & 0x40) >> 6;
				ts_pmt.reserved_1 = (ts_data[6] & 0x30) >> 4;
				ts_pmt.section_length |= (ts_data[6] << 8);
				ts_pmt.section_length |= ts_data[7];

				if (ts_pmt.table_id == 0x2)
				{

					pmt_head = new unsigned char[ts_pmt.section_length];

					memcpy(pmt_head, (ts_data + 8), ts_pmt.section_length);

					ts_pmt.program_info_length |= (pmt_head[7] & 0x0F) << 4;
					ts_pmt.program_info_length |= pmt_head[8];
					printf("pmt program message len : %d\n", ts_pmt.program_info_length);

					media_message_len = ts_pmt.section_length - 9 - ts_pmt.program_info_length;
					while ((media_message_len - (5 * media_num) - media_message) > 4)
					{
						// 获取第一个流媒体信息
						memset((unsigned char *)&pmt_stream, 0, sizeof(pmt_stream));
						pmt_stream.stream_type |= pmt_head[9 + (5 * media_num) + ts_pmt.program_info_length + media_message];
						pmt_stream.reserved_5 = pmt_head[10 + (5 * media_num) + ts_pmt.program_info_length + media_message] >> 5;
						pmt_stream.elementary_PID |= (pmt_head[10 + (5 * media_num) + ts_pmt.program_info_length + media_message] & 0x1F) << 8;
						pmt_stream.elementary_PID |= pmt_head[11 + (5 * media_num) + ts_pmt.program_info_length + media_message];
						pmt_stream.reserved_6 = pmt_head[12 + (5 * media_num) + ts_pmt.program_info_length + media_message] >> 4;
						pmt_stream.ES_info_length |= (pmt_head[12 + (5 * media_num) + ts_pmt.program_info_length + media_message]&0x0F) << 8;
						pmt_stream.ES_info_length |= pmt_head[13 + (5 * media_num) + ts_pmt.program_info_length + media_message];

						media_message += pmt_stream.ES_info_length;
						media_num++;

						if (pmt_stream.stream_type == 0x1b || pmt_stream.stream_type == 0x24)
						{
							*video_pid = pmt_stream.elementary_PID;
							*video_type = pmt_stream.stream_type;
						}
						if (pmt_stream.stream_type == 0xf || pmt_stream.stream_type == 0x3 || pmt_stream.stream_type == 0x11 || pmt_stream.stream_type == 0x4)
						{
							*audio_pid = pmt_stream.elementary_PID;
							*audio_type = pmt_stream.stream_type;
						}
					}
#if 0
					for (int i = 0; i < ((ts_pmt.section_length - 13) / 5); i++)
					{
						memset((unsigned char *)&pmt_stream, 0, sizeof(pmt_stream));
						pmt_stream.stream_type = pmt_head[9 + (i * 5)];
						pmt_stream.reserved_5 = pmt_head[10 + (i * 5)] >> 5;
						pmt_stream.elementary_PID |= pmt_head[10 + (i * 5)] << 8;
						pmt_stream.elementary_PID |= pmt_head[11 + (i * 5)];
						pmt_stream.reserved_6 = pmt_head[12 + (i * 5)] >> 4;
						pmt_stream.ES_info_length |= pmt_head[12 + (i * 5)] << 8;
						pmt_stream.ES_info_length |= pmt_head[13 + (i * 5)];
						// change by yh 20230302 for support H265
						if (pmt_stream.stream_type == 0x1b || pmt_stream.stream_type == 0x24)
						{
							*video_pid = pmt_stream.elementary_PID;
							*video_type = pmt_stream.stream_type;
						}
						if (pmt_stream.stream_type == 0xf || pmt_stream.stream_type == 0x3 || pmt_stream.stream_type == 0x11 || pmt_stream.stream_type == 0x4)
						{
							*audio_pid = pmt_stream.elementary_PID;
							*audio_type = pmt_stream.stream_type;
						}
					}
#endif
					break;
				}
				else
					continue;
			}
			else
			{

				ts_pmt.table_id = ts_data[4];
				ts_pmt.section_syntax_indicator = ts_data[5] >> 7;
				ts_pmt.zero = (ts_data[5] & 0x40) >> 6;
				ts_pmt.reserved_1 = (ts_data[5] & 0x30) >> 4;
				ts_pmt.section_length |= (ts_data[5] << 8);
				ts_pmt.section_length |= ts_data[6];
				if (ts_pmt.table_id == 0x2)
				{

					pmt_head = new unsigned char[ts_pmt.section_length];

					memcpy(pmt_head, (ts_data + 7), ts_pmt.section_length);

					for (int i = 0; i < ((ts_pmt.section_length - 13) / 5); i++)
					{
						pmt_stream.stream_type = pmt_head[9 + (i * 5)];
						pmt_stream.reserved_5 = pmt_head[10 + (i * 5)] >> 5;
						pmt_stream.elementary_PID |= pmt_head[10 + (i * 5)] << 8;
						pmt_stream.elementary_PID |= pmt_head[11 + (i * 5)];
						pmt_stream.reserved_6 = pmt_head[12 + (i * 5)] >> 4;
						pmt_stream.ES_info_length |= pmt_head[12 + (i * 5)] << 8;
						pmt_stream.ES_info_length |= pmt_head[13 + (i * 5)];
						if (pmt_stream.stream_type == 0x1b || pmt_stream.stream_type == 0x24)
						{
							*video_pid = pmt_stream.elementary_PID;
							*video_type = pmt_stream.stream_type;
						}
						if (pmt_stream.stream_type == 0xf || pmt_stream.stream_type == 0x3 || pmt_stream.stream_type == 0x11 || pmt_stream.stream_type == 0x4)
						{
							*audio_pid = pmt_stream.elementary_PID;
							*audio_type = pmt_stream.stream_type;
						}
					}
					break;
				}
				else
					continue;
			}
			delete[] pmt_head;
			pmt_head = NULL;
		}
		else
			continue;
	}
	if (pmt_item == _pmt_pid.end())
		return -1;

#if 0
	memset((unsigned char *)&ts_pmt, 0, sizeof(ts_pmt));
	memset((unsigned char *)&pmt_stream, 0, sizeof(pmt_stream));

	while(index <= (datalen-188))
	{
		memset((unsigned char *)&ts_head_s, 0, sizeof(ts_head_s));

		ts_head_s.sync_byte = ts_data[index];
		ts_head_s.transport_error_indicator = ts_data[(index+1)] >> 7;
		ts_head_s.payload_unit_start_indicator = (ts_data[(index+1)]&0x40) >> 6;
		ts_head_s.transport_priority = (ts_data[(index+1)]&0x20) >> 5;
		ts_head_s.pid |= ts_data[(index+1)] << 8;
		ts_head_s.pid |= ts_data[(index+2)];
		ts_head_s.transport_scrambling_cntrol = ts_data[(index+3)] >> 6;
		ts_head_s.adaptation_field_control = (ts_data[(index+3)]&0x30) >> 4;
		ts_head_s.continuity_counter = (ts_data[(index+3)]&0xF);


		if(ts_head_s.sync_byte == 0x47 && ts_head_s.pid == pat_s.pmt_pid)
		{

			if(ts_head_s.adaptation_field_control == 1)
			{
				//				fin.read((char *)pmt_len, 4);
				ts_pmt.table_id = ts_data[(index+5)];
				ts_pmt.section_syntax_indicator = ts_data[(index+6)] >> 7;
				ts_pmt.zero = (ts_data[(index+6)]&0x40) >> 6;
				ts_pmt.reserved_1 = (ts_data[(index+6)]&0x30) >> 4;
				ts_pmt.section_length |= (ts_data[(index+6)] << 8);
				ts_pmt.section_length |= ts_data[(index+7)];

				if(ts_pmt.table_id == 0x2)
				{

					pmt_head = new unsigned char[ts_pmt.section_length];

					//					fin.read((char *)pmt_head, ts_pmt.section_length);
					memcpy(pmt_head, (ts_data+index+8), ts_pmt.section_length);


					for(int i = 0; i < ((ts_pmt.section_length -13)/5); i++)
					{
						pmt_stream.stream_type = pmt_head[9+(i*5)];
						pmt_stream.reserved_5 = pmt_head[10+(i*5)] >> 5;
						pmt_stream.elementary_PID |= pmt_head[10+(i*5)] << 8;
						pmt_stream.elementary_PID |= pmt_head[11+(i*5)];
						pmt_stream.reserved_6 = pmt_head[12+(i*5)] >> 4;
						pmt_stream.ES_info_length |= pmt_head[12+(i*5)] << 8;
						pmt_stream.ES_info_length |= pmt_head[13+(i*5)];

						if(pmt_stream.stream_type == 0x1b || pmt_stream.stream_type == 0x2)
							video_pid = pmt_stream.elementary_PID;
						if(pmt_stream.stream_type == 0xf || pmt_stream.stream_type == 0x3 || pmt_stream.stream_type == 0x11 || pmt_stream.stream_type == 0x4)
							audio_pid = pmt_stream.elementary_PID;
					}
					index += ts_pmt.section_length+8;
					pmt_flag = 1;

					break;
				}


			}
			else
			{

				ts_pmt.table_id = ts_data[(index+4)];
				ts_pmt.section_syntax_indicator = ts_data[(index+5)] >> 7;
				ts_pmt.zero = (ts_data[(index+5)]&0x40) >> 6;
				ts_pmt.reserved_1 = (ts_data[(index+5)]&0x30) >> 4;
				ts_pmt.section_length |= (ts_data[(index+5)] << 8);
				ts_pmt.section_length |= ts_data[(index+6)];
				if(ts_pmt.table_id == 0x2)
				{

					pmt_head = new unsigned char[ts_pmt.section_length];

					memcpy(pmt_head, (ts_data+index+7), ts_pmt.section_length);


					for(int i = 0; i < ((ts_pmt.section_length -13)/5); i++)
					{
						pmt_stream.stream_type = pmt_head[9+(i*5)];
						pmt_stream.reserved_5 = pmt_head[10+(i*5)] >> 5;
						pmt_stream.elementary_PID |= pmt_head[10+(i*5)] << 8;
						pmt_stream.elementary_PID |= pmt_head[11+(i*5)];
						pmt_stream.reserved_6 = pmt_head[12+(i*5)] >> 4;
						pmt_stream.ES_info_length |= pmt_head[12+(i*5)] << 8;
						pmt_stream.ES_info_length |= pmt_head[13+(i*5)];
						if(pmt_stream.stream_type == 0x1b || pmt_stream.stream_type == 0x2)
							video_pid = pmt_stream.elementary_PID;
						if(pmt_stream.stream_type == 0xf || pmt_stream.stream_type == 0x3 || pmt_stream.stream_type == 0x11 || pmt_stream.stream_type == 0x4)
							audio_pid = pmt_stream.elementary_PID;

					}

					index += ts_pmt.section_length+7;
					pmt_flag = 1;

					break;
				}
			}
		}

		if(pmt_flag == 1)
		{
			pmt_flag = 0;
			break;
		}
		index++;

	}


	if(index >= (datalen-188))
	{
		printf("not found PMT data!\n");
		return -1;
	}
	delete[] pmt_head;
	pmt_head = NULL;
#endif
	return 0;
}
