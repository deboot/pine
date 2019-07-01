
#ifndef __push_h
#define __push_h

#include <stdio.h>
#include "../librtmp/rtmp.h"
#include "../librtmp/rtmp_sys.h"

int RTMP264_Connect(const char* url);    
int RTMP264_Send(char *data,unsigned int size,int bIsKeyFrame,unsigned int nTimeStamp);
int RTMP264_SetMetadata(uint32_t width, uint32_t height, uint32_t frameRate,
                        uint8_t *sps,uint8_t sps_size, uint8_t *pps, uint8_t pps_size);
int RTMP264_SendSpsPps(uint8_t *pps,uint32_t pps_len, uint8_t * sps,uint32_t sps_len,
                       uint32_t nTimeStamp);
uint32_t RTMP264_GetTimeStamp(uint32_t tick, uint32_t fps);
void RTMP264_Close();  

#endif /*__push_h*/
