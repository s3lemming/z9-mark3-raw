#include "native_settings_load.h"
static uint32_t rd32(const uint8_t*p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static int read_bytes(const struct m3_native_load_api*a,uint64_t p,uint8_t*b,size_t n){
 for(size_t i=0;i<n;i++)b[i]=0xa5;
 return a->read(a->user,p,b,n)==1;
}
enum m3_native_load_result m3_native_settings_load_owned(
 const struct m3_native_load_api*a,const struct m3_native_storage_descriptor*d,
 const struct m3_native_settings_schema*s,struct m3_hsfc_settings*out,
 struct m3_native_load_receipt*r){
 if(!r)return M3_LOAD_INVALID;
 volatile uint8_t*zero=(volatile void*)r;for(size_t i=0;i<sizeof *r;i++)zero[i]=0;
 enum m3_native_load_result status=M3_LOAD_INVALID;
 if(!a||!out||!a->is_owned||!a->select_native||!a->read||!m3_native_settings_domain(d,s))goto done;
 if(a->is_owned(a->user)!=1){status=M3_LOAD_NOT_OWNED;goto done;}
 const struct m3_native_settings_record*records=s->records;uint64_t total=0;
 for(unsigned i=0;i<202;i++){
  if(records[i].bytes>d->body_bytes){status=M3_LOAD_MALFORMED;goto done;}
  total+=((uint64_t)records[i].bytes+3)&~3ull;
 }
 if(total!=d->body_bytes){status=M3_LOAD_MALFORMED;goto done;}
 uint64_t bank=0,footer=0;uint32_t counter=0;
 if(a->select_native(a->user,d,s,&bank,&footer,&counter)!=M3_NATIVE_BANK_FIELDS_COMPLETE){status=M3_LOAD_NATIVE_UNAVAILABLE;goto done;}
 uint64_t bank_end=0;
 if(bank==d->bank_A){bank_end=bank+d->bank_A_bytes;r->selected_bank=0;}
 if(bank==d->bank_B){bank_end=bank+d->bank_B_bytes;r->selected_bank=1;}
 if(!bank_end||footer<bank||footer>bank_end-8||((footer-bank)&3)){status=M3_LOAD_MALFORMED;goto done;}
 r->bank=bank;r->footer=footer;r->native_counter=counter;
 uint8_t header[8];if(!read_bytes(a,footer,header,8)){status=M3_LOAD_READ_FAILED;goto done;}
 if(rd32(header)!=0x4d444e45||rd32(header+4)!=counter){status=M3_LOAD_MALFORMED;goto done;}
 uint64_t p=bank,limit=footer+8,latest_footer=0,seen[4]={0,0,0,0};
 uint32_t opaque_run=0;struct m3_hsfc_settings latest={UINT32_MAX,UINT32_MAX,0};status=M3_LOAD_ABSENT;
 while(p<limit){
  if(limit-p<8){status=M3_LOAD_MALFORMED;goto done;}
  if(!read_bytes(a,p,header,8)){status=M3_LOAD_READ_FAILED;goto done;}
  uint32_t id=rd32(header),length=rd32(header+4);
  if(id==0x4d444e45){
   if(length==0x33534d48){
    /* Conservatively reject a magic-valued native counter or damaged own
     * marker unless the entire11-footer envelope is present. */
    if(limit-p<88){status=M3_LOAD_INVALID;goto done;}
    uint8_t envelope[88];if(!read_bytes(a,p,envelope,88)){status=M3_LOAD_READ_FAILED;goto done;}
    for(unsigned j=0;j<11;j++)if(rd32(envelope+j*8)!=0x4d444e45){status=M3_LOAD_INVALID;goto done;}
    struct m3_hsfc_settings candidate={UINT32_MAX,UINT32_MAX,0};enum m3_record_result result=m3_settings_decode(envelope,80,&candidate);r->private_entries++;
    if(result==M3_RECORD_OK){if(candidate.codec==3||candidate.codec==4){status=M3_LOAD_OK;latest=candidate;}else status=M3_LOAD_UNSUPPORTED;}
    else status=result==M3_RECORD_VERSION?M3_LOAD_VERSION:M3_LOAD_INVALID;
    opaque_run=0;r->native_footers+=11;latest_footer=p+80;p+=88;
   }else{
    opaque_run++;r->native_footers++;latest_footer=p;p+=8;
    if(opaque_run>=11){status=M3_LOAD_INVALID;if(opaque_run==11)r->suspicious_runs++;}
   }
  }else{
   opaque_run=0;
   if(id>=202||length!=records[id].bytes||length>d->body_bytes){status=M3_LOAD_MALFORMED;goto done;}
   uint64_t bytes=((uint64_t)length+3)&~3ull;
   if(bytes>limit-p-8){status=M3_LOAD_MALFORMED;goto done;}
   seen[id/64]|=1ull<<(id%64);r->native_records++;p+=8+bytes;
  }
 }
 r->scanned_bytes=p-bank;
 for(unsigned i=0;i<202;i++)if(seen[i/64]&(1ull<<(i%64)))r->covered_native_records++;
 if(r->covered_native_records!=202||latest_footer!=footer){status=M3_LOAD_MALFORMED;goto done;}
 if(!read_bytes(a,footer,header,8)){status=M3_LOAD_READ_FAILED;goto done;}
 if(rd32(header)!=0x4d444e45||rd32(header+4)!=counter){status=M3_LOAD_MALFORMED;goto done;}
 if(a->is_owned(a->user)!=1){status=M3_LOAD_NOT_OWNED;goto done;}
 if(status==M3_LOAD_OK)*out=latest;
done:
 r->status=status;return status;
}
