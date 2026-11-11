/* Z9 5.32 HE menu integration candidate; eight active contexts.
 * Full rapid-OFF/cancel qualification remains separate from normal capture.
 * Original input is never used to claim the patched input is a platform fix.
 */
#include "stock_nef_job.h"
#include "native_burst_reference.h"
#include "native_live_settings.h"
#include "native_runtime_layout.h"
#include "native_codec_geometry.h"
#include "native_auxiliary_gate.h"
#ifndef M3_VIRTUAL_NRMAP
#define M3_VIRTUAL_NRMAP 0
#endif
#ifndef M3_VIRTUAL_NRMAP_VALUE
#define M3_VIRTUAL_NRMAP_VALUE 0
#endif
_Static_assert(M3_VIRTUAL_NRMAP==0||M3_VIRTUAL_NRMAP==1,"Virtual Nrmap switch must be boolean");
_Static_assert(M3_VIRTUAL_NRMAP_VALUE==0||M3_VIRTUAL_NRMAP_VALUE==128,"Unqualified virtual Nrmap value");
#if M3_VIRTUAL_NRMAP
#include "native_virtual_nrmap.h"
#endif
#if defined(M3_AUX_ENTRY_PROBE) && M3_AUX_ENTRY_PROBE
#include "native_auxiliary_entry_probe.h"
#endif
/* Only the owned NEF metadata view changes. Capture/JPEG, native release,
 * packet layout and burst identity remain attached to their original frame. */
#ifndef M3_NEF_S_KIND
#define M3_NEF_S_KIND 0
#endif
_Static_assert(M3_NEF_S_KIND==0||M3_NEF_S_KIND==1,"NEF kind switch must be boolean");
/* Explicit candidate switch: frozen v28 behavior remains available. */
#ifndef M3_SHOTINFO_BATTERY
#define M3_SHOTINFO_BATTERY 0
#endif
#if M3_SHOTINFO_BATTERY
#include "native_shotinfo_battery.h"
#endif
#ifndef M3_SHOTINFO_BANK12
#define M3_SHOTINFO_BANK12 0
#endif
#if M3_SHOTINFO_BANK12
#include "native_shotinfo_bank12.h"
_Static_assert(M3_SHOTINFO_BATTERY==1,"Bank12 candidate requires the existing battery-tail ABI");
#endif
#ifndef M3_SHOTINFO_AXIS25
#define M3_SHOTINFO_AXIS25 0
#endif
#ifndef M3_SHOTINFO_SECTION15
#define M3_SHOTINFO_SECTION15 0
#endif
#if M3_SHOTINFO_SECTION15
#include "native_shotinfo_section15.h"
_Static_assert(M3_SHOTINFO_AXIS25==1,"Section15 candidate requires the owned combined sample");
#endif
#ifndef M3_SHOTINFO_GROUP1820
#define M3_SHOTINFO_GROUP1820 0
#endif
#if M3_SHOTINFO_GROUP1820
#include "native_shotinfo_group1820.h"
_Static_assert(M3_SHOTINFO_SECTION15==1,"Group1820 requires the v32 owned sample prefix");
#endif
#ifndef M3_SHOTINFO_GROUP1723
#define M3_SHOTINFO_GROUP1723 0
#endif
#if M3_SHOTINFO_GROUP1723
#include "native_shotinfo_group1723.h"
_Static_assert(M3_SHOTINFO_GROUP1820==1,"Group1723 requires the v33 owned sample prefix");
#endif
#if M3_SHOTINFO_AXIS25
#include "native_shotinfo_axis25.h"
_Static_assert(M3_SHOTINFO_BANK12==1,"Axis25 candidate requires the owned sample lease");
struct retained_metadata_sample {
 struct m3_shotinfo_bank12_sample bank12;
 struct m3_shotinfo_axis25_sample axis25;
#if M3_SHOTINFO_SECTION15
 struct m3_shotinfo_section15_sample section15;
#if M3_SHOTINFO_GROUP1820
 struct m3_shotinfo_group1820_sample group1820;
#endif
#endif
#if M3_SHOTINFO_GROUP1723
 struct m3_shotinfo_group1723_sample group1723;
#endif
};
_Static_assert(offsetof(struct retained_metadata_sample,axis25)==832,"Bank12 sample prefix moved");
#endif
#ifndef M3_PROBE_RAW_ONLY
#define M3_PROBE_RAW_ONLY 0
#endif
#ifndef M3_PROBE_RAW_BYTES
#define M3_PROBE_RAW_BYTES 17139200u
#endif
#ifndef M3_PROBE_NATIVE_CLASS
#define M3_PROBE_NATIVE_CLASS 2u
#endif
_Static_assert((M3_PROBE_RAW_BYTES==17139200u && M3_PROBE_NATIVE_CLASS==2u) ||
 (M3_PROBE_RAW_BYTES==28565504u && M3_PROBE_NATIVE_CLASS==4u),"Unsupported native HE class/length pair");
#ifndef FN
#define FN(a,t) ((t)(uintptr_t)(a))
#endif
#ifndef M3_PROBE_ALLOW_DX
#define M3_PROBE_ALLOW_DX 0
#endif
_Static_assert(M3_PROBE_ALLOW_DX==0||M3_PROBE_ALLOW_DX==1,"DX opt-in must be boolean");
typedef uint64_t u64;typedef uint32_t u32;typedef uint8_t u8;
/* Packet capacity binds the native class. Selected settings are checked
 * independently at admission, before a job can write its private NEF. */
static u32 raw_bytes_for_packet(u32 dimensions,u32 capacity){
 if(dimensions==((5520u<<16)|8280u))
  return capacity>=M3_HE_STAR_BYTES?M3_HE_STAR_BYTES:M3_HE_BYTES;
 if(M3_PROBE_ALLOW_DX&&dimensions==((3608u<<16)|5408u)){
  if(capacity>M3_HE_STAR_DX_CAPACITY)return 0;
  return capacity>=M3_HE_STAR_DX_BYTES?M3_HE_STAR_DX_BYTES:M3_HE_DX_BYTES;
 }
 return 0;
}
static int crop_for_dimensions(u32 dimensions,u32 bytes,u32 *lo,u32 *hi){
 if(!bytes||bytes!=raw_bytes_for_packet(dimensions,bytes))return 0;
 u32 w=dimensions&65535u,h=dimensions>>16,x=w==8280u?12u:8u;
 *lo=(8u<<16)|x;*hi=((h-8u)<<16)|(w-x);return 1;
}

