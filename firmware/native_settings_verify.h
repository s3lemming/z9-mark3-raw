#ifndef MARK3_NATIVE_SETTINGS_VERIFY_H
#define MARK3_NATIVE_SETTINGS_VERIFY_H
#include "native_settings_context.h"
#include "native_settings_view.h"
#include "native_settings_bank.h"
/* Save-API verify callback. Reconstruct current original F view, use native
 * bank/end/counter authority, load exact HE settings and compare ALL native
 * record payloads with a bounded read-only last-record-wins walk. Original
 *40D75A0C is a mutating DIFFERENCE builder(0 equal),never called on live F
 * records here. Native bank caches may update. No accept/publish/migration.
 * Requires live initialized native state and complete F owner/admission;
 * a token alone is not proof that native setting producers are immutable. */
int m3_native_saved_settings_verify(void*,const struct m3_native_save_cookie*);
#endif
