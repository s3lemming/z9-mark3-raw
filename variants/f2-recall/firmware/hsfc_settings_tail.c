#include "hsfc_settings_tail.h"
static uint32_t rd32(const unsigned char*p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static void wr32(unsigned char*p,uint32_t x){for(unsigned i=0;i<4;i++)p[i]=(unsigned char)(x>>(i*8));}
static int equal(const unsigned char*a,const unsigned char*b,size_t n){for(size_t i=0;i<n;i++)if(a[i]!=b[i])return 0;return 1;}
enum m3_tail_result m3_settings_write_tail(const struct m3_tail_api*api,uint64_t address,size_t room,
 const struct m3_hsfc_settings*s,uint32_t native_counter,struct m3_tail_receipt*r){
 if(!r)return M3_TAIL_INVALID;
 *r=(struct m3_tail_receipt){0,0,0};
 if(!api||!api->program_word||!api->read||(address&3)||room<M3_SETTINGS_TAIL_BYTES||address>UINT64_MAX-M3_SETTINGS_TAIL_BYTES)return M3_TAIL_INVALID;
 unsigned char data[M3_SETTINGS_TAIL_BYTES],actual[M3_SETTINGS_TAIL_BYTES];
 if(m3_settings_encode(s,data,M3_SETTINGS_CARRIER_BYTES)!=M3_RECORD_OK)return M3_TAIL_INVALID;
 if(!api->read(api->context,address,actual,sizeof actual))return M3_TAIL_VERIFY_FAILED;
 for(size_t i=0;i<sizeof actual;i++)if(actual[i]!=255)return M3_TAIL_INVALID;
 wr32(data+M3_SETTINGS_CARRIER_BYTES,0x4d444e45u);wr32(data+M3_SETTINGS_CARRIER_BYTES+4,native_counter);
 /* The final ORIGINAL counter is written behind an erased first marker.
  * Filling backward and committing the first magic word last prevents an
  * intermediate settings word from becoming the visible native counter.
  * Flash interruption may invalidate the target journal; native alternate
  * bank recovery must still be independently qualified by the caller. */
 for(size_t i=M3_SETTINGS_TAIL_BYTES/4;i-->1;){
  r->program_calls++;if(!api->program_word(api->context,address+i*4,rd32(data+i*4)))return M3_TAIL_PROGRAM_FAILED;
 }
 if(!api->read(api->context,address+4,actual+4,sizeof actual-4)||!equal(actual+4,data+4,sizeof actual-4))return M3_TAIL_VERIFY_FAILED;
 r->commit_word_issued=1;r->program_calls++;
 if(!api->program_word(api->context,address,rd32(data)))return M3_TAIL_PROGRAM_FAILED;
 if(!api->read(api->context,address,actual,sizeof actual)||!equal(actual,data,sizeof actual))return M3_TAIL_VERIFY_FAILED;
 r->verified=1;return M3_TAIL_COMMITTED;
}
