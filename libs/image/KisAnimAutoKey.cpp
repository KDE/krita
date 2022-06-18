/*
 *  SPDX-FileCopyrightText: 2022 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Eoin O'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAnimAutoKey.h"
#include "kis_image_config.h"


namespace KisAutoKey
{

Mode activeMode()
{
    KisImageConfig cfg(true);
    if (cfg.autoKeyEnabled()) {
        return cfg.autoKeyModeDuplicate() ? KisAutoKey::DUPLICATE
                                          : KisAutoKey::BLANK;
    }
    return KisAutoKey::NONE;
}
}
