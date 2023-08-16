#include "pes.hpp"

int PES_PACK::flag = 0;
// uint8_t PES_PACK::pts_val[5] = {0};

void PES_PACK::set_pts_dts(uint64_t us, uint8_t *buf)
{
	uint64_t pts = us * 9LL / 100LL;
	//    unsigned char pts_val[5];
	buf[0] = 0x20 | (((pts >> 30) & 7) << 1) | 1; // 3bits
	buf[1] = (pts >> 22) & 0xff;				  // 8bits
	buf[2] = (((pts >> 15) & 0x7f) << 1) | 1;	  // 7bits
	buf[3] = (pts >> 7) & 0xff;					  // 8bits
	buf[4] = ((pts & 0x7f) << 1) | 1;

	return;
}

void PES_PACK::Set_Pes_Pack(uint8_t *es, uint64_t pts, int size, int type, uint8_t *pes_out, uint32_t *pes_out_len)
{
	uint8_t *pes_pack;
	uint32_t pes_len = 0;
	uint8_t *buffer = NULL;

	pes_len = size + sizeof(pes_pack_head);
	uint8_t *pes_head = (uint8_t *)malloc(sizeof(pes_pack_head));
	memset(pes_head, 0, sizeof(pes_pack_head));

	pes_pack_head.packet_start_code_prefix = 0x000001;
	if (type == VIDEO)
	{
		pes_pack_head.stream_id = 0xE0;
		pes_pack_head.PES_packet_length = 0x0;
	}
	else if (type == AUDIO)
	{
		pes_pack_head.stream_id = 0xC0;
		pes_pack_head.PES_packet_length = ((short)size + 8);
	}
	pes_pack_head.pes_flage = 0x2;
	pes_pack_head.PES_scrambling_control = 0x0;
	pes_pack_head.PES_priority = 0x0;
	pes_pack_head.Data_alignment_indicator = 0x0;
	pes_pack_head.copyright = 0;
	pes_pack_head.Original_or_copy = 0x0;
	pes_pack_head.flags = 0x80;
	pes_pack_head.PES_head_len = 0x05;
	//    pes_pack_head.flags_data = pts;

	//    memcpy(&pes_head[0], (uint8_t *)pes_pack_head.packet_start_code_prefix, 3);
	pes_head[0] = pes_pack_head.packet_start_code_prefix >> 16;
	pes_head[1] = pes_pack_head.packet_start_code_prefix >> 8;
	pes_head[2] = pes_pack_head.packet_start_code_prefix;
	pes_head[3] = pes_pack_head.stream_id;
	//    memcpy(&pes_head[4], (uint8_t *)pes_pack_head.PES_packet_length, 2);
	pes_head[4] = pes_pack_head.PES_packet_length >> 8;
	pes_head[5] = pes_pack_head.PES_packet_length;
	pes_head[6] |= pes_pack_head.pes_flage << 6;
	pes_head[6] |= pes_pack_head.PES_scrambling_control << 4;
	pes_head[6] |= pes_pack_head.PES_priority << 3;
	pes_head[6] |= pes_pack_head.Data_alignment_indicator << 2;
	pes_head[6] |= pes_pack_head.copyright << 1;
	pes_head[6] |= pes_pack_head.Original_or_copy << 0;
	pes_head[7] = pes_pack_head.flags;
	pes_head[8] = pes_pack_head.PES_head_len;
#if 0
	//    memcpy(&pes_head[9], (uint8_t *)pes_pack_head.flags_data, 5);
	pes_head[9] = pes_pack_head.flags_data >> 32;
	pes_head[10] = pes_pack_head.flags_data >> 24;
	pes_head[11] = pes_pack_head.flags_data >> 16;
	pes_head[12] = pes_pack_head.flags_data >> 8;
	pes_head[13] = pes_pack_head.flags_data;
#endif
#if 0
	pes_head[9] = pts >> 32;
	pes_head[10] = pts >> 24;
	pes_head[11] = pts >> 16;
	pes_head[12] = pts >> 8;
	pes_head[13] = pts;

	pts_val[0] = pts >> 32;
	pts_val[1] = pts >> 24;
	pts_val[2] = pts >> 16;
	pts_val[3] = pts >> 8;
	pts_val[4] = pts;
#endif
#if 1
	set_pts_dts(pts, &pes_head[9]);
//	set_pts_dts(pts, pts_val);
#endif

#if 0
	pes_head[14] = dts >> 32;
	pes_head[15] = dts >> 24;
	pes_head[16] = dts >> 16;
	pes_head[17] = dts >> 8;
	pes_head[18] = dts;
#endif
	pes_pack = (uint8_t *)malloc(pes_len);
	memset(pes_pack, 0, pes_len);

	memcpy(pes_pack, (uint8_t *)pes_head, sizeof(pes_pack_head));
	memcpy(pes_pack + sizeof(pes_pack_head), es, size);

	memcpy(pes_out, pes_pack, pes_len);
	*pes_out_len = pes_len;

#if 0
	if(type == VIDEO)
	{
		set_ts_pack->ts_pack((uint8_t *)pes_pack, pes_len, VIDEO, keyFrame);
	}
	if(type == AUDIO)
		set_ts_pack->ts_pack((uint8_t *)pes_pack, pes_len, AUDIO, keyFrame);
#endif

	free(pes_pack);
	free(pes_head);
	free(buffer);
	return;
}
#if 0
TS_PACK *PES_PACK::get_ts_pack()
{
	return set_ts_pack;
}
#endif

