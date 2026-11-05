#include "native_settings_verify.h"
static uint32_t owned(void*user){return m3_native_save_context_owned(user)?1u:0u;}
static int schema_equal(const struct m3_native_settings_schema*a,const struct m3_native_settings_schema*b){return a->count==b->count&&a->records==b->records&&a->auxiliary_count==b->auxiliary_count&&a->auxiliary==b->auxiliary;}
static int descriptor_equal(const struct m3_native_storage_descriptor*a,const struct m3_native_storage_descriptor*b){
 return a->native0==b->native0&&a->native8==b->native8&&a->native16==b->native16&&a->native24==b->native24&&a->bank_A==b->bank_A&&a->bank_B==b->bank_B&&a->bank_A_bytes==b->bank_A_bytes&&a->bank_B_bytes==b->bank_B_bytes&&a->flags==b->flags&&a->body==b->body&&a->body_bytes==b->body_bytes;
}
static uint32_t u32(const uint8_t*p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
/* The original comparator rewrites each record's dirty word. This equivalent
 * equality walk owns only52 bitmap bytes and preserves every live dirty word.
 * Later delta records overwrite the same ID's previous comparison result. */
static int compare(const struct m3_native_bank_api*a,const struct m3_native_storage_descriptor*d,const struct m3_native_settings_schema*s,uint64_t bank,uint64_t footer){
 if(!a||!a->read||!a->is_owned||a->is_owned(a->user)!=1||s->count!=202)return 0;
 volatile uint8_t seen[26],different[26];for(unsigned i=0;i<26;i++){seen[i]=0;different[i]=0;}
 const struct m3_native_settings_record*records=s->records;uint64_t cursor=bank;uint8_t bytes[64];unsigned steps=0;
 while(cursor<footer){
  if(++steps>0x40000/8||footer-cursor<8||a->read(a->user,cursor,bytes,8)!=1)return 0;
  uint32_t id=u32(bytes),length=u32(bytes+4);
  if(id==0x4d444e45u){cursor+=8;continue;}
  if(id>=202||!length||length!=records[id].bytes||length>d->body_bytes)return 0;
  uint64_t source=(uintptr_t)records[id].data,body=(uintptr_t)d->body;
  if(source<body||source>body+d->body_bytes||length>body+d->body_bytes-source)return 0;
  uint32_t padded=(length+3u)&~3u;if(padded>footer-cursor-8)return 0;
  uint32_t diff=0;
  const volatile uint8_t*live=(const volatile uint8_t*)(uintptr_t)source;
  for(uint32_t offset=0;offset<length;){
   uint32_t n=length-offset;if(n>sizeof bytes)n=sizeof bytes;
   if(a->read(a->user,cursor+8+offset,bytes,n)!=1)return 0;
   for(uint32_t i=0;i<n;i++)diff|=bytes[i]^live[offset+i];
   offset+=n;
  }
  unsigned byte=id>>3,bit=1u<<(id&7);seen[byte]|=(uint8_t)bit;
  if(diff)different[byte]|=(uint8_t)bit;else different[byte]&=(uint8_t)~bit;
  cursor+=8+padded;
 }
 for(unsigned id=0;id<202;id++)if(!(seen[id>>3]&(1u<<(id&7)))||(different[id>>3]&(1u<<(id&7))))return 0;
 return cursor==footer&&a->is_owned(a->user)==1;
}
int m3_native_saved_settings_verify(void*user,const struct m3_native_save_cookie*c){
 struct m3_native_save_frame*f=user;
 if(!owned(f)||c!=&f->cookie||c->api!=&f->api||c->locked!=1||c->enabled!=1||c->original_status!=1||c->force>1||c->tail_called>1||(c->settings.codec!=3&&c->settings.codec!=4)||!m3_native_settings_domain(c->descriptor,c->schema))return 0;
 if(c->force&&!c->tail_called)return 0;
 if(c->tail_called&&(c->tail_result!=M3_TAIL_COMMITTED||c->tail_receipt.verified!=1))return 0;
 struct m3_native_view_api view_api;struct m3_native_settings_view before,after;
 if(!m3_native_view_bind_rom532(&view_api,f,owned)||m3_native_settings_view_owned(&view_api,&before)!=M3_NATIVE_VIEW_OK||!schema_equal(c->schema,&before.schema)||!descriptor_equal(c->descriptor,&before.storage))return 0;
 struct m3_native_bank_api bank_api;struct m3_native_bank_provider provider;struct m3_native_load_api load_api;
 if(!m3_native_bank_bind_rom532(&bank_api,f,owned)||!m3_native_load_bind_bank_provider(&load_api,&provider,&bank_api,(const void*)(uintptr_t)0xa03478120ull))return 0;
 struct m3_hsfc_settings found;struct m3_native_load_receipt receipt;
 if(m3_native_settings_load_owned(&load_api,&before.storage,&before.schema,&found,&receipt)!=M3_LOAD_OK||found.option!=c->settings.option||found.codec!=c->settings.codec||found.generation!=c->settings.generation)return 0;
 if(c->tail_called&&(receipt.footer!=c->cursor+80||receipt.native_counter!=c->counter))return 0;
 if(!compare(&bank_api,&before.storage,&before.schema,receipt.bank,receipt.footer))return 0;
 uint64_t bank=0,footer=0;uint32_t counter=0;
 if(m3_native_bank_select(&provider,&before.storage,&before.schema,&bank,&footer,&counter)!=7||bank!=receipt.bank||footer!=receipt.footer||counter!=receipt.native_counter)return 0;
 if(m3_native_settings_view_owned(&view_api,&after)!=M3_NATIVE_VIEW_OK||!schema_equal(&before.schema,&after.schema)||!descriptor_equal(&before.storage,&after.storage)||!owned(f))return 0;
 return compare(&bank_api,&after.storage,&after.schema,bank,footer)&&owned(f);
}