/* The accepted DX contract uses native small HE. The default-off full HE
 * experiment is retained for reproducibility: Adobe ignored its DX crop.
 * Native area remains independent of compressed canvas dimensions. */
static int crop_for_area(u32 dimensions,u32 bytes,u32 area,u32 *lo,u32 *hi){
 if(area==0)return dimensions==((5520u<<16)|8280u)&&crop_for_dimensions(dimensions,bytes,lo,hi);
 if(area!=1||!M3_PROBE_ALLOW_DX)return 0;
 if(dimensions==((3608u<<16)|5408u))return crop_for_dimensions(dimensions,bytes,lo,hi);
 if(M3_PROBE_ALLOW_FULL_HE_DX&&M3_PROBE_NATIVE_CLASS==2u&&dimensions==((5520u<<16)|8280u)&&bytes==17139200u){
  *lo=(964u<<16)|1444u;*hi=(4556u<<16)|6836u;return 1;
 }
 return 0;
}

static u32 rd32(const void *p){const u8*s=p;return (u32)s[0]|(u32)s[1]<<8|(u32)s[2]<<16|(u32)s[3]<<24;}
static u64 rd64(const void*p){return rd32(p)|(u64)rd32((const u8*)p+4)<<32;}
static int target_metadata(u64 m){
 if(!m||rd32((const void*)(uintptr_t)m)!=3)return 0;
 u32 mode=rd32((const void*)(uintptr_t)(m+0x24a0));
 u32 area=rd32((const void*)(uintptr_t)(m+0x26c));
 return (mode==3u||mode==6u)&&(area==0u||(area==1u&&M3_PROBE_ALLOW_DX));
}
static void wr32(void*p,u32 v){u8*s=p;for(unsigned i=0;i<4;i++)s[i]=(u8)(v>>(8*i));}
static void wr64(void*p,u64 v){wr32(p,(u32)v);wr32((u8*)p+4,(u32)(v>>32));}
static void copy(void*d,const void*s,size_t n){u8*a=d;const u8*b=s;for(size_t i=0;i<n;i++)a[i]=b[i];}
static void zero(void*p,size_t n){u8*s=p;for(size_t i=0;i<n;i++)s[i]=0;}
/* One original 32-byte completion payload; its producer's stack dies before
 * RAW-only JPEG cancellation. Keep the exact bytes in the existing heap
 * owner until the writer/commit/cancel checks join. No frame-table ABI change. */
struct completion_event {u32 phase;u8 payload[32];u64 native_result;};
struct work {u8 R[0x7a0],Y[0x300];struct m3_stock_nef_job job;struct completion_event completion;u8 auxiliary8[0x22200+63];};
struct probe_state {
 u32 magic,version,state,terminal,retained,rejected,prepared,composed;
 u32 register_rc,jpeg_status,nef_status,adapter_rc,jid,nid,counter,sequence;
 u64 R,E,M,data,allocation,node,header,total;
 u32 before_state,after_jpeg_state,after_nef_state,fault,header_capacity,raw_bytes;
 u32 native_cancel,release_count,initialization_class,scope;
 u32 attempts,invalid_reason,last_capacity,last_format,last_bits,last_dimensions;
 u64 last_base,last_data;
 u32 DCF_cancel_calls,DCF_cancel_state,NEF_after_cancel;
 u32 file_commit_attempts,file_commit_rc,file_commit_state_before,file_commit_state_after;
 u64 file_commit_bytes,file_commit_object,file_commit_path;
 u32 file_commit_verified,file_commit_name_length;
 u32 cancel_requested,slot;u64 generation,control;
 u64 private_M;u32 private_M_release_rc,private_M_released;
};
enum {FREE=0,CLAIMING=1,HELD=2,PREPARED=3,WRITING=4,DONE=5,COMPOSING=6,DISPOSING=7,SAMPLING=8};
#define FRAME_SLOTS 8u
#if M3_SHOTINFO_BANK12
struct bank12_lease {u64 sample,generation;};
#endif
struct frame_table {
 u32 magic,version,slots,next_slot,retain_calls,retained,overflow,duplicates;
 u32 compose_calls,composed,write_calls,raw_writes,file_commits,freed,cancel_requests,faults;
 u32 missing_HE_writes,unmatched_free,active,peak,identity_conflicts,private_M_allocated,private_M_freed,reserved[7];
 u64 next_generation;
 struct probe_state frames[FRAME_SLOTS];
#if M3_SHOTINFO_BATTERY
 /* Version6's existing128B header and312B records stay at the same offsets.
  * The new sample tail occupies formerly unused space BEFORE live settings. */
 struct m3_shotinfo_battery_sample battery[FRAME_SLOTS];
#endif
#if M3_SHOTINFO_BANK12
 struct bank12_lease bank12[FRAME_SLOTS];
#endif
};
#define TABLE ((volatile struct frame_table*)(uintptr_t)M3_FRAME_STATE_ADDRESS)
#define P p
_Static_assert(sizeof(struct frame_table)<=M3_FRAME_STATE_LIMIT,"Frame state overlaps live settings");
_Static_assert(offsetof(struct frame_table,frames)==128&&sizeof(struct probe_state)==312,"Existing native observer ABI changed");
#if M3_SHOTINFO_BANK12
_Static_assert(offsetof(struct frame_table,battery)==2624&&offsetof(struct frame_table,bank12)==2944&&sizeof(struct frame_table)==3072,"Bank12 leases overlap live settings");
#elif M3_SHOTINFO_BATTERY
_Static_assert(offsetof(struct frame_table,battery)==2624&&sizeof(struct frame_table)==2944,"Battery samples exceed owned frame-state tail");
#endif
static void add(volatile u32*p){__atomic_fetch_add(p,1,__ATOMIC_RELAXED);}
#define CANCEL_FLAG 0x100ull
static u64 control(volatile struct probe_state*p){return __atomic_load_n(&P->control,__ATOMIC_ACQUIRE);}
static u32 get_state(volatile struct probe_state*p){return (u32)(control(p)&255);}
static int cancelled(volatile struct probe_state*p){return !!(control(p)&CANCEL_FLAG);}
static void publish(volatile struct probe_state*p,u32 s){
 __atomic_store_n(&P->state,s,__ATOMIC_RELAXED);
 u64 old=control(p),next;
 do {next=(old&~255ull)|s;}while(!__atomic_compare_exchange_n(&P->control,&old,next,0,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE));
}
static int claim(volatile struct probe_state*p,u32 from,u32 to){
 u64 old=control(p);if((old&255)!=from)return 0;
 if(!__atomic_compare_exchange_n(&P->control,&old,(old&~255ull)|to,0,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE))return 0;
 __atomic_store_n(&P->state,to,__ATOMIC_RELEASE);return 1;
}
static int active(u32 s){return s!=FREE&&s!=DONE;}
#if M3_SHOTINFO_BANK12
/* Counters use the existing header's seven reserved words. Neither frame
 * records nor the live settings/audio bootstrap area move. */
