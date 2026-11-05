#ifndef M23_AUDIO_RUNTIME_H
#define M23_AUDIO_RUNTIME_H
#include <stddef.h>
#include <stdint.h>
/* One immutable allocation per boot; no PCM or runtime reclamation implied. */
enum { M23_AUDIO_BYTES=8192, M23_AUDIO_ADAPTER_LIMIT=1536 };
void m23_audio_runtime_init(void);
void *m23_audio_runtime_base(void);
int m23_audio_runtime_ready(void);
#endif
