#ifndef MARK3_NATIVE_PRECAPTURE_LIMIT_H
#define MARK3_NATIVE_PRECAPTURE_LIMIT_H
#include <stdint.h>
uint32_t m3_precapture_property_get(uint32_t);
uint32_t m3_precapture_frames(uint32_t);
void m3_precapture_menu_refresh(void *);
void m3_precapture_selection_changed(void);
uint32_t m3_precapture_native_batch(const uint32_t *,const uint32_t *,uint32_t);
#endif
