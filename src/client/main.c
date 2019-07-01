#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../librtmp/rtmp.h"
#include "../librtmp/log.h"

int main(int argc, char **argv)
{
   RTMP * rtmp = RTMP_Alloc();

   RTMP_Init(rtmp);
   char *URL = argv[1];

   RTMP_LogLevel loglvl=RTMP_LOGDEBUG;
   RTMP_LogSetLevel(loglvl);

   if(!RTMP_SetupURL(rtmp,URL)){
      RTMP_Free(rtmp);
   }

   rtmp->Link.lFlags |=RTMP_LF_LIVE;

   RTMP_SetBufferMS(rtmp, 3600 * 1000);

   if(!RTMP_Connect(rtmp, NULL)) {
      RTMP_Free(rtmp);
      return -1;
   }

   if(!RTMP_ConnectStream(rtmp, 0)){
      RTMP_Free(rtmp);
      return -1;
   }

   int total = 0;

   #define MAX_BUFF_SIZE 1024*1024*3

   char *buffer = (char *)malloc(MAX_BUFF_SIZE);
   memset(buffer, 0, MAX_BUFF_SIZE);

   FILE *fp = fopen("out.h264", "wb");
   double mega = 1024.0 * 1024;


   //simple write FLV file.
   /*
   while(1) {
      n = RTMP_Read(rtmp, buffer, MAX_BUFF_SIZE);
      if(n < 1) break;
      fwrite(buffer, 1, n, fp);

      total += n;
      printf("Receive:%6dB, Total: %5.3fBM\n",n, total/mega);
    }
    */

    int firt_entry = 0;
    RTMPPacket packet = {};
    static unsigned char const start_code[4] = {0x00, 0x00, 0x00, 0x01};
     while(RTMP_ReadPacket(rtmp, &packet)) {
        if(RTMPPacket_IsReady(&packet)) {
           if(packet.m_packetType == RTMP_PACKET_TYPE_VIDEO) {

              RTMPPacket_Dump(&packet);
              //RTMP_LogHexString(RTMP_LOGDEBUG,packet.m_body,packet.m_nBodySize);

              char *data = packet.m_body;
              if(data[0] == 0x17 && data[1] == 0x00){ //sps and pps
                 fwrite(start_code, 1, 4, fp );
                 int spsnum = data[10]&0x1f;
                 int number_sps = 11;
                 int count_sps = 1;
                 while (count_sps <= spsnum){
                    int spslen =(data[number_sps]&0xFF)<<8 |(data[number_sps+1]&0xFF);
                    number_sps += 2;
                    fwrite(data+number_sps, 1, spslen, fp );
                    fwrite(start_code, 1, 4, fp );
                    number_sps += spslen;
                    count_sps ++;
                 }

                 int ppsnum = data[number_sps]&0x1f;
                 int number_pps = number_sps+1;
                 int count_pps = 1;

                 while (count_pps <= ppsnum){
                    int ppslen =(data[number_pps]&0xFF)<<8|data[number_pps+1]&0xFF;
                    number_pps += 2;
                    fwrite(data+number_pps, 1, ppslen, fp );
                    number_pps += ppslen;
                    count_pps ++;
                 }

                 firt_entry = TRUE;
              }else if(firt_entry == TRUE) {
                 int len =0;
                 int num =5;

                 fwrite(start_code, 1, 4, fp );

                 while(num < packet.m_nBodySize) {
                    len =(data[num]&0xFF)<<24|(data[num+1]&0xFF)<<16|(data[num+2]&0xFF)<<8|data[num+3]&0xFF;
                    num = num+4;
                    fwrite(data+num, 1, len, fp );
                    num = num + len;
                 }

              }

              total += packet.m_nBodySize;
              printf("Receive:%6dB, Total: %5.3fBM\n",packet.m_nBodySize, total/mega);
              RTMPPacket_Free(&packet);
              RTMPPacket_Reset(&packet);
           }
        }
     }

   if(fp != NULL) { fclose(fp); }

   if(rtmp != NULL) {
      RTMP_Close(rtmp);
      RTMP_Free(rtmp);
   }

   return 0;
}
