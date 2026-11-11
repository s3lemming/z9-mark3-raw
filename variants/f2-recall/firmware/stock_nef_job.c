#include "stock_nef_job.h"
static uint16_t r16(const void *p){const uint8_t *s=p;return(uint16_t)(s[0]|(uint16_t)s[1]<<8);}
static uint32_t r32(const void *p){const uint8_t *s=p;return(uint32_t)s[0]|(uint32_t)s[1]<<8|(uint32_t)s[2]<<16|(uint32_t)s[3]<<24;}
static uint64_t r64(const void *p){const uint8_t *s=p;return(uint64_t)r32(s)|(uint64_t)r32(s+4)<<32;}
static void zero(uint8_t *p,size_t n){for(size_t i=0;i<n;i++)p[i]=0;}
static int overlap(const void *a,size_t an,const void *b,size_t bn){
 uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;
 if(x>UINTPTR_MAX-an||y>UINTPTR_MAX-bn)return 1;
 return x<y+bn&&y<x+an;
}
static int jpeg(const uint8_t *d,uint32_t w,uint32_t h){
 uint64_t p=r64(d+8);uint32_t n=r32(d+0x10);
 return r32(d)==1&&r16(d+4)==w&&r16(d+6)==h&&p&&n&&n<=0x2000000u&&p<=UINT64_MAX-n;
}
struct native_geometry {uint32_t codec,bytes,area;uint16_t raw_w,raw_h,preview_w,preview_h;};
static const struct native_geometry *geometry(uint32_t codec,uint32_t area,const uint8_t *raw){
 /* Exact original S producer/NEF tuples. Menu submission, actual pipeline
  * output and physical-camera acceptance remain independent qualifications.
  * M+2fc is an opaque guard shared by actual FX and DX, not an area selector.
  */
 static const struct native_geometry rows[]={
  {2,17139200u,0,8280,5520,8256,5504},
  {2,7316992u,1,5408,3608,5392,3592},
#if M3_PROBE_ALLOW_FULL_HE_DX
  /* Original C30 DX HE canvas stays full; original151 crops its preview. */
  {2,17139200u,1,8280,5520,5392,3592},
#endif
  {4,28565504u,0,8280,5520,8256,5504},
  {4,12194816u,1,5408,3608,5392,3592}
 };
 for(size_t i=0;i<sizeof rows/sizeof rows[0];i++){
  const struct native_geometry *g=&rows[i];
  if(codec==g->codec&&area==g->area&&r16(raw+8)==g->raw_w&&r16(raw+10)==g->raw_h&&r32(raw+0x18)==g->bytes)return g;
 }
 return 0;
}
enum m3_stock_job_result m3_stock_native_nef_job(const struct m3_stock_job_api *api,const struct m3_stock_job_inputs *in,struct m3_stock_nef_job *out){
 if(!api||!api->initialize||!api->header_capacity||!in||!out)return M3_JOB_INVALID;
 if(out->state!=M3_JOB_EMPTY)return M3_JOB_BUSY;
 for(size_t i=0;i<sizeof out->native_entry;i++)if(out->native_entry[i])return M3_JOB_BUSY;
 if(!in->capture_R||in->R_bytes<0x7a0||!in->capture_M||in->M_bytes<M3_NATIVE_METADATA_BYTES||!in->presentation_Y||in->Y_bytes<0x178||!in->auxiliary_Y||in->auxiliary_Y_bytes<0x178||!in->header_resource||in->header_resource_bytes<0x10||!in->header_buffer_descriptor||in->header_descriptor_bytes<0x10)return M3_JOB_INVALID;
 if(overlap(out,sizeof *out,in->capture_R,0x7a0)||overlap(out,sizeof *out,in->capture_M,M3_NATIVE_METADATA_BYTES)||overlap(out,sizeof *out,in->presentation_Y,0x178)||overlap(out,sizeof *out,in->auxiliary_Y,0x178)||overlap(out,sizeof *out,in->header_resource,0x10)||overlap(out,sizeof *out,in->header_buffer_descriptor,0x10))return M3_JOB_INVALID;
 const uint8_t *M=in->capture_M,*R=in->capture_R,*Y=in->presentation_Y,*raw=Y+0x58;
 if((in->NEF_file_id&15u)!=2u||r64(R+0x790)!=(uintptr_t)M||r64(R+0x6a8)!=(uintptr_t)in->auxiliary_Y||r32(M+0x2fc)!=0||r32(M+0x1a10)!=3)return M3_JOB_INVALID;
 const uint32_t codec=r32(M+0x2ac);const struct native_geometry *g=geometry(codec,r32(M+0x26c),raw);
 uint64_t address=r64(raw+0x10);
 /* Word +4=5 is retained from the original 14-bit S descriptor. Other
  * branches remain closed until their own producer/consumer ABI is verified.
  */
 if(!g||r32(raw)!=3||r32(raw+4)!=5||!address||(address&511u)||address>UINT64_MAX-g->bytes)return M3_JOB_INVALID;
 if(g->area==1&&g->raw_w==8280&&
    (r16(M+0x280)!=1444||r16(M+0x282)!=964||r16(M+0x284)!=6836||r16(M+0x286)!=4556))return M3_JOB_INVALID;
 if(!jpeg(Y+0xe0,160,120)||!jpeg(Y+0x100,g->preview_w,g->preview_h)||!jpeg(Y+0x120,640,424)||!jpeg(Y+0x140,1620,1080))return M3_JOB_INVALID;
 if(r64((const uint8_t *)in->header_resource+8)!=(uintptr_t)in->header_buffer_descriptor)return M3_JOB_INVALID;
 uint64_t buffer=r64(in->header_buffer_descriptor);uint32_t room=r32(in->header_buffer_descriptor+8),capacity=api->header_capacity(3);
 if(!capacity||!buffer||(buffer&63u)||room<capacity||buffer>UINT64_MAX-room)return M3_JOB_INVALID;
 uint32_t mode=api->initialize(in->NEF_file_id,(void *)R,(void *)Y,in->header_resource,out->native_entry);
 uint8_t *entry=out->native_entry;const uintptr_t expected[]={
  (uintptr_t)(Y+0x58),(uintptr_t)(Y+0x120),(uintptr_t)(Y+0x140),(uintptr_t)(Y+0xe0),(uintptr_t)(Y+0x100),(uintptr_t)(in->auxiliary_Y+0x158),0,0};
 int valid=mode==1&&r32(entry+0x48)==in->NEF_file_id&&r64(entry+0x50)==(uintptr_t)M&&r64(entry+0x58)==(uintptr_t)entry&&r64(entry+0x60)==(uintptr_t)(entry+0x2a0)&&r64(entry+0x2a0)==buffer&&r32(entry+0x2a8)==capacity&&!r32(entry+0x2ac);
 for(unsigned i=0;i<8;i++)valid=valid&&r64(entry+i*8)==expected[i];
 if(!valid){zero(entry,sizeof out->native_entry);return M3_JOB_NATIVE_ERROR;}
 out->codec_class=codec;out->header_capacity=capacity;out->state=M3_JOB_DESCRIPTORS_PREPARED;return M3_JOB_OK;
}