void PES_PACK::get_pts_us(uint8_t *pts_val, uint64_t &pts)
{
	long long PTS = (pts_val[4] >> 1) | (pts_val[3] << 7) | (pts_val[2] >> 1) << 15 | pts_val[1] << 22 | (pts_val[0] >> 1) << 30;
	pts = PTS * 100LL / 9LL;

	//    printf("===>pts = %lld\n", pts);

	return;
}

int PES_PACK::parse_pes_head(unsigned char *ts_data)
{
	TS_Header ts_head_s;
	PES_Header pes_head_s;
	//    unsigned char ts_head[4] = {0};

//	unsigned char *Adaptive_Region = NULL;
//	char Adaptive_Region_Len = 0x0;
	unsigned char ch_len = 0;
	unsigned char pes_head[14] = {0x0};
	unsigned int pes_head_len = 0;
//	unsigned char *pes_data = NULL;
	unsigned int pes_data_len = 0;
	unsigned int index = 0;
//	unsigned char buf[1024] = {0};
//	unsigned int conflag = 0;

	memset((unsigned char *)&ts_head_s, 0, sizeof(ts_head_s));
	memset((unsigned char *)&pes_head_s, 0, sizeof(pes_head_s));
	memset((unsigned char *)pes_head, 0, sizeof(pes_head));

	ts_head_s.sync_byte = ts_data[index];
	ts_head_s.transport_error_indicator = ts_data[(index + 1)] >> 7;
	ts_head_s.payload_unit_start_indicator = (ts_data[(index + 1)] & 0x40) >> 6;
	ts_head_s.transport_priority = (ts_data[(index + 1)] & 0x20) >> 5;
	ts_head_s.pid |= ts_data[(index + 1)] << 8;
	ts_head_s.pid |= ts_data[(index + 2)];
	ts_head_s.transport_scrambling_cntrol = ts_data[(index + 3)] >> 6;
	ts_head_s.adaptation_field_control = (ts_data[(index + 3)] & 0x30) >> 4;
	ts_head_s.continuity_counter = (ts_data[(index + 3)] & 0xF);

	if (ts_head_s.adaptation_field_control == 2)
		return 0;

	if ((ts_head_s.sync_byte == 0x47) && ((ts_head_s.pid == _audio_pid) || (ts_head_s.pid == _video_pid)))
	{
		pes_head_len = pes_head_len + 4;
		index += 4;
		if (ts_head_s.adaptation_field_control == 0x3)
		{
			ch_len = ts_data[index];
			pes_head_len = pes_head_len + ch_len + 1;
			index += (ch_len + 1);
		}

		// if(ts_head_s.pid == _audio_pid)
		// {
		// 	printf("%02x %02x %02x %02x\n", ts_data[0], ts_data[1], ts_data[2], ts_data[3]);
		// }

		if (ts_head_s.payload_unit_start_indicator == 0x1)
		{
			if (ts_head_s.pid == _audio_pid)
			{
				audio_media_flag++;
				// printf("audio_media_flag : %d\n", audio_media_flag);
				if (audio_media_flag == 2)
				{
					return_type = _audio_type;
					return_pts = old_audio_pts;
					// printf("audio type : %02x\n", return_type);
				}
			}
			else if (ts_head_s.pid == _video_pid)
			{
				video_media_flag++;

				if (video_media_flag == 2)
				{
					return_type = _video_type;
					return_pts = old_video_pts;
				}
			}

			memcpy(pes_head, &ts_data[index], 14);
			pes_head_s.packet_start_code_prefix |= pes_head[0] << 16;
			pes_head_s.packet_start_code_prefix |= pes_head[1] << 8;
			pes_head_s.packet_start_code_prefix |= pes_head[2];
			pes_head_s.stream_id = pes_head[3];
			pes_head_s.PES_packet_length |= pes_head[4] << 8;
			pes_head_s.PES_packet_length |= pes_head[5];
			pes_head_s.pes_flage = pes_head[6] >> 6;
			pes_head_s.PES_scrambling_control = (pes_head[6] & 0x30) >> 4;
			pes_head_s.PES_priority = (pes_head[6] & 0x08) >> 3;
			pes_head_s.Data_alignment_indicator = (pes_head[6] & 0x04) >> 2;
			pes_head_s.copyright = (pes_head[6] & 0x02) >> 1;
			pes_head_s.Original_or_copy = pes_head[6] & 0x01;
			pes_head_s.flags = pes_head[7];
			pes_head_s.PES_head_len = pes_head[8];

			if (ts_head_s.pid == _audio_pid)
			{
				if (audio_media_flag == 1)
					get_pts_us(&pes_head[9], old_audio_pts);
				if (audio_media_flag == 2)
					get_pts_us(&pes_head[9], new_audio_pts);
			}
			else if (ts_head_s.pid == _video_pid)
			{
				if (video_media_flag == 1)
					get_pts_us(&pes_head[9], old_video_pts);
				if (video_media_flag == 2)
					get_pts_us(&pes_head[9], new_video_pts);
			}
			pes_head_len = pes_head_len + pes_head_s.PES_head_len + 9;
			index += pes_head_s.PES_head_len + 9;
		}
		pes_data_len = 188 - pes_head_len;

		// 写帧数据到vector容器中
		if (ts_head_s.pid == _video_pid)
			save_video_frame(ts_data, pes_head_len, pes_data_len);
		else if (ts_head_s.pid == _audio_pid)
			save_audio_frame(ts_data, pes_head_len, pes_data_len);
	}
	else
		return -1;

#if 0
	// if(ts_head_s.sync_byte == 0x47 && (ts_head_s.pid == _video_pid || ts_head_s.pid == _audio_pid))
	if ((ts_head_s.sync_byte == 0x47) && (ts_head_s.pid == _video_pid))
	{
		// if(ts_head_s.pid == _audio_pid)
		// {
		// 	for(int i = 0; i < 132; i++)
		// 	{
		// 		if(i % 16 == 0)
		// 			printf("\n");
		// 		printf("%02x ", ts_data[i]);
		// 	}
		// }
		// printf("\n");

		pes_head_len = pes_head_len + 4;
		index += 4;
		if (ts_head_s.adaptation_field_control == 0x3)
		{
			ch_len = ts_data[index];
			pes_head_len = pes_head_len + ch_len + 1;
			index += (ch_len + 1);
		}
		if (ts_head_s.payload_unit_start_indicator == 0x1)
		{
			video_media_flag++;
			//            media_frame.clear();
			if (video_media_flag == 1)
			{
#if 0
				video_media_frame.clear();
				if (ts_head_s.pid == _video_pid)
					return_type = _video_type;
				if (ts_head_s.pid == _audio_pid)
					return_type = _audio_type;
#endif
			}
			if (video_media_flag == 2)
			{
#if 0
				if (ts_head_s.pid == _video_pid)
					new_type = _video_type;
				if (ts_head_s.pid == _audio_pid)
					new_type = _audio_type;
#endif
				return_type = _video_type;
				return_pts = old_video_pts;
			}

			print_data(ts_data, 188);

			printf("index = %d\n", index);

			memcpy(pes_head, &ts_data[index], 14);
			pes_head_s.packet_start_code_prefix |= pes_head[0] << 16;
			pes_head_s.packet_start_code_prefix |= pes_head[1] << 8;
			pes_head_s.packet_start_code_prefix |= pes_head[2];
			pes_head_s.stream_id = pes_head[3];
			pes_head_s.PES_packet_length |= pes_head[4] << 8;
			pes_head_s.PES_packet_length |= pes_head[5];
			pes_head_s.pes_flage = pes_head[6] >> 6;
			pes_head_s.PES_scrambling_control = (pes_head[6] & 0x30) >> 4;
			pes_head_s.PES_priority = (pes_head[6] & 0x08) >> 3;
			pes_head_s.Data_alignment_indicator = (pes_head[6] & 0x04) >> 2;
			pes_head_s.copyright = (pes_head[6] & 0x02) >> 1;
			pes_head_s.Original_or_copy = pes_head[6] & 0x01;
			pes_head_s.flags = pes_head[7];
			pes_head_s.PES_head_len = pes_head[8];
			print_data(&pes_head[9], 5);
			if (video_media_flag == 1)
				get_pts_us(&pes_head[9], old_video_pts);
			if (video_media_flag == 2)
				get_pts_us(&pes_head[9], new_video_pts);

			pes_head_len = pes_head_len + pes_head_s.PES_head_len + 9;
			index += pes_head_s.PES_head_len + 9;
		}
		pes_data_len = 188 - pes_head_len;

		// 写帧数据到vector容器中
		//		printf("===>media_flag = %d\n", media_flag);
		if (video_media_flag == 1)
		{
			if (new_video_pts > 0)
			{
				old_video_pts = new_video_pts;
			}

			if (video_back_frame.size() > 0)
			{
				video_media_frame.insert(video_media_frame.end(), video_back_frame.begin(), video_back_frame.end());
				video_back_frame.clear();
			}

			video_media_frame.insert(video_media_frame.end(), (ts_data + pes_head_len), (ts_data + pes_data_len + pes_head_len));
		}
		else if (video_media_flag == 2)
		{
			video_back_frame.clear();
			video_back_frame.insert(video_back_frame.end(), (ts_data + pes_head_len), (ts_data + pes_data_len + pes_head_len));
		}
	}
	else if ((ts_head_s.sync_byte == 0x47) && (ts_head_s.pid == _audio_pid))
	{
		// if(ts_head_s.pid == _audio_pid)
		// {
		// 	for(int i = 0; i < 132; i++)
		// 	{
		// 		if(i % 16 == 0)
		// 			printf("\n");
		// 		printf("%02x ", ts_data[i]);
		// 	}
		// }
		// printf("\n");

		pes_head_len = pes_head_len + 4;
		index += 4;
		if (ts_head_s.adaptation_field_control == 0x3)
		{
			ch_len = ts_data[index];
			pes_head_len = pes_head_len + ch_len + 1;
			index += (ch_len + 1);
		}
		if (ts_head_s.payload_unit_start_indicator == 0x1)
		{
			audio_media_flag++;
			//            media_frame.clear();
			if (audio_media_flag == 1)
			{
#if 0
				video_media_frame.clear();
				if (ts_head_s.pid == _video_pid)
					return_type = _video_type;
				if (ts_head_s.pid == _audio_pid)
					return_type = _audio_type;
#endif
			}
			if (audio_media_flag == 2)
			{
#if 0
				if (ts_head_s.pid == _video_pid)
					new_type = _video_type;
				if (ts_head_s.pid == _audio_pid)
					new_type = _audio_type;
#endif
				return_type = _audio_type;
				return_pts = old_audio_pts;
			}

			memcpy(pes_head, &ts_data[index], 14);
			pes_head_s.packet_start_code_prefix |= pes_head[0] << 16;
			pes_head_s.packet_start_code_prefix |= pes_head[1] << 8;
			pes_head_s.packet_start_code_prefix |= pes_head[2];
			pes_head_s.stream_id = pes_head[3];
			pes_head_s.PES_packet_length |= pes_head[4] << 8;
			pes_head_s.PES_packet_length |= pes_head[5];
			pes_head_s.pes_flage = pes_head[6] >> 6;
			pes_head_s.PES_scrambling_control = (pes_head[6] & 0x30) >> 4;
			pes_head_s.PES_priority = (pes_head[6] & 0x08) >> 3;
			pes_head_s.Data_alignment_indicator = (pes_head[6] & 0x04) >> 2;
			pes_head_s.copyright = (pes_head[6] & 0x02) >> 1;
			pes_head_s.Original_or_copy = pes_head[6] & 0x01;
			pes_head_s.flags = pes_head[7];
			pes_head_s.PES_head_len = pes_head[8];

			if (audio_media_flag == 1)
				get_pts_us(&pes_head[9], old_audio_pts);
			if (audio_media_flag == 2)
				get_pts_us(&pes_head[9], new_audio_pts);

			pes_head_len = pes_head_len + pes_head_s.PES_head_len + 9;
			index += pes_head_s.PES_head_len + 9;
		}
		pes_data_len = 188 - pes_head_len;

		// 写帧数据到vector容器中
		//		printf("===>media_flag = %d\n", media_flag);
		if (audio_media_flag == 1)
		{
			if (new_audio_pts > 0)
			{
				old_audio_pts = new_audio_pts;
			}

			if (audio_back_frame.size() > 0)
			{
				audio_media_frame.insert(audio_media_frame.end(), audio_back_frame.begin(), audio_back_frame.end());
				audio_back_frame.clear();
			}

			audio_media_frame.insert(audio_media_frame.end(), (ts_data + pes_head_len), (ts_data + pes_data_len + pes_head_len));
		}
		else if (audio_media_flag == 2)
		{
			audio_back_frame.clear();
			audio_back_frame.insert(audio_back_frame.end(), (ts_data + pes_head_len), (ts_data + pes_data_len + pes_head_len));
		}
	}
	else
		return -1;
#endif

	return 0;
}