enum {BANK12_ALLOCATED,BANK12_FREED,BANK12_CAPTURED,BANK12_PUBLISHED,
      BANK12_UNAVAILABLE,BANK12_ALLOC_FAILED,BANK12_REJECTED};
static void bank12_lock(void *mutex,void *opaque){
 (void)opaque;(void)FN(0x40d306c4,int(*)(void*))(mutex);
}
static void bank12_unlock(void *mutex,void *opaque){
 (void)opaque;FN(0x40d306b0,void(*)(void*))(mutex);
}
static void bank12_capture(volatile struct probe_state*p){
 const u8*m=(const void*)(uintptr_t)P->M;
#if M3_SHOTINFO_AXIS25
 int need12=!rd32(m+0x2980)&&rd64(m+0x2988);
 int need25=!rd32(m+0x2a50)&&rd64(m+0x2a58);
#if M3_SHOTINFO_SECTION15
 int need15=!rd32(m+0x29b0)&&rd64(m+0x29b8);
#if M3_SHOTINFO_GROUP1820
 int needgroup=(!rd32(m+0x29e0)&&rd64(m+0x29e8))||
   (!rd32(m+0x29f0)&&rd64(m+0x29f8))||(!rd32(m+0x2a00)&&rd64(m+0x2a08));
#if M3_SHOTINFO_GROUP1723
 int need1723=(!rd32(m+0x29d0)&&rd64(m+0x29d8))||(!rd32(m+0x2a30)&&rd64(m+0x2a38));
 if(!need12&&!need25&&!need15&&!needgroup&&!need1723)return;
#else
 if(!need12&&!need25&&!need15&&!needgroup)return;
#endif
#else
 if(!need12&&!need25&&!need15)return;
#endif
#else
 if(!need12&&!need25)return;
#endif
 struct retained_metadata_sample*combined=FN(0x40d30458,void*(*)(u32,u32))(sizeof *combined,0);
 struct m3_shotinfo_bank12_sample*s=combined?&combined->bank12:0;
#else
 if(rd32(m+0x2980)||!rd64(m+0x2988))return;
 struct m3_shotinfo_bank12_sample*s=FN(0x40d30458,void*(*)(u32,u32))(sizeof *s,0);
#endif
 if(!s){add(&TABLE->reserved[BANK12_ALLOC_FAILED]);return;}
#if M3_SHOTINFO_AXIS25
 zero(combined,sizeof *combined);
#else
 zero(s,sizeof *s);
#endif
 TABLE->bank12[P->slot].sample=(uintptr_t)s;
 TABLE->bank12[P->slot].generation=P->generation;
 add(&TABLE->reserved[BANK12_ALLOCATED]);
 const struct m3_bank_mutex_ops mutex={bank12_lock,bank12_unlock,0};
 if(m3_shotinfo_bank12_capture(s,P->generation,m,M3_NATIVE_METADATA_BYTES,
    (const volatile void*)(uintptr_t)0xa04a8cc60ull,&mutex))
  add(&TABLE->reserved[BANK12_CAPTURED]);
 else add(&TABLE->reserved[BANK12_UNAVAILABLE]);
#if M3_SHOTINFO_AXIS25
 /* Same whole read-only serializer used by original S. No ring update,
  * state consumption, donor photo data or fabricated axis coefficients. */
 if(need25)(void)m3_shotinfo_axis25_capture(&combined->axis25,P->generation,
    m,M3_NATIVE_METADATA_BYTES,FN(0x40c8c7a0,m3_shotinfo_axis25_producer));
#if M3_SHOTINFO_SECTION15
 if(need15)(void)m3_shotinfo_section15_capture(&combined->section15,P->generation,
    m,M3_NATIVE_METADATA_BYTES,FN(0x40f6de38,m3_shotinfo_section15_producer));
#if M3_SHOTINFO_GROUP1820
 /* One whole original builder for three slots. Only its owned shadow M
  * descriptors are redirected; native service mirror writes are retained. */
 if(needgroup)(void)m3_shotinfo_group1820_capture(&combined->group1820,P->generation,
    m,M3_NATIVE_METADATA_BYTES,FN(0x40f537a0,m3_shotinfo_group1820_producer));
#if M3_SHOTINFO_GROUP1723
 if(need1723)(void)m3_shotinfo_group1723_capture(&combined->group1723,P->generation,
    m,M3_NATIVE_METADATA_BYTES,m3_shotinfo_group1723_native_ready,m3_shotinfo_group1723_native_build);
#endif
#endif
#endif
#endif
}
static void bank12_publish(volatile struct probe_state*p,void*private_M){
 volatile struct bank12_lease*l=&TABLE->bank12[P->slot];
 if(!l->sample)return;
 if(l->generation!=P->generation){add(&TABLE->reserved[BANK12_REJECTED]);return;}
 struct m3_shotinfo_bank12_sample*s=(void*)(uintptr_t)l->sample;
#if M3_SHOTINFO_AXIS25
 struct retained_metadata_sample*combined=(void*)s;
 if(m3_shotinfo_axis25_publish(&combined->axis25,P->generation,
    (const void*)(uintptr_t)P->M,M3_NATIVE_METADATA_BYTES,
    private_M,M3_NATIVE_METADATA_BYTES))combined->axis25.stage=2;
#if M3_SHOTINFO_SECTION15
 if(m3_shotinfo_section15_publish(&combined->section15,P->generation,
    (const void*)(uintptr_t)P->M,M3_NATIVE_METADATA_BYTES,
    private_M,M3_NATIVE_METADATA_BYTES))combined->section15.stage=2;
#if M3_SHOTINFO_GROUP1820
 if(m3_shotinfo_group1820_publish(&combined->group1820,P->generation,
    (const void*)(uintptr_t)P->M,M3_NATIVE_METADATA_BYTES,
    private_M,M3_NATIVE_METADATA_BYTES))combined->group1820.stage=2;
#if M3_SHOTINFO_GROUP1723
 if(m3_shotinfo_group1723_publish(&combined->group1723,P->generation,
    (const void*)(uintptr_t)P->M,M3_NATIVE_METADATA_BYTES,
    private_M,M3_NATIVE_METADATA_BYTES))combined->group1723.stage=2;
#endif
#endif
#endif
#endif
 if(s->stage!=M3_BANK12_RETAIN_STAGE)return;
 if(m3_shotinfo_bank12_publish(s,P->generation,(const void*)(uintptr_t)P->M,
    M3_NATIVE_METADATA_BYTES,private_M,M3_NATIVE_METADATA_BYTES)){
  s->stage=2;add(&TABLE->reserved[BANK12_PUBLISHED]);
 }else add(&TABLE->reserved[BANK12_REJECTED]);
}
#endif
static void initialize_table(void){TABLE->magic=0x334d395au;TABLE->version=6;TABLE->slots=FRAME_SLOTS;}
static int initialize_frame(volatile struct probe_state*p,u32 slot){
 /* Keep CLAIMING published until every identity field has been initialized. */
 volatile u8*b=(volatile u8*)p;for(unsigned i=0;i<sizeof *p;i++)if((i<8||i>=12)&&(i<offsetof(struct probe_state,control)||i>=offsetof(struct probe_state,control)+sizeof P->control))b[i]=0;
 P->slot=slot;P->generation=__atomic_add_fetch(&TABLE->next_generation,1,__ATOMIC_RELAXED);
 if(P->generation>0xffffffffull){publish(p,DONE);return 0;}
 __atomic_store_n(&P->control,(P->generation<<32)|CLAIMING,__ATOMIC_RELEASE);
 return 1;
}
static u32 dcf_state(u32 id){
 u64 o=FN(0x4085d7a0,u64(*)(u32))(id);if(!o)return 0xffffffffu;
 u32 s=rd32((void*)(uintptr_t)(o+0x70));FN(0x4085d93c,void(*)(u64))(o);return s;
}
static u32 dispose(volatile struct probe_state*p,u32 terminal){
 /* Never make a slot reusable until native free and private cleanup return.
  * DISPOSING also prevents a duplicate native cancellation from double-free. */
 u64 e=P->E,r=P->R,h=P->header,node=P->node;P->terminal=terminal;publish(p,DISPOSING);
 P->cancel_requested=cancelled(p);P->native_cancel=P->cancel_requested;
 if(P->cancel_requested&&!P->fault)P->fault=12;
 P->header=0;P->node=0;
 if(h)FN(0x41052f20,void(*)(u64))(h);
 if(P->private_M&&!P->private_M_released){
  P->private_M_release_rc=FN(0x4104d084,u32(*)(u64))(P->private_M);
  if(!(P->private_M_release_rc&1)){
   /* Failed owned-M release is not a reusable DONE slot. Preserve its work
    * and retained E for diagnosis rather than losing the only ownership. */
   P->node=node;P->fault=13;add(&TABLE->faults);return 13;
  }
  P->private_M_released=1;add(&TABLE->private_M_freed);
 }
#if M3_SHOTINFO_BANK12
 volatile struct bank12_lease*l=&TABLE->bank12[P->slot];
 if(l->sample){
  if(l->generation!=P->generation){P->node=node;P->fault=17;add(&TABLE->faults);return 17;}
  u64 sample=l->sample;l->sample=0;l->generation=0;
  FN(0x40d30498,void(*)(void*))((void*)(uintptr_t)sample);
  add(&TABLE->reserved[BANK12_FREED]);
 }
#endif
 if(node)FN(0x40d30498,void(*)(void*))((void*)(uintptr_t)node);
 if(e){if(!P->cancel_requested&&r&&rd64((void*)(uintptr_t)(r+0x748))==e)wr64((void*)(uintptr_t)(r+0x748),0);P->release_count++;FN(0x4104d0c0,void(*)(u64))(e);add(&TABLE->freed);}
 if(P->fault)add(&TABLE->faults);
 P->cancel_requested=cancelled(p);P->native_cancel=P->cancel_requested;
 u32 fault=P->fault;
 __atomic_fetch_sub(&TABLE->active,1,__ATOMIC_RELAXED);m3_live_retire();publish(p,DONE);return fault;
}
/* Called at the original Tdec release boundary, after the original producer
 * has finished. No file/codec completion is fabricated by this adapter. */
