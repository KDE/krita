/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006, 2010 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOINSETS_H
#define KOINSETS_H

#include "kritaflake_export.h"

#include <QDebug>

/**
 * An Insets object is a representation of the strokes of a shape.
 */
struct KRITAFLAKE_EXPORT KoInsets
{
public:
    /**
     * Constructor.
     * @param top the inset at the top.
     * @param left the inset at the left
     * @param bottom the inset at the bottom
     * @param right the inset at the right
     */
    KoInsets(qreal top, qreal left, qreal bottom, qreal right)
    : top(top)
    , bottom(bottom)
    , left(left)
    , right(right)
    {
    }
    /**
     * Constructor.
     * Initializes all values to 0
     */
    KoInsets() : top(0.), bottom(0.), left(0.), right(0.) {
    }

    /// clears the insets so all sides are set to zero
    void clear() {
        top = 0;
        bottom = 0;
        left = 0;
        right = 0;
    }

    qreal top;     ///< Top inset
    qreal bottom;  ///< Bottom inset
    qreal left;    ///< Left inset
    qreal right;   ///< Right inset
};

#ifndef QT_NO_DEBUG_STREAM
KRITAFLAKE_EXPORT QDebug operator<<(QDebug, const KoInsets &);
#endif

#endif
