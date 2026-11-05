#ifndef MARK3_NATIVE_HUD_PUBLISH_H
#define MARK3_NATIVE_HUD_PUBLISH_H
#include <stdint.h>
uint32_t m3_hud_quality_callback(uint32_t,void*);
uint32_t m3_hud_star_callback(uint32_t,void*);
uint32_t m3_hud_mode_callback(uint32_t,void*);
uint32_t m3_hud_remaining_callback(uint32_t,void*);
void m3_hud_remaining_source_changed(void);
void m3_native_buffer_publish(uint32_t,uint32_t);
void m3_hud_selection_changed(void);
#endif
