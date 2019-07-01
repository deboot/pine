#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../librtmp/rtmp.h"
#include "../librtmp/rtmp_sys.h"
#include "../librtmp/amf.h"
#include "../librtmp/log.h"

#define RTMP_HEAD_SIZE   (sizeof(RTMPPacket)+RTMP_MAX_HEADER_SIZE)
#define BUFFER_SIZE 32768

typedef struct _RTMPMetadata
{
    //video, must be h264 type
    unsigned int    nWidth;
    unsigned int    nHeight;
    unsigned int    nFrameRate;
    unsigned int    nSpsLen;
    unsigned char   *Sps;
    unsigned int    nPpsLen;
    unsigned char   *Pps;
} RTMPMetadata,*LPRTMPMetadata;


RTMP* m_pRtmp;  
RTMPMetadata metaData = {};

/**
 * init RTMP Server Connection
 *
 * @param url is RTMP Server address.
 *					
 * @success 1, on error 0
 */ 
int RTMP264_Connect(const char* url)  
{  
	m_pRtmp = RTMP_Alloc();
	RTMP_Init(m_pRtmp);

    if (RTMP_SetupURL(m_pRtmp,(char*)url) == FALSE) {
		RTMP_Free(m_pRtmp);
		return false;
	}

    RTMP_LogLevel loglvl=RTMP_LOGDEBUG;
    RTMP_LogSetLevel(loglvl);

    /*Set RTMP Stream as writable, it must be set before connection.*/
	RTMP_EnableWrite(m_pRtmp);

    if (RTMP_Connect(m_pRtmp, NULL) == FALSE) {
		RTMP_Free(m_pRtmp);
		return false;
	} 

    if (RTMP_ConnectStream(m_pRtmp,0) == FALSE) {
		RTMP_Close(m_pRtmp);
		RTMP_Free(m_pRtmp);
		return false;
	}
	return true;  
}  


/**
 * Disconnection from RTMP server and realse resource.
 *
 */    
void RTMP264_Close()  
{  
    if(m_pRtmp) {
		RTMP_Close(m_pRtmp);  
		RTMP_Free(m_pRtmp);  
		m_pRtmp = NULL;  
    }
} 

/**
 * Sned H.264 video sps and pps info
 *
 * @param pps H.264 video pps data info
 * @param pps_len H.264 video pps data length
 * @param sps H.264 video sps data info
 * @param sps_len H.264 video sps data length
 *
 * @success 1, on error 0
 */
int RTMP264_SendSpsPps(unsigned char *pps,int pps_len,unsigned char * sps,int sps_len, unsigned int nTimeStamp)
{
    RTMPPacket * packet=NULL;
    unsigned char * body=NULL;
    int i;

    packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE+1024);
    memset(packet,0,RTMP_HEAD_SIZE+1024);
    packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
    body = (unsigned char *)packet->m_body;

    i = 0;
    body[i++] = 0x17;
    body[i++] = 0x00;

    body[i++] = 0x00;
    body[i++] = 0x00;
    body[i++] = 0x00;

    /*AVCDecoderConfigurationRecord*/
    body[i++] = 0x01;
    body[i++] = sps[1];
    body[i++] = sps[2];
    body[i++] = sps[3];
    body[i++] = 0xff;

    /*sps*/
    body[i++]   = 0xe1;
    body[i++] = (sps_len >> 8) & 0xff;
    body[i++] = sps_len & 0xff;
    memcpy(&body[i],sps,sps_len);
    i +=  sps_len;

    /*pps*/
    body[i++]   = 0x01;
    body[i++] = (pps_len >> 8) & 0xff;
    body[i++] = (pps_len) & 0xff;
    memcpy(&body[i],pps,pps_len);
    i +=  pps_len;

    packet->m_packetType      = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nBodySize       = i;
    packet->m_nChannel        = 0x04;
    packet->m_nTimeStamp      = nTimeStamp;
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType      = RTMP_PACKET_SIZE_MEDIUM;
    packet->m_nInfoField2     = m_pRtmp->m_stream_id;

    int nRet = FALSE;
    if (RTMP_IsConnected(m_pRtmp)) {
        nRet = RTMP_SendPacket(m_pRtmp,packet,TRUE);
    }
    free(packet);
    return nRet;
}

/**
 * Send H.264 video data per frame.
 *
 * @param data a H.264 video frame.
 * @param size of frame.
 * @param bIsKeyFrame if current frame is I frame,set this flag.
 * @param nTimeStamp current TimeStamp 1000/fps
 *
 * @success 1, on error 0
 */
int RTMP264_Send(char *data,unsigned int length,int bIsKeyFrame, uint32_t timestamp)
{
     RTMPPacket *packet = (RTMPPacket *) malloc(RTMP_HEAD_SIZE + length + 9);
     memset(packet, 0, RTMP_HEAD_SIZE);
     packet->m_body = (char *) packet + RTMP_HEAD_SIZE;
     packet->m_nBodySize = length + 9;

     /* send video packet*/
     uint8_t *body = (uint8_t *) packet->m_body;
     memset(body, 0, length + 9);

     /*key frame*/
     if (bIsKeyFrame) {
         body[0] = 0x17; //UB[4] FrameType: 1(key frame, a seekable frame);UB[4] CodecID:7(AVC)     }
         RTMP264_SendSpsPps(metaData.Pps,metaData.nPpsLen,metaData.Sps,metaData.nSpsLen,timestamp);
     }else {
         body[0] = 0x27; //UB[4] FrameType: 2(inter frame, a non-seekable frame);UB[4] CodecID:7(AVC)

     }
     body[1] = 0x01; //UI8 AVCPacketType:1(AVC NALU)
     body[2] = 0x00; //SI24 CompositionTime there isn't B frame,set it 0
     body[3] = 0x00;
     body[4] = 0x00;

     body[5] = (length >> 24) & 0xff;    // SI32 NALU length
     body[6] = (length >> 16) & 0xff;
     body[7] = (length >> 8) & 0xff;
     body[8] = (length) & 0xff;

     /*copy data*/
     memcpy(&body[9], data, length);     // UI[length] NALU data

     packet->m_hasAbsTimestamp = 0;
     packet->m_packetType      = RTMP_PACKET_TYPE_VIDEO;
     packet->m_nInfoField2     = m_pRtmp->m_stream_id;
     packet->m_nChannel        = 0x04;
     packet->m_headerType      = RTMP_PACKET_SIZE_LARGE;
     packet->m_nTimeStamp      = timestamp;

     int bRet = FALSE;

     if (RTMP_IsConnected(m_pRtmp)) {
         bRet = RTMP_SendPacket(m_pRtmp, packet, TRUE);
     }

     free(packet);
     return bRet;
}
int RTMP264_SetMetadata(uint32_t width, uint32_t height, uint32_t frameRate,
                        uint8_t *sps,uint8_t sps_size, uint8_t *pps, uint8_t pps_size)
{
    metaData.nWidth = width;
    metaData.nHeight = height;
    metaData.nFrameRate = frameRate;

    if(sps != NULL && sps_size > 0) {
        metaData.Sps = (unsigned char*)malloc(sps_size);
        memcpy(metaData.Sps,sps,sps_size);
        metaData.nSpsLen = sps_size;
    }

    if(pps != NULL && pps_size > 0){
        metaData.Pps = (unsigned char*)malloc(pps_size);
        memcpy(metaData.Pps,pps,pps_size);
        metaData.nPpsLen  = pps_size;
    }
}

uint32_t RTMP264_GetTimeStamp(uint32_t tick, uint32_t fps)
{
    return tick + (1000 / fps);
}