u32 m3_probe_retain(u64 r){
 if(!r)return 0;
#if defined(M3_AUX_ENTRY_PROBE) && M3_AUX_ENTRY_PROBE
 m3_aux_entry_retain(r);
#endif
 initialize_table();add(&TABLE->retain_calls);
 u64 m=rd64((void*)(uintptr_t)(r+0x790)),e=rd64((void*)(uintptr_t)(r+0x748));
 if(!e||!target_metadata(m))return 0;
 u32 counter=rd32((void*)(uintptr_t)(m+0x250c)),seq=rd32((void*)(uintptr_t)(m+0x120));
 for(unsigned i=0;i<FRAME_SLOTS;i++){
  volatile struct probe_state*p=&TABLE->frames[i];if(!active(get_state(p)))continue;
  if(P->E==e&&P->R==r&&P->M==m&&P->counter==counter&&P->sequence==seq){add(&TABLE->duplicates);return 1;}
  if(P->E==e||P->R==r){add(&TABLE->identity_conflicts);__atomic_fetch_or(&P->control,CANCEL_FLAG,__ATOMIC_ACQ_REL);return P->E==e;}
 }
 u64 a=rd64((void*)(uintptr_t)(e+8));if(!a)return 0;
 u32 n=rd32((void*)(uintptr_t)(a+8));u64 base=rd64((void*)(uintptr_t)a),data=rd64((void*)(uintptr_t)(e+0x40));const u8*d=(void*)(uintptr_t)(e+0x30);
 u32 raw_bytes=raw_bytes_for_packet(rd32(d+8),n);
 if(!raw_bytes||n<raw_bytes||n>0x2000000u||rd32(d)!=3||rd32(d+4)!=5||!base||data<base||data-base>n||raw_bytes>n-(data-base))return 0;
 u32 crop_lo,crop_hi;
 if(!crop_for_area(rd32(d+8),raw_bytes,rd32((const void*)(uintptr_t)(m+0x26c)),&crop_lo,&crop_hi))return 0;
 struct m3_hsfc_settings selected;
 if(!m3_live_admit(&selected))return 0;
 if((selected.codec!=3&&selected.codec!=4)||
    (selected.codec==3?!m3_codec_star_bytes(raw_bytes):m3_codec_star_bytes(raw_bytes))||
    (selected.option!=0&&selected.option!=2&&selected.option!=4)){
  m3_live_retire();return 0;
 }
 u32 first=__atomic_fetch_add(&TABLE->next_slot,1,__ATOMIC_RELAXED)%FRAME_SLOTS;
 for(unsigned j=0;j<FRAME_SLOTS;j++){
  unsigned i=(first+j)%FRAME_SLOTS;volatile struct probe_state*p=&TABLE->frames[i];u32 st=get_state(p);
#if M3_SHOTINFO_BANK12
  /* An unexpected old lease is never overwritten or reused. */
  if(TABLE->bank12[i].sample||TABLE->bank12[i].generation)continue;
#endif
  if((st!=FREE&&st!=DONE)||!claim(p,st,CLAIMING))continue;
  if(!initialize_frame(p,i)){add(&TABLE->overflow);m3_live_retire();return 0;}
  P->magic=0x334d395au;P->version=6;P->scope=selected.option==0?2:1;P->R=r;P->E=e;P->M=m;P->data=data;P->allocation=n;
  P->counter=counter;P->sequence=seq;P->retained=1;P->attempts=1;P->raw_bytes=raw_bytes;
  P->last_capacity=n;P->last_format=rd32(d);P->last_bits=rd32(d+4);P->last_dimensions=rd32(d+8);P->last_base=base;P->last_data=data;
#if M3_SHOTINFO_BANK12
  /* All blocking native calls below have an initialized, counted owner.
   * Native E cancellation marks SAMPLING and lets this owner join; compose
   * cannot claim the frame until its retained sample is fully published.
   * The pre-existing native R/M rapid-cancel contract remains separate. */
  add(&TABLE->retained);u32 count=__atomic_add_fetch(&TABLE->active,1,__ATOMIC_RELAXED),peak=__atomic_load_n(&TABLE->peak,__ATOMIC_RELAXED);
  while(count>peak&&!__atomic_compare_exchange_n(&TABLE->peak,&peak,count,0,__ATOMIC_RELAXED,__ATOMIC_RELAXED)){}
  publish(p,SAMPLING);
#endif
#if M3_SHOTINFO_BATTERY
  /* Original40EBE56C is a whole8B battery serializer, not an image worker.
   * Sample at this retained frame boundary. No native M or JPEG data changes;
   * a later global state can never change this private per-generation sample. */
  struct m3_shotinfo_battery_sample *battery=(void*)(uintptr_t)&TABLE->battery[i];
  zero(battery,sizeof *battery);
  if(!rd32((const void*)(uintptr_t)(m+0x2a60))&&rd64((const void*)(uintptr_t)(m+0x2a68)))
   (void)m3_shotinfo_battery_capture(battery,P->generation,(const void*)(uintptr_t)m,M3_NATIVE_METADATA_BYTES,FN(0x40ebe56c,m3_shotinfo_battery_producer));
#endif
#if M3_SHOTINFO_BANK12
  bank12_capture(p);
  u64 observed=control(p);
  for(;;){
   u32 state=(observed&CANCEL_FLAG)?DISPOSING:HELD;
   u64 next=(observed&~255ull)|state;
   __atomic_store_n(&P->state,state,__ATOMIC_RELAXED);
   if(!__atomic_compare_exchange_n(&P->control,&observed,next,0,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE))continue;
   if(state==DISPOSING)(void)dispose(p,32);
   /* Even a cancelled owner already performed E cleanup. Returning0 would
    * make hook_retain invoke the original E-release a second time. */
   return 1;
  }
#else
  add(&TABLE->retained);u32 count=__atomic_add_fetch(&TABLE->active,1,__ATOMIC_RELAXED),peak=__atomic_load_n(&TABLE->peak,__ATOMIC_RELAXED);
  while(count>peak&&!__atomic_compare_exchange_n(&TABLE->peak,&peak,count,0,__ATOMIC_RELAXED,__ATOMIC_RELAXED)){}
  publish(p,HELD);return 1;
#endif
 }
 add(&TABLE->overflow);m3_live_retire();return 0;
}
static int same_frame(volatile struct probe_state*p,u64 r){return r==P->R&&rd64((void*)(uintptr_t)(r+0x748))==P->E&&rd64((void*)(uintptr_t)(r+0x790))==P->M&&rd32((void*)(uintptr_t)(P->M+0x250c))==P->counter&&rd32((void*)(uintptr_t)(P->M+0x120))==P->sequence;}
/* Invoked after the original JPEG composer returns, in its own construction
 * task. Its same-frame Flash subobject has now been initialized. */
