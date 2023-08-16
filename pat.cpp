#include "pat.hpp"

int PAT_PACK::MakeTable(uint32_t *crc32_table)
{
	for (uint32_t i = 0; i < 256; i++)
	{
		uint32_t k = 0;
		for (uint32_t j = (i << 24) | 0x800000; j != 0x80000000; j <<= 1)
		{
			k = (k << 1) ^ (((k ^ j) & 0x80000000) ? 0x04c11db7 : 0);
		}

		crc32_table[i] = k;
	}
}

uint32_t PAT_PACK::crc32(uint8_t *buffer, uint32_t size, uint32_t *crc32_table)
{
	uint32_t crc32_reg = 0xFFFFFFFF;
	for (uint32_t i = 0; i < size; i++)
	{
		crc32_reg = (crc32_reg << 8) ^ crc32_table[((crc32_reg >> 24) ^ *buffer++) & 0xFF];
	}
	return crc32_reg;
}

void PAT_PACK::set_pmt_pid(unsigned short pid)
{
	_pmt_pid = pid;
}

void PAT_PACK::InitPat(uint8_t *buf, uint32_t *len)
{
	TS_PAT_Program PAT_Program;
	//	TS_PACK set_ts_pack;
	uint32_t crc32Table[256] = {0};
	uint32_t crc = 0;
	//	uint8_t buf[183] = {0};
	int i = 0;
	int pat_head_len = 0;

	set_pat_pack.program.clear();
#if 0
	map<short, short>::iterator it;
	for(it = mux_media._Program_No.begin(); it != mux_media._Program_No.end();it++)
	{
		//        printf("pmt program no : %02x, pid : %02x of pat pack\n", it->first, it->second);
		PAT_Program.program_number = it->first;
		PAT_Program.program_map_PID = it->second;
		PAT_Program.reserved_3 = 0x7;
		set_pat_pack.program.push_back(PAT_Program);
	}
#endif
	PAT_Program.program_number = 0x1;
	PAT_Program.program_map_PID = _pmt_pid;
	PAT_Program.reserved_3 = 0x7;
	set_pat_pack.program.push_back(PAT_Program);

	MakeTable(crc32Table);

	set_pat_pack.table_id = 0x0;
	set_pat_pack.section_syntax_indicator = 0x1;
	set_pat_pack.zero = 0x0;
	set_pat_pack.reserved_1 = 0x3;
	set_pat_pack.section_length = (0x9 + (set_pat_pack.program.size() * 4));
	set_pat_pack.transport_stream_id = 0x1;
	set_pat_pack.reserved_2 = 0x3;
	set_pat_pack.version_number = 0x0;
	set_pat_pack.current_next_indicator = 0x1;
	set_pat_pack.section_number = 0x0;
	set_pat_pack.last_section_number = 0x0;

	//    set_pat_pack.network_PID = 0x00;//节目号(PAT_Program.program_number)为0时使用该字段，表示这是NIT，节目号非0时表示这是PMT

	// 按bit排序
	buf[0] = set_pat_pack.table_id;
	buf[1] |= (set_pat_pack.section_syntax_indicator) << 7;
	buf[1] |= (set_pat_pack.zero) << 6;
	buf[1] |= (set_pat_pack.reserved_1) << 4;
	buf[1] |= (set_pat_pack.section_length) >> 8;
	buf[2] |= (set_pat_pack.section_length);
	buf[3] |= (set_pat_pack.transport_stream_id) >> 8;
	buf[4] |= (set_pat_pack.transport_stream_id);
	buf[5] |= (set_pat_pack.reserved_2) << 6;
	buf[5] |= (set_pat_pack.version_number) << 1;
	buf[5] |= (set_pat_pack.current_next_indicator) << 0;
	buf[6] |= (set_pat_pack.section_number);
	buf[7] |= (set_pat_pack.last_section_number);

	for (i = 0; i < set_pat_pack.program.size(); ++i)
	{
		buf[8 + (3 * i)] |= (set_pat_pack.program[i].program_number) >> 8;
		buf[9 + (3 * i)] |= (set_pat_pack.program[i].program_number);
		buf[10 + (3 * i)] |= (set_pat_pack.program[i].reserved_3) << 5;
		buf[10 + (3 * i)] |= (set_pat_pack.program[i].program_map_PID) >> 8;
		buf[11 + (3 * i)] |= (set_pat_pack.program[i].program_map_PID);
	}
	i--;
	crc = crc32((uint8_t *)buf, (8 + (set_pat_pack.program.size() * 4)), crc32Table);
	set_pat_pack.CRC_32 = crc;
	buf[12 + (3 * i)] |= (set_pat_pack.CRC_32) >> 24;
	buf[13 + (3 * i)] |= (set_pat_pack.CRC_32) >> 16;
	buf[14 + (3 * i)] |= (set_pat_pack.CRC_32) >> 8;
	buf[15 + (3 * i)] |= (set_pat_pack.CRC_32);

	int pat_len = set_pat_pack.section_length + 3;

	for (i = pat_len; i < 183; i++)
	{
		buf[i] = 0xff;
	}
	//	set_ts_pack.ts_pack((uint8_t *)buf, 183, PAT, false);
	*len = 183;
	return;
}

