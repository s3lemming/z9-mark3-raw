#ifndef MARK3_NATIVE_RUNTIME_LAYOUT_H
#define MARK3_NATIVE_RUNTIME_LAYOUT_H
/* Exactly two startup-owned pages. Native401611CC adds one guard page before
 * 16KiB alignment, so a third page moves the original table arena. */
#define M3_FRAME_STATE_ADDRESS 0xa04e3d000ull
#define M3_FRAME_STATE_LIMIT 0xc00u
#define M3_LIVE_SETTINGS_ADDRESS (M3_FRAME_STATE_ADDRESS+M3_FRAME_STATE_LIMIT)
#ifdef M3_INTEGRATED_AUDIO
/* A32-byte boot-owned anchor points to the separate mark2 audio allocation. */
#define M3_LIVE_SETTINGS_LIMIT (4096u-M3_FRAME_STATE_LIMIT-32u)
#else
#define M3_LIVE_SETTINGS_LIMIT (4096u-M3_FRAME_STATE_LIMIT)
#endif
#define M3_MENU_STATE_ADDRESS 0xa04e3e000ull
#endif
