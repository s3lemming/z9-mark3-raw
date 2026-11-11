#ifndef M3_AUXILIARY_ENTRY_PROBE_H
#define M3_AUXILIARY_ENTRY_PROBE_H
#include <stdint.h>
#include <stddef.h>
/* Diagnostic-only ABI. No native metadata, descriptors or pixels are edited. */
struct m3_aux_entry_anchor {
 uint64_t pending[3];uint32_t quota,dropped;uint64_t filesystem_owner;
 uint32_t enumeration_rc,create_rc,write_rc,close_rc,root,stage;
};
_Static_assert(sizeof(struct m3_aux_entry_anchor)==64,"entry anchor ABI");
uint32_t m3_aux_entry_nrmap(void*,uint64_t);
uint32_t m3_aux_entry_facemap(void*,uint64_t);
void m3_aux_entry_retain(uint64_t);
void m3_aux_entry_write(uint64_t,uint64_t);
#endif