void m3_probe_compose(u64 task,u64 r,u64 j,u32 original_mode){
 if(original_mode!=0||!r)return;
 add(&TABLE->compose_calls);
 volatile struct probe_state*p=0;
 for(unsigned i=0;i<FRAME_SLOTS;i++){volatile struct probe_state*q=&TABLE->frames[i];if(get_state(q)==HELD&&same_frame(q,r)&&claim(q,HELD,COMPOSING)){p=q;break;}}
 if(!p)return;
 P->jid=rd32((void*)(uintptr_t)j);P->nid=(P->jid&~15u)|2u;
 /* Native file-ID bit29 selects card2. Keep it in both member IDs;
  * only unsupported slot bits30..31 are a reason to reject this frame.
  * Shifting by30 also retains the original zero scratch value on admission. */
 if((P->jid&15u)!=0||P->jid>>30){P->fault=1;dispose(p,11);return;}
 struct work*w=FN(0x40d30458,void*(*)(u32,u32))(sizeof *w,0);if(!w){P->fault=2;dispose(p,12);return;}
 zero(w,sizeof *w);P->node=(u64)(uintptr_t)w;copy(w->R,(void*)(uintptr_t)r,sizeof w->R);
 P->private_M=FN(0x4104cc68,u64(*)(u32,u32))(1,1);
 if(!P->private_M){P->fault=14;dispose(p,22);return;}
 add(&TABLE->private_M_allocated);
 /* The original allocator owns all0x4480 metadata bytes and its initialized
  * auxiliary buffers. Original copy preserves destination buffer identities
  * and copies the source payloads. A prefix memcpy is not a full M clone. */
 FN(0x4104cec4,void(*)(u64,u64))(P->M,P->private_M);
 u8*private_M=(void*)(uintptr_t)P->private_M;
#if M3_SHOTINFO_BATTERY
 /* Add only the captured native-produced slot26 to this owned NEF copy.
  * Invalid/missing source or unexpected allocator layout leaves it absent;
  * native sections stay authoritative. Other missing sections remain open. */
 struct m3_shotinfo_battery_sample *battery=(void*)(uintptr_t)&TABLE->battery[P->slot];
 if(m3_shotinfo_battery_publish(battery,P->generation,(const void*)(uintptr_t)P->M,M3_NATIVE_METADATA_BYTES,private_M,M3_NATIVE_METADATA_BYTES))battery->stage=2;
#endif
#if M3_SHOTINFO_BANK12
 bank12_publish(p,private_M);
#endif
 /* RAW-only removes the original JPEG member after verified NEF commit.
  * Its selected first-image reference must consequently name the NEF.
  * Change only the private same-frame view; native DCF/index consumers and
  * actual camera series playback remain separate qualification gates. */
 if(P->scope==2&&m3_native_raw_burst_reference(private_M,M3_NATIVE_METADATA_BYTES,P->jid,P->nid)!=M3_BURST_REFERENCE_OK){
  P->fault=15;dispose(p,23);return;
 }
 u64 y=rd64((void*)(uintptr_t)(r+0x6a8));if(!y){P->fault=3;dispose(p,13);return;}copy(w->Y,(void*)(uintptr_t)y,sizeof w->Y);
 /* Only a private same-frame NEF view changes. The original JPEG metadata
  * remains byte-identical. The configured native class is bound to its packet.
  * No Z8 field offsets or invented positive sensor values are imported. */
 P->initialization_class=rd32(private_M+0x2ac);wr32(private_M+0x2ac,m3_codec_star_bytes(P->raw_bytes)?4:2);wr64(w->R+0x790,P->private_M);
#if M3_NEF_S_KIND
 /* Original406E62E4 persists kind0 as ShutterMode10h, whose original
  * reader406DB34C restores ordinary RAW preparation. Do not change the
  * original M or its C15/C30 release/sequence fields. Existing files are
  * unaffected. Decoder/NR/ISP numerical acceptance remains separate. */
 wr32(private_M,0);
#endif
 copy(w->Y+0x58,(void*)(uintptr_t)(P->E+0x30),0x20);wr32(w->Y+0x58+0x18,P->raw_bytes);
 copy(w->Y+0x100,(void*)(uintptr_t)(y+0x38),0x18);copy(w->Y+0x120,(void*)(uintptr_t)(y+0xa0),0x18);copy(w->Y+0x140,(void*)(uintptr_t)(y+0xc0),0x18);
 /* Original completion flag and exact native descriptor are required before
  * borrowing a byte. The private frame owns the full stride/crop canvas until
  * NEF write/commit/cancellation join. Incomplete auxiliaries remain absent. */
 uintptr_t auxiliary=((uintptr_t)w->auxiliary8+63u)&~(uintptr_t)63u;
 if(!m3_auxiliary8_copy((const void*)(uintptr_t)y,0x300,rd32(private_M+0x26c),w->Y+0x158,(void*)auxiliary,0x22200)){
#if M3_VIRTUAL_NRMAP
  /* A completed but malformed native map is a failure, not permission to
   * hide a bad buffer. The synthetic producer replaces only an uncompleted
   * private view and publishes fully initialized owned pixels, never a DSP
   * completion claim. Native R/M/Y and JPEG stay untouched. */
  if(__atomic_load_n((const u8*)(uintptr_t)(y+0x21),__ATOMIC_ACQUIRE)||
     !m3_virtual_nrmap_fill(w->Y,sizeof w->Y,rd32(private_M+0x26c),
       (void*)auxiliary,0x22200,M3_VIRTUAL_NRMAP_VALUE)){
   P->fault=17;dispose(p,24);return;
  }
#else
  zero(w->Y+0x158,0x20);
#endif
 }
 wr64(w->R+0x6a8,(uintptr_t)w->Y);
 /* Exact original S FX/DX RAW crop corners, derived only for the private
  * NEF view of an otherwise zero native JPEG CropArea. The actual retained
  * E descriptor selects the row; a later global menu change cannot resize it.
  * This does not qualify physical Cxx DX spatial sampling or cancel lifetime.
  */
 u32 crop_lo,crop_hi;
 if(!crop_for_area(P->last_dimensions,P->raw_bytes,rd32(private_M+0x26c),&crop_lo,&crop_hi)){
  P->fault=8;dispose(p,18);return;
 }
 if(!rd64(private_M+0x280)&&!rd32(private_M+0x29c)){
  wr32(private_M+0x280,crop_lo);wr32(private_M+0x284,crop_hi);
 }else if(rd32(private_M+0x280)!=crop_lo||rd32(private_M+0x284)!=crop_hi){
  P->fault=8;dispose(p,18);return;
 }
 /* Preview source reuse is limited to this diagnostic. Final fine/normal
  * NEFs still require the original NEF preview encodes and regressions. */
 if(!FN(0x41052f74,u32(*)(u32,u32))(0x2f,0)){P->fault=4;dispose(p,14);return;}
 u64 h=FN(0x41052bfc,u64(*)(u32,u32,u32))(rd32((void*)(uintptr_t)task),0x2f,0);if(!h){P->fault=5;dispose(p,15);return;}P->header=h;
 struct m3_stock_job_api api={FN(0x40eb58c8,u32(*)(u32,void*,void*,void*,void*)),FN(0x406c8d94,u32(*)(u32))};
 struct m3_stock_job_inputs in={P->nid,w->R,sizeof w->R,private_M,M3_NATIVE_METADATA_BYTES,w->Y,sizeof w->Y,w->Y,sizeof w->Y,(void*)(uintptr_t)h,0x10,(void*)(uintptr_t)rd64((void*)(uintptr_t)(h+8)),0x10};
 P->adapter_rc=m3_stock_native_nef_job(&api,&in,&w->job);if(P->adapter_rc){P->fault=6;dispose(p,16);return;}P->prepared++;
 /* The original initializer always supplies Y+158, even for JPEG-only
  * sources. All-zero original C15 auxiliary bytes mean absent, not a zero
  * sized image at a nonzero strip offset. Original 406CA050 tests array[5]
  * for directory kind 7. Normalize only this proven absent descriptor. */
 u32 auxiliary_nonzero=0;for(unsigned i=0;i<0x20;i++)auxiliary_nonzero|=w->Y[0x158+i];
 if(!auxiliary_nonzero)wr64(w->job.native_entry+0x28,0);
 FN(0x406c8d98,void(*)(u32,u32,void*))(0,1,w->job.native_entry+0x48);
 P->total=rd64(w->job.native_entry+0x48+0x30);P->header_capacity=w->job.header_capacity;
 if(P->total<P->raw_bytes||P->total>P->raw_bytes+0x2000000ull){P->fault=7;dispose(p,17);return;}P->composed++;add(&TABLE->composed);publish(p,PREPARED);if(cancelled(p)&&claim(p,PREPARED,DISPOSING))dispose(p,31);
}
/* Original native close marks descriptor bit6; it can leave directory and
 * allocation entries pending. Original path-info opcode0F first drains that
 * native close cache, then queries size. Use the original DCF-serialized
 * FileInfo workspace, rather than inventing a private output-structure size.
 * NAME acquires this lock itself, so obtain it before the explicit lock;
 * GET/PUT are likewise outside the workspace critical section.
 * This remains a bounded diagnostic; full native cancellation ownership is not
 * qualified merely by adding a native size query. */
