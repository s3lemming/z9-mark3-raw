#ifndef M3_STOCK_NEF_JOB_H
#define M3_STOCK_NEF_JOB_H
#include <stddef.h>
#include <stdint.h>
#ifndef M3_PROBE_ALLOW_FULL_HE_DX
#define M3_PROBE_ALLOW_FULL_HE_DX 0
#endif
#define M3_NATIVE_FRAME_ENTRY_BYTES 0x2d8u
#define M3_NATIVE_METADATA_BYTES 0x4480u
struct m3_stock_job_api {
 uint32_t (*initialize)(uint32_t,void *,void *,void *,void *);
 uint32_t (*header_capacity)(uint32_t);
};
struct m3_stock_job_inputs {
 uint32_t NEF_file_id;
 const uint8_t *capture_R;size_t R_bytes;
 const uint8_t *capture_M;size_t M_bytes;
 const uint8_t *presentation_Y;size_t Y_bytes;
 const uint8_t *auxiliary_Y;size_t auxiliary_Y_bytes;
 void *header_resource;size_t header_resource_bytes;
 const uint8_t *header_buffer_descriptor;size_t header_descriptor_bytes;
};
enum m3_stock_job_state { M3_JOB_EMPTY=0,M3_JOB_DESCRIPTORS_PREPARED=1 };
struct m3_stock_nef_job {
 _Alignas(16) uint8_t native_entry[M3_NATIVE_FRAME_ENTRY_BYTES];
 uint32_t state,codec_class,header_capacity;
};
enum m3_stock_job_result { M3_JOB_OK=0,M3_JOB_INVALID=1,M3_JOB_BUSY=2,M3_JOB_NATIVE_ERROR=3 };
/* The caller owns all objects, the real native header-pool reservation and
 * actual same-frame pixel/metadata lifetimes. Equality of pointers here does
 * not establish capture identity, DMA completion, storage reservation or locks.
 * Output is a native descriptor job only, never a composed/accepted/written NEF.
 * Caller keeps M/Y/header and output alive until all native consumers finish.
 */
enum m3_stock_job_result m3_stock_native_nef_job(const struct m3_stock_job_api *,const struct m3_stock_job_inputs *,struct m3_stock_nef_job *);
#endif
