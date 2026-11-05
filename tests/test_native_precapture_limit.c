#define _GNU_SOURCE
#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "native_precapture_limit.h"
static uint64_t selection;static uint32_t photo=1,mode=3,area,special,requested=3;
static unsigned calls,refreshes,disabled,checks,notifications,doorbells;
static uint32_t batch_rc;static unsigned batch_calls,notify_before_batch;
static const uint32_t *expected_ids,*expected_values;static uint32_t expected_count;
static unsigned queue_locked;
#define C(x) do{assert(x);checks++;}while(0)
uint64_t m3_precapture_test_selection(void){return selection;}
uint32_t m3_precapture_test_applied_mode(void){return *(uint32_t*)(uintptr_t)0xa03d9d218ull;}
static uint32_t cached(uint32_t id){switch(id){case 0x1d00:return photo;case 0x1c26:return mode;case 0x603:return area;case 0x1c8c:return special;default:abort();}}
static uint32_t original_get(uint32_t id){calls++;return id==0x02006401?requested:0x12345678;}
static void refresh(void *ctx){
 refreshes++;
 if(ctx==(void*)(uintptr_t)0xa02905f50ull&&m3_precapture_property_get(0x02006401)==3)
  assert(!disabled); /* Original focus must be allowed to restore the1s row. */
}
static void disable(void *ctx,uint32_t ui,uint32_t value){assert(ctx==(void*)(uintptr_t)0xa02905f50ull&&ui==0x35be);disabled=value;}
static uint32_t mutex_fixture(uintptr_t p){assert(p==0xa03e132a8ull&&!queue_locked);queue_locked=1;return 1;}
static uint32_t unlock_fixture(uintptr_t p){assert(p==0xa03e132a8ull&&queue_locked);queue_locked=0;return 1;}
static void bell_fixture(void){doorbells++;}
static uint32_t notify(uint32_t id,uint32_t value){assert(id==0x02006401);notifications++;return((uint32_t(*)(uint32_t,uint32_t))(uintptr_t)0x41017c30)(id,value);}
static uint32_t native_batch(const uint32_t*ids,const uint32_t*values,uint32_t count){
 assert(ids==expected_ids&&values==expected_values&&count==expected_count);
 assert(notifications==notify_before_batch&&!queue_locked);batch_calls++;
 if(batch_rc)return batch_rc;
 if(!ids||!values||!count||count>512)return 0; /* Invalid fixture has no borrowed array. */
 return((uint32_t(*)(const uint32_t*,const uint32_t*,uint32_t))(uintptr_t)0x41017c54)(ids,values,count);
}
static uint32_t batch(const uint32_t*ids,const uint32_t*values,uint32_t count){
 expected_ids=ids;expected_values=values;expected_count=count;notify_before_batch=notifications;
 unsigned prior=batch_calls;uint32_t rc=m3_precapture_native_batch(ids,values,count);
 assert(batch_calls==prior+1&&!queue_locked);return rc;
}
uintptr_t m3_precapture_test_function(uintptr_t a){switch(a){case 0x410db64c:return(uintptr_t)original_get;case 0x406a3f54:return(uintptr_t)cached;case 0x40525404:return(uintptr_t)refresh;case 0x403b1f74:return(uintptr_t)disable;case 0x41017c30:return(uintptr_t)notify;case 0x41017c54:return(uintptr_t)native_batch;case 0x40f9d61c:return a;default:abort();}}
static void apply(uint32_t v){*(uint32_t*)(uintptr_t)0xa03d9d218ull=v;}
static void map(uint64_t at,size_t n){assert(mmap((void*)(uintptr_t)at,n,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)==(void*)(uintptr_t)at);}
static void relay(uintptr_t at,uintptr_t target){uint32_t ins[2]={0x58000050u,0xd61f0200u};memcpy((void*)at,ins,8);memcpy((void*)(at+8),&target,8);__builtin___clear_cache((char*)at,(char*)(at+16));}
int main(int argc,char **argv){
 assert(argc==2);int fd=open(argv[1],O_RDONLY);assert(fd>=0);off_t n=lseek(fd,0,SEEK_END);assert(n>0);size_t z=((size_t)n+4095)&~4095ull;map(0x40160000,z);assert(pread(fd,(void*)(uintptr_t)0x40160000,n,0)==n);
 map(0xa029e4000,0x4000);assert(pread(fd,(void*)(uintptr_t)0xa029e4000,0x4000,0xa029e4000ull-0x9fdd0b000ull)==0x4000);close(fd);map(0xa03d9d000,0x1000);__builtin___clear_cache((char*)(uintptr_t)0x40160000,(char*)(uintptr_t)(0x40160000+z));
 /* Execute complete original ID/value queue publication, with only RTOS
  * mutex/doorbell dependencies replaced in disposable EL0 RAM. One native
  * subscriber is registered for6401; original queue construction is intact. */
 map(0xa03db4000ull,0x90000);*(uint32_t*)(uintptr_t)0xa03db4aa0ull=1;
 *(uint32_t*)(uintptr_t)0xa03db4ab8ull=1;
 *(uint8_t*)(uintptr_t)(0xa03db4aa0ull+0x1c+0x64*32)=2;
 relay(0x40d306c4,(uintptr_t)mutex_fixture);relay(0x40d306b0,(uintptr_t)unlock_fixture);relay(0x41016878,(uintptr_t)bell_fixture);
 uint32_t(*native)(uint32_t)=(void*)(uintptr_t)0x40f9d61c;apply(3);C(native(3)==30);C(native(2)==15);C(native(0)==0);
 selection=((uint64_t)3<<32);C(m3_precapture_property_get(0x02006401)==2);C(requested==3);C(m3_precapture_frames(3)==15);C(m3_precapture_frames(0)==0);C(m3_precapture_frames(2)==15);C(m3_precapture_property_get(0x02006402)==0x12345678);
 m3_precapture_menu_refresh((void*)(uintptr_t)0xa02905f50ull);C(refreshes==1&&disabled==1);
 area=1;C(m3_precapture_property_get(0x02006401)==3);C(m3_precapture_frames(3)==30);m3_precapture_menu_refresh((void*)(uintptr_t)0xa02905f50ull);C(disabled==0);area=0;
 selection=((uint64_t)4<<32);C(m3_precapture_property_get(0x02006401)==3);C(m3_precapture_frames(3)==30);
 selection=((uint64_t)3<<32)|8;C(m3_precapture_frames(3)==30);selection=((uint64_t)3<<32)|10;C(m3_precapture_property_get(0x02006401)==3);
 for(unsigned i=0;i<3;i++){selection=((uint64_t)3<<32)|(i==0?0:i==1?2:4);C(m3_precapture_frames(3)==15);}
 requested=4;C(m3_precapture_property_get(0x02006401)==4);requested=3;
 mode=6;apply(6);C(native(3)==15);C(m3_precapture_property_get(0x02006401)==3);C(m3_precapture_frames(3)==15);
 mode=3;C(m3_precapture_property_get(0x02006401)==2);C(m3_precapture_frames(3)==15);apply(3);C(m3_precapture_frames(3)==15);
 special=1;C(m3_precapture_property_get(0x02006401)==3);C(m3_precapture_frames(3)==30);special=0;photo=0;C(m3_precapture_frames(3)==30);photo=1;selection=0;C(m3_precapture_property_get(0x02006401)==3);
 disabled=9;m3_precapture_menu_refresh((void*)(uintptr_t)1);C(disabled==9);
 selection=((uint64_t)3<<32);m3_precapture_selection_changed();C(notifications==1&&requested==3&&doorbells==1);
 uint32_t*q=(void*)(uintptr_t)0xa03de3128ull;C(q[1]==1&&q[2]==0x02006401&&q[3]==2);
 selection=((uint64_t)4<<32);m3_precapture_selection_changed();C(notifications==2&&requested==3&&doorbells==1);C(q[1]==1&&q[3]==3);
 requested=4;m3_precapture_selection_changed();C(notifications==2&&q[3]==3);requested=3;
 uint32_t ids[3]={0x07000603,0x07000603,0x02006401},values[3]={1,1,3};
 selection=(uint64_t)3<<32;photo=1;special=0;mode=3;apply(3);
 for(unsigned option=0;option<=4;option+=2){
  selection=((uint64_t)3<<32)|option;unsigned before=notifications;
  area=1;C(batch(ids,values,1)==0);C(notifications==before+1&&q[3]==3&&m3_precapture_frames(requested)==30&&requested==3);
  area=0;values[0]=0;C(batch(ids,values,1)==0);C(notifications==before+2&&q[3]==2&&m3_precapture_frames(requested)==15&&requested==3);values[0]=1;
 }
 unsigned before=notifications;area=1;C(batch(ids,values,2)==0);C(notifications==before+1); /* One extra notification per native batch. */
 before=notifications;batch_rc=0xabcdef01;C(batch(ids,values,1)==batch_rc);C(notifications==before);batch_rc=0;
 ids[0]=0x02000603;C(batch(ids,values,1)==0&&notifications==before);ids[0]=0x04000603;C(batch(ids,values,1)==0&&notifications==before);ids[0]=0x02006401;C(batch(ids,values,1)==0&&notifications==before);ids[0]=0x07000603;
 values[0]=2;C(batch(ids,values,1)==0&&notifications==before);values[0]=1;
 selection=(uint64_t)4<<32;C(batch(ids,values,1)==0&&notifications==before);
 selection=((uint64_t)3<<32)|8;C(batch(ids,values,1)==0&&notifications==before);selection=((uint64_t)3<<32)|10;C(batch(ids,values,1)==0&&notifications==before);selection=0;C(batch(ids,values,1)==0&&notifications==before);selection=(uint64_t)3<<32;
 apply(6);C(batch(ids,values,1)==0&&notifications==before);apply(3);
 photo=0;C(batch(ids,values,1)==0&&notifications==before);photo=1;special=1;C(batch(ids,values,1)==0&&notifications==before);special=0;area=2;C(batch(ids,values,1)==0&&notifications==before);area=1;
 C(batch(0,values,1)==0&&notifications==before);C(batch(ids,0,1)==0&&notifications==before);C(batch(ids,values,0)==0&&notifications==before);C(batch(ids,values,513)==0&&notifications==before);
 for(requested=0;requested<4;requested++){area=0;C(batch(ids,values,1)==0&&q[3]==(requested==3?2:requested));area=1;C(batch(ids,values,1)==0&&q[3]==requested);}
 printf("PRECAPTURE_NATIVE_ORIGINAL_CONVERSION_CHECKS %u original_getter_calls=%u\n",checks,calls);return 0;
}
