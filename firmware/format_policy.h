/* mark3 authored policy, separate from Z8 hook implementation. No MMIO/ROM access. */
#ifndef MARK3_FORMAT_POLICY_H
#define MARK3_FORMAT_POLICY_H
#include <stdint.h>
#define M3_REQUIRED_CONTRACTS 0x1fu
#define M3_DX_CONTRACT 0x20u
#define M3_HE_STAR_DX_CONTRACT 0x40u
/* Bit 0: effective-mode ABI; 1: frame lifetime; 2: HE encode/decode/size;
 * 3: writer/DCF; 4: menu persistence/display. All require Z9 evidence. */
/* DX additionally requires its geometry/source, metadata/crop and complete
 * delivery contract. A native area value or JPEG support alone is insufficient.
 * HE-star DX additionally requires its own size/codec contract bit0x40.
 * These logical geometry values are NOT native DSID07000603 values0/1. */
enum m3_geometry { M3_GEOMETRY_UNVERIFIED=0, M3_GEOMETRY_FX_LARGE=1,
                   M3_GEOMETRY_DX_LARGE=2 };
enum m3_mode { M3_S, M3_CL, M3_CH, M3_C15, M3_C30, M3_C60, M3_C120, M3_MOVIE };
enum m3_format { M3_RAW_JPEG=0, M3_RAW=1, M3_TIFF=2, M3_JPEG=3 };
enum m3_codec { M3_LOSSLESS=1, M3_HE_STAR=3, M3_HE=4 };
enum m3_result { M3_STOCK, M3_ALLOWED, M3_NOT_READY, M3_UNSUPPORTED, M3_BUSY, M3_INVALID };
struct m3_context {
    uint32_t effective_mode; /* Adapter must derive actual mode, never Quick DSID 1C26 alone. */
    uint32_t quick_choice;   /* Diagnostic only: may remain C15 while physical dial is S. */
    /* Legacy field name/layout retained: fx_large now holds m3_geometry.
     * Existing callers with0/1 preserve behavior; DX is explicit value2. */
    uint32_t photo, fx_large, special_processing, inflight, contracts;
};
struct m3_selection { uint32_t format, jpeg_grade, jpeg_star, codec; };
struct m3_option { const char *label; struct m3_selection value; };
extern const struct m3_option m3_format_options[13];
#define M3_HSFC_OPTION_COUNT 5u
/* Display rows map to stable policy IDs; ordinary stock 13 are untouched. */
extern const uint32_t m3_hsfc_option_ids[M3_HSFC_OPTION_COUNT];
enum m3_result m3_can_select(const struct m3_context *, const struct m3_selection *);
/* Snapshot only after admission; caller owns lock and complete transaction. */
enum m3_result m3_snapshot(const struct m3_context *, const struct m3_selection *,
                          uint32_t primary, uint32_t role, uint32_t sequence,
                          struct m3_selection *copy, uint32_t route[3]);
#endif