// int PAT_PACK::parse_pat(unsigned char *ts_data, unsigned short *pmt_pid)
int PAT_PACK::parse_pat(unsigned char *ts_data, std::vector<unsigned short> *pmt_pid)
{
	TS_Header ts_head_s;
	TS_PAT ts_pat;
	memset(&ts_pat, 0, sizeof(ts_pat));

	unsigned int read_len = 0;

	unsigned char pat_len[4] = {0};
	unsigned char *pat_head = NULL;
	unsigned int index = 0;
	unsigned int program_count = 0;

	memset((unsigned char *)&ts_head_s, 0, sizeof(ts_head_s));

	ts_head_s.sync_byte = ts_data[0];
	ts_head_s.transport_error_indicator = ts_data[1] >> 7;
	ts_head_s.payload_unit_start_indicator = (ts_data[1] & 0x40) >> 6;
	ts_head_s.transport_priority = (ts_data[1] & 0x20) >> 5;
	ts_head_s.pid |= ts_data[1] << 8;
	ts_head_s.pid |= ts_data[2];
	ts_head_s.transport_scrambling_cntrol = ts_data[3] >> 6;
	ts_head_s.adaptation_field_control = (ts_data[3] & 0x30) >> 4;
	ts_head_s.continuity_counter = (ts_data[3] & 0xF);

	if (ts_head_s.sync_byte == 0x47 && ts_head_s.pid == 0x0)
	{
		// for(int i = 0; i < 188; i++)
		// {
		// 	if(i%16 == 0)
		// 		printf("\n");
		// 	printf("%02x ", ts_data[i]);
		// }
		// printf("\n");
		// sleep(10);
		if (ts_head_s.adaptation_field_control == 1)
		{

			ts_pat.table_id = ts_data[5];
			ts_pat.section_syntax_indicator = ts_data[6] >> 7;
			ts_pat.zero = (ts_data[6] & 0x40) >> 6;
			ts_pat.reserved_1 = (ts_data[6] & 0x30) >> 4;
			ts_pat.section_length |= ((ts_data[6]&0x0F) << 8);
			ts_pat.section_length |= ts_data[7];

		

			if (ts_pat.table_id == 0x0)
			{
				pat_head = new unsigned char[ts_pat.section_length];

				memcpy(pat_head, (ts_data + 8), ts_pat.section_length);
				program_count = (ts_pat.section_length - 5 - 4) / 4;
				printf("program_count = %d\n", program_count);

				for (int i = 0; i < program_count; i++)
				{
					TS_PAT_Program pat_pro;
					memset((unsigned char *)&pat_pro, 0, sizeof(pat_pro));
					pat_pro.program_number = 0;
					pat_pro.program_map_PID = 0;
					pat_pro.program_number |= pat_head[5 + (4 * i)] << 8;
					pat_pro.program_number |= pat_head[6 + (4 * i)];
					pat_pro.program_map_PID |= pat_head[7 + (4 * i)] << 8;
					pat_pro.program_map_PID |= pat_head[8 + (4 * i)];
					
					pmt_pid->push_back(pat_pro.program_map_PID);
				}
				index += ts_pat.section_length + 8;
				// printf("5 : %02x, 6 : %02x, 7 : %02x, 8 : %02x\n", pat_head[5], pat_head[6], pat_head[7], pat_head[8]);
			}
			else
			{
				std::cout << "1 this is not PAT pack!" << std::endl;
				return -1;
			}
		}
		else
		{

			ts_pat.table_id = pat_len[4];
			ts_pat.section_syntax_indicator = pat_len[5] >> 7;
			ts_pat.zero = (pat_len[5] & 0x40) >> 6;
			ts_pat.reserved_1 = (pat_len[5] & 0x30) >> 4;
			ts_pat.section_length |= (pat_len[5] << 8);
			ts_pat.section_length |= pat_len[6];

			if (ts_pat.table_id == 0x0)
			{
				pat_head = new unsigned char[ts_pat.section_length];

				memcpy(pat_head, (ts_data + 7), ts_pat.section_length);
				program_count = (ts_pat.section_length - 5 - 4) / 4;

				for (int i = 0; i < program_count; i++)
				{
					TS_PAT_Program pat_pro;
					memset((unsigned char *)&pat_pro, 0, sizeof(pat_pro));
					pat_pro.program_number = 0;
					pat_pro.program_map_PID = 0;
					pat_pro.program_number |= pat_head[5 + (4 * i)] << 8;
					pat_pro.program_number |= pat_head[6 + (4 * i)];
					pat_pro.program_map_PID |= pat_head[7 + (4 * i)] << 8;
					pat_pro.program_map_PID |= pat_head[8 + (4 * i)];

					pmt_pid->push_back(pat_pro.program_map_PID);
				}

				index += ts_pat.section_length + 7;
			}
			else
			{
				std::cout << "2 this is not PAT pack!" << std::endl;
				return -1;
			}
		}
		delete[] pat_head;
		pat_head = NULL;
	}
	else
	{
		std::cout << "3 this is not PAT pack!" << std::endl;
		return -1;
	}
	return 0;
}
