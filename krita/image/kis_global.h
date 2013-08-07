/*
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#ifndef KISGLOBAL_H_
#define KISGLOBAL_H_

#include <limits.h>

#include <kglobal.h>
#include <calligraversion.h>

#include <KoConfig.h>

#define KRITA_VERSION CALLIGRA_VERSION

const quint8 quint8_MAX = UCHAR_MAX;
const quint16 quint16_MAX = 65535;

const qint32 qint32_MAX = (2147483647);
const qint32 qint32_MIN = (-2147483647 - 1);

const quint8 MAX_SELECTED = UCHAR_MAX;
const quint8 MIN_SELECTED = 0;
const quint8 SELECTION_THRESHOLD = 1;

enum enumCursorStyle {
    CURSOR_STYLE_TOOLICON = 0,
    CURSOR_STYLE_CROSSHAIR = 1,
    CURSOR_STYLE_POINTER = 2,
    CURSOR_STYLE_OUTLINE = 3,
    CURSOR_STYLE_NO_CURSOR = 4,
    CURSOR_STYLE_SMALL_ROUND = 5
};

/*
 * Most wacom pads have 512 levels of pressure; Qt only supports 256, and even
 * this is downscaled to 127 levels because the line would be too jittery, and
 * the amount of masks take too much memory otherwise.
 */
const qint32 PRESSURE_LEVELS = 127;
const double PRESSURE_MIN = 0.0;
const double PRESSURE_MAX = 1.0;
const double PRESSURE_DEFAULT = PRESSURE_MAX;
const double PRESSURE_THRESHOLD = 5.0 / 255.0;

// copy of lcms.h
#define INTENT_PERCEPTUAL                 0
#define INTENT_RELATIVE_COLORIMETRIC      1
#define INTENT_SATURATION                 2
#define INTENT_ABSOLUTE_COLORIMETRIC      3

#include <cmath>
#include <QPointF>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


// converts \p a to [0, 2 * M_PI) range
inline qreal normalizeAngle(qreal a) {
    if (a < 0.0) {
        a = 2 * M_PI + fmod(a, 2 * M_PI);
    }

    return a > 2 * M_PI ? fmod(a, 2 * M_PI) : a;
}

inline qreal shortestAngularDistance(qreal a, qreal b) {
    qreal dist = fmod(qAbs(a - b), 2 * M_PI);
    if (dist > M_PI) dist = 2 * M_PI - dist;

    return dist;
}

inline qreal incrementInDirection(qreal a, qreal inc, qreal direction) {
    qreal b1 = a + inc;
    qreal b2 = a - inc;

    qreal d1 = shortestAngularDistance(b1, direction);
    qreal d2 = shortestAngularDistance(b2, direction);

    return d1 < d2 ? b1 : b2;
}

template<typename T>
inline T pow2(T x) {
    return x * x;
}

template<>
inline QPointF qAbs(const QPointF &pt) {
    return QPointF(qAbs(pt.x()), qAbs(pt.y()));
}

#endif // KISGLOBAL_H_

