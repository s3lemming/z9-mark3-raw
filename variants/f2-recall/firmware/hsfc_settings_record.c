#include "hsfc_settings_record.h"
static uint32_t read32(const unsigned char*p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static void write32(unsigned char*p,uint32_t x){for(unsigned i=0;i<4;i++)p[i]=(unsigned char)(x>>(i*8));}
static uint32_t crc(const unsigned char*p,size_t n){uint32_t c=~0u;for(size_t i=0;i<n;i++){c^=p[i];for(unsigned j=0;j<8;j++)c=(c>>1)^(0xedb88320u& (0u-(c&1)));}return ~c;}
static int valid(const struct m3_hsfc_settings*s){return s&&s->generation&&(s->codec==3||s->codec==4)&&(s->option==0||s->option==2||s->option==4||s->option==8||s->option==10);}
enum m3_record_result m3_settings_encode(const struct m3_hsfc_settings*s,void*output,size_t capacity){
 if(!valid(s)||!output||capacity<M3_SETTINGS_CARRIER_BYTES)return M3_RECORD_INVALID;
 unsigned char body[40]={0},*out=output;write32(body,0x33534d48u);write32(body+4,1);write32(body+8,10);write32(body+12,(uint32_t)s->generation);write32(body+16,(uint32_t)(s->generation>>32));write32(body+20,s->option);write32(body+24,s->codec);write32(body+36,crc(body,36));
 for(unsigned i=0;i<10;i++){write32(out+i*8,0x4d444e45u);write32(out+i*8+4,read32(body+i*4));}return M3_RECORD_OK;
}
enum m3_record_result m3_settings_decode(const void*input,size_t bytes,struct m3_hsfc_settings*out){
 if(!input&&bytes==0)return M3_RECORD_ABSENT;
 if(!input||!out||bytes!=M3_SETTINGS_CARRIER_BYTES)return M3_RECORD_INVALID;
 const unsigned char*p=input;unsigned char b[40];for(unsigned i=0;i<10;i++){if(read32(p+i*8)!=0x4d444e45u)return M3_RECORD_INVALID;write32(b+i*4,read32(p+i*8+4));}
 if(read32(b)!=0x33534d48u||read32(b+8)!=10||read32(b+36)!=crc(b,36))return M3_RECORD_INVALID;
 if(read32(b+4)!=1)return M3_RECORD_VERSION;
 struct m3_hsfc_settings s={read32(b+20),read32(b+24),read32(b+12)|(uint64_t)read32(b+16)<<32};
 if(read32(b+28)||read32(b+32)||!valid(&s))return M3_RECORD_INVALID;
 *out=s;return M3_RECORD_OK;
}
enum m3_record_result m3_settings_migrate_legacy(uint32_t quality,struct m3_hsfc_settings*out){
 if(quality>1||!out)return M3_RECORD_INVALID;
 /* Initial implementation uses HE. HE-star remains a future format. */
 struct m3_hsfc_settings s={quality?10u:8u,4,1};*out=s;return M3_RECORD_OK;
}
