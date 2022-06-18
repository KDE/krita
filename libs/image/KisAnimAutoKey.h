/*
 *  SPDX-FileCopyrightText: 2022 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Eoin O'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_ANIM_AUTOKEY_H
#define _KIS_ANIM_AUTOKEY_H

#include "kritaimage_export.h"

namespace KisAutoKey {
enum Mode {
    NONE, // AutoKey is disabled. Keyframes must be created manually.
    BLANK, // AutoKey creates an empty/blank frame. Acts like DUPLICATE when not
           // applicable (i.e.: filters, transforms, etc.).
    DUPLICATE // AutoKey will duplicate the active frame.
};

KRITAIMAGE_EXPORT Mode activeMode();
}

#endif //_KIS_ANIM_AUTOKEY_H