static u32 verify_nef_file_commit(volatile struct probe_state*p){
 P->file_commit_attempts++;P->file_commit_rc=0xfffffffcu;P->file_commit_verified=0;
 u64 o=FN(0x4085d7a0,u64(*)(u32))(P->nid);P->file_commit_object=o;if(!o)return 0;
 u64 name=FN(0x4085eea0,u64(*)(u64))(o);u32 ok=0;
 if(name==o+0x74){
  FN(0x40860208,void(*)(void))();
  P->file_commit_state_before=rd32((void*)(uintptr_t)(o+0x70));
  u32 n=rd32((void*)(uintptr_t)(name+0x20));P->file_commit_name_length=n;P->file_commit_path=name;
  const u8*path=(const u8*)(uintptr_t)name;
  /* Original408714E0 is an identity C-string getter for this native path.
   * Its32-byte inline array and length+20 are original consumers, not Z8 ABI. */
  if(get_state(p)==WRITING&&rd32((void*)(uintptr_t)(o+0x28))==P->nid&&P->file_commit_state_before==1&&n>=7&&n<32&&path[n]==0&&
     (path[0]=='A'||path[0]=='B')&&path[1]==':'&&path[2]=='\\'&&path[n-4]=='.'&&
     (path[n-3]|32)=='n'&&(path[n-2]|32)=='e'&&(path[n-1]|32)=='f'){
   P->file_commit_rc=FN(0x402da25c,u32(*)(const void*,void*))((const void*)(uintptr_t)name,(void*)(uintptr_t)0xa04acf2dcull);
   if(!P->file_commit_rc)P->file_commit_bytes=rd64((void*)(uintptr_t)(0xa04acf2dcull+0x218));
   P->file_commit_state_after=rd32((void*)(uintptr_t)(o+0x70));
   ok=!P->file_commit_rc&&P->file_commit_bytes==P->total&&P->file_commit_state_after==1&&
      rd32((void*)(uintptr_t)(o+0x28))==P->nid&&get_state(p)==WRITING;
  }
  FN(0x40860224,void(*)(void))();
 }
 FN(0x4085d93c,void(*)(u64))(o);P->file_commit_verified=ok;return ok;
}
/* Replaces only BL406BA6D4 at original photo-completion site406CA4F8.
 * That caller ignores the callback result at406CA4FC. All unowned events,
 * ordinary S and RAW+JPEG call the unchanged original dispatcher directly.
 * The native writer/close, event bytes and native index guards stay intact. */
