#include "native_gate.h"
enum {INITIALIZING=1,READY=2,FAILED=3};
const struct m3_native_gate_api m3_native_gate_rom532={
 (void(*)(struct m3_native_mutex*,uint32_t,uint32_t,const uint8_t*))(uintptr_t)0x40d30620,
 (int(*)(struct m3_native_mutex*))(uintptr_t)0x40d306c4,
 (int32_t(*)(int32_t))(uintptr_t)0x4018058c,
 (void(*)(void))(uintptr_t)0x40183490,
 (const volatile uint32_t*)(uintptr_t)0xa02ac4ad0ull
};
static int span(const void*p,size_t n){return p&&(uintptr_t)p<=UINTPTR_MAX-n;}
static int overlap(const void*a,size_t an,const void*b,size_t bn){return(uintptr_t)a<(uintptr_t)b+bn&&(uintptr_t)b<(uintptr_t)a+an;}
static int api_valid(const struct m3_native_gate_api*a){return span(a,sizeof *a)&&(uintptr_t)a%_Alignof(struct m3_native_gate_api)==0&&a->create&&a->wait&&a->release_kernel&&a->schedule&&span((const void*)a->handle_limit,4)&&(uintptr_t)a->handle_limit%4==0;}
static int gate_valid(const struct m3_native_gate*g){return span(g,sizeof *g)&&(uintptr_t)g%_Alignof(struct m3_native_gate)==0;}
static int token_valid(const struct m3_native_gate*g,const struct m3_native_gate_token*t){return span(t,sizeof *t)&&(uintptr_t)t%_Alignof(struct m3_native_gate_token)==0&&!overlap(g,sizeof *g,t,sizeof *t);}
static uint32_t state(const struct m3_native_gate*g){return __atomic_load_n(&g->state,__ATOMIC_ACQUIRE);}
static void fail(struct m3_native_gate*g){__atomic_store_n(&g->state,FAILED,__ATOMIC_RELEASE);}
static int unlock(const struct m3_native_gate_api*a,struct m3_native_gate*g){int32_t rc=a->release_kernel(g->native.handle);a->schedule();if(rc){fail(g);return 0;}return 1;}
enum m3_gate_result m3_native_gate_init(const struct m3_native_gate_api*a,struct m3_native_gate*g,const uint8_t name[8]){
 if(!api_valid(a)||!gate_valid(g)||!span(name,8)||overlap(g,sizeof *g,a,sizeof *a)||overlap(g,sizeof *g,name,8))return M3_GATE_INVALID;
 const uint8_t*p=(const uint8_t*)g;for(size_t i=0;i<sizeof *g;i++)if(p[i])return M3_GATE_BUSY;
 uint32_t limit=*a->handle_limit;if(!limit||limit>INT32_MAX)return M3_GATE_UNAVAILABLE;
 __atomic_store_n(&g->state,INITIALIZING,__ATOMIC_RELEASE);
 a->create(&g->native,2,0,name);
 if(g->native.handle<=0||(uint32_t)g->native.handle>limit){fail(g);return M3_GATE_NATIVE_FAILED;}
 __atomic_store_n(&g->state,READY,__ATOMIC_RELEASE);return M3_GATE_OK;
}
enum m3_gate_result m3_native_gate_acquire(const struct m3_native_gate_api*a,struct m3_native_gate*g,struct m3_native_gate_token*t){
 if(!api_valid(a)||!gate_valid(g)||!token_valid(g,t)||overlap(t,sizeof *t,a,sizeof *a))return M3_GATE_INVALID;
 if(t->gate||t->generation||t->held||t->reserved)return M3_GATE_BUSY;
 if(state(g)!=READY||g->native.handle<=0||(uint32_t)g->native.handle>*a->handle_limit)return M3_GATE_UNAVAILABLE;
 if(a->wait(&g->native)!=1)return M3_GATE_NATIVE_FAILED;
 if(state(g)!=READY||__atomic_load_n(&g->owner,__ATOMIC_ACQUIRE)){fail(g);unlock(a,g);return M3_GATE_UNAVAILABLE;}
 if(g->generation==UINT64_MAX){fail(g);unlock(a,g);return M3_GATE_EXHAUSTED;}
 t->gate=g;t->generation=++g->generation;t->held=1;t->reserved=0;
 __atomic_store_n(&g->owner,t,__ATOMIC_RELEASE);return M3_GATE_OK;
}
int m3_native_gate_owned(const struct m3_native_gate*g,const struct m3_native_gate_token*t){
 if(!gate_valid(g)||!token_valid(g,t)||state(g)!=READY)return 0;
 if(__atomic_load_n(&g->owner,__ATOMIC_ACQUIRE)!=t)return 0;
 return t->gate==g&&t->held==1&&!t->reserved&&t->generation&&t->generation==g->generation;
}
enum m3_gate_result m3_native_gate_release(const struct m3_native_gate_api*a,struct m3_native_gate*g,struct m3_native_gate_token*t){
 if(!api_valid(a)||!m3_native_gate_owned(g,t))return M3_GATE_INVALID;
 __atomic_store_n(&g->owner,0,__ATOMIC_RELEASE);
 t->gate=0;t->generation=0;t->held=0;t->reserved=0;
 return unlock(a,g)?M3_GATE_OK:M3_GATE_NATIVE_FAILED;
}
