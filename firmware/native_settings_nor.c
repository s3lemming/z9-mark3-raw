#include "native_settings_nor.h"
static int overlap(uint64_t a,uint64_t an,uint64_t b,uint64_t bn){return a<b+bn&&b<a+an;}
static int tail(const struct m3_native_save_frame*f,uint64_t address,size_t bytes){
 if(!m3_native_save_context_owned(f))return 0;
 const struct m3_native_save_cookie*c=&f->cookie;
 if(!c->locked||c->enabled!=1||c->tail_called!=1||!m3_native_settings_domain(c->descriptor,c->schema)||c->cursor%4||!bytes||bytes>88)return 0;
 uint64_t cursor=c->cursor;int bank=(cursor>=0x45e20000ull&&cursor<=0x45e60000ull-88)||(cursor>=0x46200000ull&&cursor<=0x46240000ull-88);
 return bank&&address>=cursor&&address<=cursor+88&&bytes<=cursor+88-address;
}
int m3_native_tail_program(void*context,uint64_t address,uint32_t word){
 struct m3_native_save_frame*f=context;if(address%4||!tail(f,address,4))return 0;
 /* The original worker uses this pointer until its completion event returns.
  * Keep this aligned stack word alive across the whole native call. */
 uint32_t value=word;uint32_t rc=((uint32_t(*)(uint64_t,const void*,uint32_t))(uintptr_t)0x40ca8374)(address,&value,2);
 return rc==0&&tail(f,address,4);
}
int m3_native_tail_read(void*context,uint64_t address,void*out,size_t bytes){
 struct m3_native_save_frame*f=context;if(!out||(uintptr_t)out>UINTPTR_MAX-bytes||!tail(f,address,bytes))return 0;
 uint64_t target=(uintptr_t)out;
 if(overlap(target,bytes,(uintptr_t)f,sizeof *f)||overlap(target,bytes,(uintptr_t)f->binding,sizeof *f->binding)||overlap(target,bytes,(uintptr_t)f->binding->gate,sizeof *f->binding->gate)||overlap(target,bytes,0x45e20000,0x40000)||overlap(target,bytes,0x46200000,0x40000))return 0;
 uint8_t*d=out;const volatile uint8_t*s=(const volatile uint8_t*)(uintptr_t)address;
 for(size_t i=0;i<bytes;i++)d[i]=s[i];
 return tail(f,address,bytes);
}
