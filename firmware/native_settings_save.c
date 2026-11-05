#include "native_settings_save.h"
uint32_t m3_native_save_prepare(const struct m3_native_save_api*api,
 const struct m3_native_storage_descriptor*d,const struct m3_native_settings_schema*s,
 struct m3_native_save_cookie*c){
 if(!c)return 2;
 /* A large aggregate initializer lowered to libc memset under the actual
  * general-register-only firmware flags. Keep this96B stack initialization
  * explicit, with no hidden runtime dependency. */
 volatile uint8_t*bytes=(volatile void*)c;
 for(size_t i=0;i<sizeof *c;i++)bytes[i]=0;
 c->descriptor=d;c->schema=s;c->api=api;c->magic=0x3353415645434f4full;
 if(!m3_native_settings_domain(d,s))return 0;
 c->enabled=2;
 if(!api||!api->acquire||!api->release||!api->select||!api->verify||!api->accept||!api->failed||!api->tail.program_word||!api->tail.read||!api->acquire(api->user))return 2;
 c->locked=1;c->settings=(struct m3_hsfc_settings){UINT32_MAX,UINT32_MAX,0};c->force=UINT32_MAX;
 enum m3_save_selection selected=api->select(api->user,s,&c->settings,&c->force);
 if(selected==M3_SAVE_NATIVE){c->enabled=0;return 0;}
 uint8_t scratch[M3_SETTINGS_CARRIER_BYTES];
 if(selected!=M3_SAVE_ENCODE||c->force>1||(c->settings.codec!=3&&c->settings.codec!=4)||m3_settings_encode(&c->settings,scratch,sizeof scratch)!=M3_RECORD_OK)return 2;
 c->enabled=1;return 1;
}
uint32_t m3_native_save_force(struct m3_native_save_cookie*c,uint32_t original){
 return original|(c&&c->enabled==1&&c->locked&&c->force==1?1u:0u);
}
void m3_native_save_footer(struct m3_native_save_cookie*c,uint64_t cursor,uint32_t counter){
 if(!c||c->enabled!=1||!c->locked||!c->api)return;
 if(c->tail_called){c->tail_result=M3_TAIL_INVALID;c->tail_receipt.verified=0;return;}
 c->tail_called=1;c->cursor=cursor;c->counter=counter;
 const struct m3_native_storage_descriptor*d=c->descriptor;uint64_t base=0,end=0;
 if(cursor>=d->bank_A&&cursor<d->bank_A+d->bank_A_bytes){base=d->bank_A;end=base+d->bank_A_bytes;}
 if(cursor>=d->bank_B&&cursor<d->bank_B+d->bank_B_bytes){base=d->bank_B;end=base+d->bank_B_bytes;}
 c->tail_result=M3_TAIL_INVALID;
 if(!base||cursor%4||end-cursor<M3_SETTINGS_TAIL_BYTES)return;
 c->tail_result=m3_settings_write_tail(&c->api->tail,cursor,(size_t)(end-cursor),&c->settings,counter,&c->tail_receipt);
}
uint32_t m3_native_save_end(struct m3_native_save_cookie*c,uint32_t original){
 if(!c||c->magic!=0x3353415645434f4full)return 0;
 c->original_status=original;uint32_t result=original;
 if(c->enabled){
  result=0;
  if(c->enabled==1&&c->locked&&(original&1)&&(!c->force||c->tail_called)&&
   (!c->tail_called||(c->tail_result==M3_TAIL_COMMITTED&&c->tail_receipt.verified))&&c->api->verify(c->api->user,c)&&c->api->accept(c->api->user,c))result=1;
 }
 if(c->enabled&&!result&&c->locked)c->api->failed(c->api->user,c);
 if(c->locked){c->locked=0;if(c->api->release(c->api->user)!=1)result=0;}
 return result;
}
