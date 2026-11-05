#ifndef M3_HSFC_GEOMETRY_MODE_H
#define M3_HSFC_GEOMETRY_MODE_H
#include <stdint.h>
/* Original 5.32 mode/area number spaces, not mark3 menu-option IDs.
 * Original4105F564 already maps native C15(6)/DX(1) to processing profile5.
 * Extend that same native profile to C30(3)/DX(1). Release mode and timing
 * fields are not rewritten. All other input pairs retain original results.
 * Current use is an explicit HE RAW diagnostic; product-menu binding and
 * physical frame-rate/encoder support require separate qualification.
 */
uint32_t m3_hsfc_geometry_mode(uint32_t native_mode,uint32_t native_area);
#endif
