#include "native_settings_context.h"
static int span(const void*p,size_t n){return p&&(uintptr_t)p<=UINTPTR_MAX-n;}
static int overlap(const void*a,size_t an,const void*b,size_t bn){return a&&b&&(uintptr_t)a<(uintptr_t)b+bn&&(uintptr_t)b<(uintptr_t)a+an;}
static int valid(const struct m3_native_save_binding*b){
 if(!span(b,sizeof *b)||(uintptr_t)b%_Alignof(struct m3_native_save_binding)||!b->gate||!b->gate_api||!b->release_failed)return 0;
 const struct m3_native_save_api*a=&b->callbacks;
 return !a->user&&!a->acquire&&!a->release&&!a->tail.context&&a->select&&a->verify&&a->accept&&a->failed&&a->tail.read&&a->tail.program_word;
}
int m3_native_save_context_owned(const struct m3_native_save_frame*f){
 return span(f,sizeof *f)&&(uintptr_t)f%_Alignof(struct m3_native_save_frame)==0&&f->binding&&m3_native_gate_owned(f->binding->gate,&f->token);
}
static int acquire(void*user){struct m3_native_save_frame*f=user;return m3_native_gate_acquire(f->binding->gate_api,f->binding->gate,&f->token)==M3_GATE_OK;}
static int release(void*user){
 struct m3_native_save_frame*f=user;
 if(m3_native_gate_release(f->binding->gate_api,f->binding->gate,&f->token)==M3_GATE_OK)return 1;
 f->binding->release_failed(f);return 0;
}
uint32_t m3_native_save_context_prepare(const struct m3_native_save_binding*b,const struct m3_native_storage_descriptor*d,const struct m3_native_settings_schema*s,struct m3_native_save_frame*f){
 if(!span(f,sizeof *f)||(uintptr_t)f%_Alignof(struct m3_native_save_frame)||
    (b&&(!span(b,sizeof *b)||overlap(f,sizeof *f,b,sizeof *b)))||
    (d&&(!span(d,sizeof *d)||overlap(f,sizeof *f,d,sizeof *d)))||
    (s&&(!span(s,sizeof *s)||overlap(f,sizeof *f,s,sizeof *s))))return 2;
 volatile uint8_t*dst=(volatile void*)f;for(size_t i=0;i<sizeof *f;i++)dst[i]=0;
 if(!valid(b))return m3_native_save_prepare(0,d,s,&f->cookie);
 f->binding=b;
 volatile uint8_t*out=(volatile void*)&f->api;const uint8_t*in=(const void*)&b->callbacks;
 for(size_t i=0;i<sizeof f->api;i++)out[i]=in[i];
 f->api.user=f;f->api.acquire=acquire;f->api.release=release;f->api.tail.context=f;
 return m3_native_save_prepare(&f->api,d,s,&f->cookie);
}
