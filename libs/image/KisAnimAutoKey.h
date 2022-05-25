/*
 *  SPDX-FileCopyrightText: 2022 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Eoin O'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_ANIM_AUTOKEY_H
#define _KIS_ANIM_AUTOKEY_H

#include "kis_image_config.h"
#include "kritaimage_export.h"

namespace KisAutoKey {
    enum KRITAIMAGE_EXPORT Mode {
        NONE,     // AutoKey is disabled. Keyframes must be created manually.
        BLANK,    // AutoKey creates an empty/blank frame.
        DUPLICATE // AutoKey will duplicate the active frame.
    };

    inline KRITAIMAGE_EXPORT Mode activeMode() {
        KisImageConfig cfg(true);
        if (cfg.autoKeyEnabled()) {
            return cfg.autoKeyModeDuplicate() ? KisAutoKey::DUPLICATE : KisAutoKey::BLANK;
        }
        return KisAutoKey::NONE;
    }
}

#endif //_KIS_ANIM_AUTOKEY_H
