/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoInsets.h"

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug debug, const KoInsets &insets)
{
#ifndef NDEBUG
    debug.nospace() << "KoInsets [top=" << insets.top;
    debug.nospace() << ", left=" << insets.left;
    debug.nospace() << ", bottom=" << insets.bottom;
    debug.nospace() << ", right=" << insets.right << ']';
#else
    Q_UNUSED(insets);
#endif
    return debug.space();
}
#endif