u64 m3_probe_photo_completion(u32 event,const void*payload){
 if(event==0x041b0002u&&payload&&TABLE->magic==0x334d395au&&TABLE->version==6){
  u32 fid=rd32(payload);
  for(unsigned i=0;i<FRAME_SLOTS;i++){
   volatile struct probe_state*p=&TABLE->frames[i];
   if(get_state(p)!=WRITING||P->scope!=2||P->nid!=fid||!P->node)continue;
   struct work*w=(void*)(uintptr_t)P->node;u32 empty=0;
   if(!__atomic_compare_exchange_n(&w->completion.phase,&empty,1,0,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE)){
    P->fault=16;return 0; /* Duplicate owned notification is RAW failure. */
   }
   copy(w->completion.payload,payload,32);
   __atomic_store_n(&w->completion.phase,2,__ATOMIC_RELEASE);
   return 0;
  }
 }
 return FN(0x406ba6d4,u64(*)(u32,const void*))(event,payload);
}
static int publish_raw_completion(volatile struct probe_state*p,struct work*w){
 u32 ready=2;
 if(P->fault||cancelled(p)||get_state(p)!=WRITING||
    !__atomic_compare_exchange_n(&w->completion.phase,&ready,3,0,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE))return 0;
 /* Original synchronous callback consumes our owned copy before returning.
  * Its opaque result is retained, never interpreted as index/UI acceptance. */
 w->completion.native_result=FN(0x406ba6d4,u64(*)(u32,const void*))(0x041b0002u,w->completion.payload);
 __atomic_store_n(&w->completion.phase,4,__ATOMIC_RELEASE);
 return !P->fault&&!cancelled(p);
}
u32 m3_probe_write(u32 slot,u32 mode,u64 j,u64 r){
#if defined(M3_AUX_ENTRY_PROBE) && M3_AUX_ENTRY_PROBE
 m3_aux_entry_write(r,j);
#endif
 add(&TABLE->write_calls);volatile struct probe_state*p=0;
 for(unsigned i=0;i<FRAME_SLOTS;i++){volatile struct probe_state*q=&TABLE->frames[i];if(get_state(q)==PREPARED&&rd32((void*)(uintptr_t)j)==q->jid&&same_frame(q,r)&&claim(q,PREPARED,WRITING)){p=q;break;}}
 if(!p){
  u32 missing=0;
  if(mode==0&&r){u64 m=rd64((void*)(uintptr_t)(r+0x790));if(target_metadata(m)){struct m3_hsfc_settings selected;if(!m3_live_read(0,&selected)||selected.option<8){add(&TABLE->missing_HE_writes);missing=1;}}}
  u32 status=FN(0x406c8d9c,u32(*)(u32,u32,u64))(slot,mode,j);
  /* The original caller40EA773C stores the exact status and40EA7740 tests
   * zero/nonzero. A salvaged JPEG is not a successful requested RAW file. */
  return missing&&!status?1u:status;
 }
 publish(p,WRITING);P->before_state=dcf_state(P->jid);
 /* Z9's original creation task uses this lock/global and success space.
  * It differs from the Z8 reference. Reservation failures are recorded;
  * this probe leaves the original JPEG path usable for diagnosis only. */
 u32 ent[4]={2,0,(u32)P->total,(u32)(P->total>>32)},ok=0x04350080u;
 if(FN(0x408786c4,u32(*)(u64,u32,u32))(0xa04ad1f30ull,0xc8,10)==ok){
  /* Original408697D4..40869834 selects this two-pointer array using the
   * same native file-ID slot bits before4086318C. +2038 is card1 and+2040
   * is card2, not one shared active-card database. Never reserve a card2
   * member against card1's directory/capacity state. The global lock still
   * owns the pointer lookup and native member registration as before. */
  u32 card=(P->jid>>29)&3u;
  u64 db=card<2?rd64((void*)(uintptr_t)(0xa04ad2038ull+8ull*card)):0;
  P->register_rc=db?FN(0x4086318c,u32(*)(u64,u32,const void*))(db,P->jid,ent):0xffffffffu;
  if(P->register_rc==ok)FN(0x40867768,void(*)(u32))(1);
  FN(0x40878744,u32(*)(u64))(0xa04ad1f30ull);
 }else P->register_rc=0xfffffffeu;
 u32 st=0;
 if(P->scope==2&&P->register_rc==ok){
  struct work*w=(void*)(uintptr_t)P->node;P->jpeg_status=0xfffffffdu;
  P->nef_status=FN(0x406c8d9c,u32(*)(u32,u32,void*))(0,1,w->job.native_entry+0x48);P->after_nef_state=dcf_state(P->nid);
  u32 committed=!P->nef_status&&P->after_nef_state==1&&verify_nef_file_commit(p);
  if(committed){
   /* Original Z9 4085DB70..90 admits reservation states {2,3,8}
    * through mask 0x10C; completed state 1 was directly observed in v4.
    * Cancel only after the NEF writer completed, then verify both members. */
   FN(0x4085db14,void(*)(u32))(P->jid);P->DCF_cancel_calls++;
   P->DCF_cancel_state=dcf_state(P->jid);P->NEF_after_cancel=dcf_state(P->nid);P->after_jpeg_state=P->DCF_cancel_state;
  }
  if(P->nef_status||P->after_nef_state!=1||!committed||!P->DCF_cancel_calls||P->DCF_cancel_state==2||P->DCF_cancel_state==3||P->DCF_cancel_state==8||P->NEF_after_cancel!=1){
   /* Diagnostic salvage is explicitly RAW failure, never RAW-only success. */
   P->fault=committed?9:11;st=FN(0x406c8d9c,u32(*)(u32,u32,u64))(slot,mode,j);P->jpeg_status=st;
  }else if(!publish_raw_completion(p,w))P->fault=16;
 }else{
  st=FN(0x406c8d9c,u32(*)(u32,u32,u64))(slot,mode,j);P->jpeg_status=st;P->after_jpeg_state=dcf_state(P->jid);
  if(P->register_rc==ok){struct work*w=(void*)(uintptr_t)P->node;P->nef_status=FN(0x406c8d9c,u32(*)(u32,u32,void*))(0,1,w->job.native_entry+0x48);P->after_nef_state=dcf_state(P->nid);if(P->nef_status||P->after_nef_state!=1||!verify_nef_file_commit(p))P->fault=11;}
  else {P->nef_status=0xffffffffu;if(P->scope==2)P->fault=10;}
 }
 if(!P->nef_status)add(&TABLE->raw_writes);
 if(P->file_commit_verified)add(&TABLE->file_commits);
 /* Capture cleanup failure before DONE becomes reusable. Reading P->fault
  * after publishing DONE could observe an unrelated later generation. */
 u32 fault=dispose(p,20);
 return fault&&!st?1u:st;
}
/* Hook the original E-release tail as well, to observe native cancellation.
 * The native caller still owns clearing R; do not clear a reused R here. */
void m3_probe_native_free(u64 e){
 /* COMPOSING/WRITING/DISPOSING owner must join before E/private buffers are
  * freed. Full native M/R cancellation ownership remains a separate gate.
  * Epoch-qualified control CAS cannot cancel a subsequently reused slot. */
 for(unsigned i=0;i<FRAME_SLOTS;i++){
  volatile struct probe_state*p=&TABLE->frames[i];u64 observed=control(p);u32 st=(u32)(observed&255);if(!active(st)||st==CLAIMING||P->E!=e)continue;
  u64 epoch=observed>>32;add(&TABLE->cancel_requests);
  for(;;){
   if((observed>>32)!=epoch||!active((u32)(observed&255)))return;
   st=(u32)(observed&255);u64 next=observed|CANCEL_FLAG;
   if(st==HELD||st==PREPARED)next=(next&~255ull)|DISPOSING;
   if(!__atomic_compare_exchange_n(&P->control,&observed,next,0,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE))continue;
   if(st==HELD||st==PREPARED)dispose(p,30);
   return;
  }
 }
 add(&TABLE->unmatched_free);FN(0x4104d0c0,void(*)(u64))(e);
}
#undef P
