#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "hsfc_settings_record.h"
static uint32_t crc(const unsigned char*p,size_t n){uint32_t c=0xffffffffu;for(size_t i=0;i<n;i++){c^=p[i];for(unsigned j=0;j<8;j++)c=(c&1)?(c>>1)^0xedb88320u:c>>1;}return c^0xffffffffu;}
int main(void){
 unsigned char blob[80],bad[80];struct m3_hsfc_settings s={0,3,0x1020304050607080ull},out={99,99,99},untouched=out;unsigned options[]={0,2,4,8,10};unsigned checks=0;
 for(unsigned i=0;i<5;i++)for(unsigned codec=3;codec<=4;codec++){s.option=options[i];s.codec=codec;assert(m3_settings_encode(&s,blob,80)==M3_RECORD_OK);assert(m3_settings_decode(blob,80,&out)==M3_RECORD_OK);assert(out.option==s.option&&out.codec==s.codec&&out.generation==s.generation);checks++;}
 /* Independent expected little-endian header and standard CRC-32 vector. */
 assert(crc((const unsigned char*)"123456789",9)==0xcbf43926u);assert(!memcmp(blob,"ENDMHMS3ENDM\1\0\0\0ENDM\12\0\0\0",24));
 for(unsigned i=0;i<80;i++)for(unsigned bit=0;bit<8;bit++){memcpy(bad,blob,80);bad[i]^=1u<<bit;out=untouched;assert(m3_settings_decode(bad,80,&out)==M3_RECORD_INVALID);assert(!memcmp(&out,&untouched,sizeof out));checks++;}
 out=untouched;assert(m3_settings_decode(0,0,&out)==M3_RECORD_ABSENT&&!memcmp(&out,&untouched,sizeof out));assert(m3_settings_decode(blob,79,&out)==M3_RECORD_INVALID);assert(m3_settings_decode(blob,81,&out)==M3_RECORD_INVALID);
 memcpy(bad,blob,80);bad[12]=2;unsigned char body[36];for(unsigned i=0;i<9;i++)memcpy(body+i*4,bad+i*8+4,4);uint32_t c=crc(body,36);for(unsigned i=0;i<4;i++)bad[76+i]=(unsigned char)(c>>(i*8));assert(m3_settings_decode(bad,80,&out)==M3_RECORD_VERSION&&!memcmp(&out,&untouched,sizeof out));
 memset(bad,0x5a,sizeof bad);s.option=1;assert(m3_settings_encode(&s,bad,80)==M3_RECORD_INVALID);for(unsigned i=0;i<80;i++)assert(bad[i]==0x5a);s.option=0;s.generation=0;assert(m3_settings_encode(&s,bad,80)==M3_RECORD_INVALID);s.generation=1;s.codec=1;assert(m3_settings_encode(&s,bad,80)==M3_RECORD_INVALID);s.codec=3;assert(m3_settings_encode(&s,bad,79)==M3_RECORD_INVALID);
 assert(m3_settings_migrate_legacy(0,&out)==M3_RECORD_OK&&out.option==8&&out.codec==4&&out.generation==1);assert(m3_settings_migrate_legacy(1,&out)==M3_RECORD_OK&&out.option==10&&out.codec==4);out=untouched;assert(m3_settings_migrate_legacy(2,&out)==M3_RECORD_INVALID&&!memcmp(&out,&untouched,sizeof out));
 printf("%u format combinations and single-bit corruption checks, independent CRC vector, unknown schema, no-touch-on-error and legacy JPEG-only migration passed. Native flash transaction/banks not qualified.\n",checks);return 0;
}
