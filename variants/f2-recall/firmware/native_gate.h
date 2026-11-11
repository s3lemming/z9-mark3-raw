#ifndef MARK3_NATIVE_GATE_H
#define MARK3_NATIVE_GATE_H
#include <stdint.h>
#include <stddef.h>
struct m3_native_mutex {int32_t handle;uint8_t name[8];};
struct m3_native_gate_token;
struct m3_native_gate {
 struct m3_native_mutex native;
 uint32_t state;
 struct m3_native_gate_token *owner;
 uint64_t generation;
};
struct m3_native_gate_token {
 struct m3_native_gate *gate;
 uint64_t generation;
 uint32_t held,reserved;
};
struct m3_native_gate_api {
 void (*create)(struct m3_native_mutex*,uint32_t,uint32_t,const uint8_t*);
 int (*wait)(struct m3_native_mutex*); /* Original wrapper:1 success. */
 int32_t (*release_kernel)(int32_t); /* Original kernel:0 success. */
 void (*schedule)(void);
 const volatile uint32_t *handle_limit;
};
enum m3_gate_result {M3_GATE_OK,M3_GATE_INVALID,M3_GATE_BUSY,M3_GATE_UNAVAILABLE,M3_GATE_NATIVE_FAILED,M3_GATE_EXHAUSTED};
_Static_assert(sizeof(struct m3_native_mutex)==12,"Native handle/name ABI");
_Static_assert(sizeof(struct m3_native_gate)==32,"Owned gate ABI");
_Static_assert(sizeof(struct m3_native_gate_token)==24,"Invocation token ABI");
/* These addresses bind native wrappers, not kernel initialization or a live
 * object. init requires a serialized, fresh zero owner and initialized RTOS
 * in an ordinary task context. Names are exactly8 readable bytes. No IRQ use.
 * Each stack token belongs to one task/invocation and is never shared/copied.
 * Native mutex release must obey LIFO order with every other native mutex.
 * Binding a gate alone does not wrap ALL F saves or freeze native producers. */
extern const struct m3_native_gate_api m3_native_gate_rom532;
enum m3_gate_result m3_native_gate_init(const struct m3_native_gate_api*,struct m3_native_gate*,const uint8_t name[8]);
enum m3_gate_result m3_native_gate_acquire(const struct m3_native_gate_api*,struct m3_native_gate*,struct m3_native_gate_token*);
int m3_native_gate_owned(const struct m3_native_gate*,const struct m3_native_gate_token*);
enum m3_gate_result m3_native_gate_release(const struct m3_native_gate_api*,struct m3_native_gate*,struct m3_native_gate_token*);
#endif
