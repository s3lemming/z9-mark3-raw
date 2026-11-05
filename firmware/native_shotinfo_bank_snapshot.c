#include "native_shotinfo_bank_snapshot.h"
static uint32_t read32(const volatile uint8_t*p){return(uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static uint64_t read64(const volatile uint8_t*p){return read32(p)|(uint64_t)read32(p+4)<<32;}
static int overlaps(uintptr_t a,size_t an,uintptr_t b,size_t bn){return a<b+bn&&b<a+an;}
int m3_bank_snapshot_record(const volatile void *descriptor,size_t descriptor_bytes,
 uint32_t expected_bytes,void *owned_record,size_t record_bytes,size_t payload_offset,
 const struct m3_bank_mutex_ops *mutex){
 uintptr_t dp=(uintptr_t)descriptor,op=(uintptr_t)owned_record;
 if(!dp||descriptor_bytes!=32||dp>UINTPTR_MAX-32||!op||
    (expected_bytes!=2&&expected_bytes!=8&&expected_bytes!=800)||
    payload_offset>record_bytes||record_bytes-payload_offset!=expected_bytes||
    op>UINTPTR_MAX-record_bytes||overlaps(dp,32,op,record_bytes)||
    !mutex||!mutex->lock||!mutex->unlock)return M3_BANK_INVALID;
 const volatile uint8_t*d=descriptor;int result=M3_BANK_UNAVAILABLE;
 /* Original40C7AA04 holds this same mutex for its entire payload write.
  * Copy while holding it, without consuming pending or toggling write-bank.
  * Calling the original getter and retaining its pointer would not pin it. */
 mutex->lock((void*)(dp+16),mutex->opaque);
 uint32_t n=read32(d);uintptr_t base=(uintptr_t)read64(d+8);uint8_t pending=d[28],bank=d[29],available=d[30];
 if(n!=expected_bytes||bank>1){result=M3_BANK_INVALID;goto done;}
 if(!available||!base)goto done;
 if(base>UINTPTR_MAX-2u*n||overlaps(base,2u*n,op,record_bytes)||overlaps(base,2u*n,dp,32)){result=M3_BANK_INVALID;goto done;}
 /* Getter semantics: pending1 reads current write-bank then flips it;
  * pending0 reads the opposite bank. Only the resulting bytes are copied. */
 const volatile uint8_t*source=(const volatile uint8_t*)(base+(pending?bank:(bank^1u))*n);
 uint8_t*out=(uint8_t*)owned_record+payload_offset;for(uint32_t i=0;i<n;i++)out[i]=source[i];result=M3_BANK_COPIED;
done:
 mutex->unlock((void*)(dp+16),mutex->opaque);return result;
}
int m3_bank_snapshot_current(const volatile void *descriptor,size_t descriptor_bytes,
 uint32_t expected_bytes,void *owned_output,size_t output_bytes,
 const struct m3_bank_mutex_ops *mutex){
 if(output_bytes!=expected_bytes)return M3_BANK_INVALID;
 return m3_bank_snapshot_record(descriptor,descriptor_bytes,expected_bytes,
  owned_output,output_bytes,0,mutex);
}