void PES_PACK::save_video_frame(unsigned char *ts_data, unsigned int pes_head_len, unsigned int pes_data_len)
{
	if (video_media_flag == 1)
	{
		if (new_video_pts > 0)
		{
			old_video_pts = new_video_pts;
		}

		if (video_back_frame.size() > 0)
		{
			video_media_frame.insert(video_media_frame.end(), video_back_frame.begin(), video_back_frame.end());
			video_back_frame.clear();
		}

		video_media_frame.insert(video_media_frame.end(), (ts_data + pes_head_len), (ts_data + pes_data_len + pes_head_len));
	}
	else if (video_media_flag == 2)
	{
		if(video_back_frame.size() < 188)
			video_media_frame.insert(video_media_frame.end(), video_back_frame.begin(), video_back_frame.end());
		video_back_frame.clear();
		video_back_frame.insert(video_back_frame.end(), (ts_data + pes_head_len), (ts_data + pes_data_len + pes_head_len));
	}
	return;
}
void PES_PACK::save_audio_frame(unsigned char *ts_data, unsigned int pes_head_len, unsigned int pes_data_len)
{
	// printf("=======>save frame size : %d\n", audio_media_flag);
	if (audio_media_flag == 1)
	{
		if (new_audio_pts > 0)
		{
			old_audio_pts = new_audio_pts;
		}

		if (audio_back_frame.size() > 0)
		{
			audio_media_frame.insert(audio_media_frame.end(), audio_back_frame.begin(), audio_back_frame.end());
			audio_back_frame.clear();
		}

		audio_media_frame.insert(audio_media_frame.end(), (ts_data + pes_head_len), (ts_data + pes_data_len + pes_head_len));
	}
	else if (audio_media_flag == 2)
	{
		if(audio_back_frame.size() < 188)
			audio_media_frame.insert(audio_media_frame.end(), audio_back_frame.begin(), audio_back_frame.end());
		audio_back_frame.clear();
		audio_back_frame.insert(audio_back_frame.end(), (ts_data + pes_head_len), (ts_data + pes_data_len + pes_head_len));
	}
	return;
}
