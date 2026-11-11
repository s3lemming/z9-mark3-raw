#include "native_settings_bank.h"
static uint32_t rd32(const uint8_t*p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static int rom_read(void*user,uint64_t address,void*out,size_t bytes){
 (void)user;if(!out)return 0;
 int A=address>=0x45e20000ull&&address<=0x45e60000ull&&bytes<=0x45e60000ull-address;
 int B=address>=0x46200000ull&&address<=0x46240000ull&&bytes<=0x46240000ull-address;
 if(!A&&!B)return 0;
 const volatile uint8_t*s=(const volatile void*)(uintptr_t)address;uint8_t*d=out;
 for(size_t i=0;i<bytes;i++)d[i]=s[i];
 return 1;
}
int m3_native_bank_bind_rom532(struct m3_native_bank_api*out,void*user,uint32_t(*owned)(void*)){
 if(!out||!owned)return 0;
 out->user=user;out->is_owned=owned;out->choose=(void*)(uintptr_t)0x40d74a94;out->end=(void*)(uintptr_t)0x40d75960;out->read=rom_read;return 1;
}
static uint32_t provider_owned(void*user){const struct m3_native_bank_provider*p=user;return p&&p->api&&p->api->is_owned?p->api->is_owned(p->api->user):0;}
static int provider_read(void*user,uint64_t address,void*out,size_t bytes){const struct m3_native_bank_provider*p=user;return p&&p->api&&p->api->read?p->api->read(p->api->user,address,out,bytes):0;}
int m3_native_load_bind_bank_provider(struct m3_native_load_api*out,
 struct m3_native_bank_provider*p,const struct m3_native_bank_api*a,
 const struct m3_native_settings_schema*g){
 if(!out||!p||!a||!a->is_owned||!a->choose||!a->end||!a->read||g!=(void*)(uintptr_t)0xa03478120ull)return 0;
 p->api=a;p->native_global_schema=g;out->user=p;out->is_owned=provider_owned;out->select_native=m3_native_bank_select;out->read=provider_read;return 1;
}
uint32_t m3_native_bank_select(void*user,const struct m3_native_storage_descriptor*d,
 const struct m3_native_settings_schema*s,uint64_t*bank,uint64_t*footer,uint32_t*counter){
 const struct m3_native_bank_provider*p=user;
 if(!p||!p->api||!bank||!footer||!counter||!m3_native_settings_domain(d,s)||p->native_global_schema!=(void*)(uintptr_t)0xa03478120ull)return 0;
 const struct m3_native_bank_api*a=p->api;
 if(!a->is_owned||!a->choose||!a->end||!a->read||a->is_owned(a->user)!=1)return 0;
 const struct m3_native_settings_schema*g=p->native_global_schema;
 if(g->count!=s->count||g->records!=s->records||g->auxiliary_count!=s->auxiliary_count||g->auxiliary!=s->auxiliary)return 0;
 const uint8_t*flags=d->flags;
 const uint8_t expected[8]={2,0,0,0,1,0,1,1};
 for(unsigned i=0;i<8;i++)if(flags[i]!=expected[i])return 0;
 struct m3_native_bank_area area;
 area.descriptor20=(uint32_t)(d->native16>>32);area.descriptor24=d->native24;
 area.bank_A=d->bank_A;area.bank_B=d->bank_B;area.bytes=d->bank_A_bytes;area.padding28=0;area.flags=d->flags;
 int32_t chosen=a->choose(d->body_bytes,&area);if(chosen<0||chosen>1)return 0;
 uint64_t b=chosen?d->bank_B:d->bank_A;uint32_t bytes=chosen?d->bank_B_bytes:d->bank_A_bytes;
 uint64_t f=a->end(s,b,bytes);if(f<b||f>b+bytes-8||((f-b)&3))return 0;
 uint8_t h[8]={0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5};
 if(a->read(a->user,f,h,8)!=1||rd32(h)!=0x4d444e45||rd32(h+4)>0xfffffff0u||a->is_owned(a->user)!=1)return 0;
 if(g->count!=s->count||g->records!=s->records||g->auxiliary_count!=s->auxiliary_count||g->auxiliary!=s->auxiliary)return 0;
 *bank=b;*footer=f;*counter=rd32(h+4);return M3_NATIVE_BANK_FIELDS_COMPLETE;
}
