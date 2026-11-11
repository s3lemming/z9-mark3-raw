#include "native_settings_view.h"
static uint32_t rd32(const uint8_t*p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static uint64_t rd64(const uint8_t*p){return rd32(p)|(uint64_t)rd32(p+4)<<32;}
int m3_native_view_bind_rom532(struct m3_native_view_api*out,void*user,uint32_t(*owned)(void*)){
 if(!out||!owned)return 0;
 out->user=user;out->is_owned=owned;out->buffer=(void*)(uintptr_t)0x4101a890;out->schema=(void*)(uintptr_t)0x4101a960;out->storage=(void*)(uintptr_t)0x40d76084;out->attributes=(void*)(uintptr_t)0x40d76068;return 1;
}
enum m3_native_view_result m3_native_settings_view_owned(const struct m3_native_view_api*a,struct m3_native_settings_view*out){
 if(!a||!out||!a->is_owned||!a->buffer||!a->schema||!a->storage||!a->attributes)return M3_NATIVE_VIEW_INVALID;
 if(a->is_owned(a->user)!=1)return M3_NATIVE_VIEW_NOT_OWNED;
 const uint8_t*reg=a->storage(15);
 if(reg!=(void*)(uintptr_t)0xa02925988ull||rd32(reg)!=15||rd32(reg+36)!=15)return M3_NATIVE_VIEW_NOT_READY;
 /* Native attribute getter has NO index bound; validate exactF index first. */
 const uint8_t*attr=a->attributes(15);
 if(attr!=(void*)(uintptr_t)0xa02926b10ull)return M3_NATIVE_VIEW_NOT_READY;
 struct m3_native_settings_view view;
 volatile uint8_t*z=(volatile void*)&view;for(size_t i=0;i<sizeof view;i++)z[i]=0;
 view.buffer.bytes=UINT32_MAX;view.buffer.data=a->buffer(&view.buffer.bytes);
 view.schema.count=UINT32_MAX;view.schema.auxiliary_count=UINT32_MAX;a->schema(&view.schema);
 view.storage.native0=rd64(reg+8);view.storage.native8=rd64(reg+16);view.storage.native16=rd64(reg+24);view.storage.native24=rd32(reg+32);
 view.storage.bank_A=rd64(attr);view.storage.bank_B=rd64(attr+8);view.storage.bank_A_bytes=rd32(attr+16);view.storage.bank_B_bytes=rd32(attr+20);view.storage.flags=(void*)(attr+28);view.storage.body=view.buffer.data;view.storage.body_bytes=view.buffer.bytes;
 const uint8_t flags[8]={2,0,0,0,1,0,1,1};
 if(!m3_native_settings_domain(&view.storage,&view.schema)||rd32(attr+24)!=0||rd32(attr+36)!=1)return M3_NATIVE_VIEW_NOT_READY;
 for(unsigned i=0;i<8;i++)if(attr[28+i]!=flags[i])return M3_NATIVE_VIEW_NOT_READY;
 if(a->is_owned(a->user)!=1)return M3_NATIVE_VIEW_NOT_OWNED;
 const uint8_t*s=(const void*)&view;volatile uint8_t*d=(volatile void*)out;
 for(size_t i=0;i<sizeof view;i++)d[i]=s[i];
 return M3_NATIVE_VIEW_OK;
}
