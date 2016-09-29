/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    bool KRITALIBKRA_EXPORT loadValue(const QDomElement &e, KisLazyFillTools::KeyStroke *stroke, const KoColorSpace *colorSpace);
}

#endif /* __KIS_COLORIZE_DOM_UTILS_H */
