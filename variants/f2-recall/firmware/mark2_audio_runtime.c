#include "mark2_audio_runtime.h"
#include "native_runtime_layout.h"
#ifndef M23_ANCHOR
#define M23_ANCHOR ((struct anchor *)(uintptr_t)(M3_FRAME_STATE_ADDRESS+4096u-32u))
#endif
#ifndef M23_ALLOC
#define M23_ALLOC(n) (((void *(*)(uint32_t,uint32_t))(uintptr_t)0x40d30458u)((n),0))
#endif
struct anchor {uint32_t magic,state,attempts,fault;uintptr_t allocation;uint32_t bytes,reserved;};
_Static_assert(sizeof(struct anchor)==32,"Audio anchor ABI");
_Static_assert(M3_LIVE_SETTINGS_LIMIT<=4096u-M3_FRAME_STATE_LIMIT-32u,"Audio anchor overlaps live settings");
void m23_audio_runtime_init(void){
 struct anchor *a=M23_ANCHOR;uint32_t expected=0;
 if(!__atomic_compare_exchange_n(&a->state,&expected,1,0,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE))return;
 a->magic=0x4d323333u;a->attempts++;
 uint8_t *p=M23_ALLOC(M23_AUDIO_BYTES);
 if(!p){a->fault=1;__atomic_store_n(&a->state,3,__ATOMIC_RELEASE);return;}
 /* Native allocation is fully initialized before any menu/audio reader sees it.
  * It stays owned until boot ends; no callback or DMA retirement is invented. */
 for(unsigned i=0;i<M23_AUDIO_BYTES;i++)p[i]=0;
 a->bytes=M23_AUDIO_BYTES;__atomic_store_n(&a->allocation,(uintptr_t)p,__ATOMIC_RELEASE);
 __atomic_store_n(&a->state,2,__ATOMIC_RELEASE);
}
int m23_audio_runtime_ready(void){return __atomic_load_n(&M23_ANCHOR->state,__ATOMIC_ACQUIRE)==2;}
void *m23_audio_runtime_base(void){return m23_audio_runtime_ready()?(void *)__atomic_load_n(&M23_ANCHOR->allocation,__ATOMIC_ACQUIRE):0;}
