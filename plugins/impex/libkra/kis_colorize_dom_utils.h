/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_COLORIZE_DOM_UTILS_H
#define __KIS_COLORIZE_DOM_UTILS_H

#include <KoColorSpaceRegistry.h>
#include "kritalibkra_export.h"
namespace KisLazyFillTools {
    struct KeyStroke;
}

namespace KisDomUtils {
    void KRITALIBKRA_EXPORT saveValue(QDomElement *parent, const QString &tag, const KisLazyFillTools::KeyStroke &stroke);
    bool KRITALIBKRA_EXPORT loadValue(const QDomElement &e,
                                      KisLazyFillTools::KeyStroke *stroke,
                                      const KoColorSpace *colorSpace,
                                      const QPoint &offset);
}

#endif /* __KIS_COLORIZE_DOM_UTILS_H */
