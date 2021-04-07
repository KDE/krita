/*
 *  SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISGLOBAL_H_
#define KISGLOBAL_H_

#include <limits>

#include <KoConfig.h>
#include "kis_assert.h"

#include <QPoint>
#include <QPointF>

const quint8 quint8_MAX = std::numeric_limits<quint8>::max();
const quint16 quint16_MAX = std::numeric_limits<quint16>::max();

const qint16 qint16_MIN = std::numeric_limits<qint16>::min();
const qint16 qint16_MAX = std::numeric_limits<qint16>::max();
const qint32 qint32_MAX = std::numeric_limits<qint32>::max();
const qint32 qint32_MIN = std::numeric_limits<qint32>::min();

const quint8 MAX_SELECTED = std::numeric_limits<quint8>::max();
const quint8 MIN_SELECTED = std::numeric_limits<quint8>::min();
const quint8 SELECTION_THRESHOLD = 1;

enum OutlineStyle {
    OUTLINE_NONE = 0,
    OUTLINE_CIRCLE,
    OUTLINE_FULL,
    OUTLINE_TILT,

    N_OUTLINE_STYLE_SIZE
};

enum CursorStyle {
    CURSOR_STYLE_NO_CURSOR = 0,
    CURSOR_STYLE_TOOLICON,
    CURSOR_STYLE_POINTER,
    CURSOR_STYLE_SMALL_ROUND,
    CURSOR_STYLE_CROSSHAIR,
    CURSOR_STYLE_TRIANGLE_RIGHTHANDED,
    CURSOR_STYLE_TRIANGLE_LEFTHANDED,
    CURSOR_STYLE_BLACK_PIXEL,
    CURSOR_STYLE_WHITE_PIXEL,

    N_CURSOR_STYLE_SIZE
};

enum OldCursorStyle {
    OLD_CURSOR_STYLE_TOOLICON = 0,
    OLD_CURSOR_STYLE_CROSSHAIR = 1,
    OLD_CURSOR_STYLE_POINTER = 2,

    OLD_CURSOR_STYLE_OUTLINE = 3,

    OLD_CURSOR_STYLE_NO_CURSOR = 4,
    OLD_CURSOR_STYLE_SMALL_ROUND = 5,

    OLD_CURSOR_STYLE_OUTLINE_CENTER_DOT = 6,
    OLD_CURSOR_STYLE_OUTLINE_CENTER_CROSS = 7,

    OLD_CURSOR_STYLE_TRIANGLE_RIGHTHANDED = 8,
    OLD_CURSOR_STYLE_TRIANGLE_LEFTHANDED = 9,

    OLD_CURSOR_STYLE_OUTLINE_TRIANGLE_RIGHTHANDED = 10,
    OLD_CURSOR_STYLE_OUTLINE_TRIANGLE_LEFTHANDED = 11
};

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

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


// converts \p a to [0, 2 * M_PI) range
template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
normalizeAngle(T a) {
    if (a < T(0.0)) {
        a = T(2 * M_PI) + std::fmod(a, T(2 * M_PI));
    }

    return a >= T(2 * M_PI) ? std::fmod(a, T(2 * M_PI)) : a;
}

// converts \p a to [0, 360.0) range
template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
normalizeAngleDegrees(T a) {
    if (a < T(0.0)) {
        a = T(360.0) + std::fmod(a, T(360.0));
    }

    return a >= T(360.0) ? std::fmod(a, T(360.0)) : a;
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

inline qreal bisectorAngle(qreal a, qreal b) {
    const qreal diff = shortestAngularDistance(a, b);
    return incrementInDirection(a, 0.5 * diff, b);
}

template<typename PointType>
inline PointType snapToClosestAxis(PointType P) {
    if (qAbs(P.x()) < qAbs(P.y())) {
        P.setX(0);
    } else {
        P.setY(0);
    }
    return P;
}

template<typename T>
inline const T pow2(const T& x) {
    return x * x;
}

template<typename T>
inline const T pow3(const T& x) {
    return x * x * x;
}

template<typename T>
inline T kisDegreesToRadians(T degrees) {
    return degrees * M_PI / 180.0;
}

template<typename T>
inline T kisRadiansToDegrees(T radians) {
    return radians * 180.0 / M_PI;
}

template<class T, typename U>
inline T kisGrowRect(const T &rect, U offset) {
    return rect.adjusted(-offset, -offset, offset, offset);
}

inline qreal kisDistance(const QPointF &pt1, const QPointF &pt2) {
    return std::sqrt(pow2(pt1.x() - pt2.x()) + pow2(pt1.y() - pt2.y()));
}

inline qreal kisSquareDistance(const QPointF &pt1, const QPointF &pt2) {
    return pow2(pt1.x() - pt2.x()) + pow2(pt1.y() - pt2.y());
}

#include <QLineF>

inline qreal kisDistanceToLine(const QPointF &m, const QLineF &line)
{
    const QPointF &p1 = line.p1();
    const QPointF &p2 = line.p2();

    qreal distance = 0;

    if (qFuzzyCompare(p1.x(), p2.x())) {
        distance = qAbs(m.x() - p2.x());
    } else if (qFuzzyCompare(p1.y(), p2.y())) {
        distance = qAbs(m.y() - p2.y());
    } else {
        qreal A = 1;
        qreal B = - (p1.x() - p2.x()) / (p1.y() - p2.y());
        qreal C = - p1.x() - B * p1.y();

        distance = qAbs(A * m.x() + B * m.y() + C) / std::sqrt(pow2(A) + pow2(B));
    }

    return distance;
}

inline QPointF kisProjectOnVector(const QPointF &base, const QPointF &v)
{
    const qreal prod = base.x() * v.x() + base.y() * v.y();
    const qreal lengthSq = pow2(base.x()) + pow2(base.y());
    qreal coeff = prod / lengthSq;

    return coeff * base;
}

#include <QRect>

inline QRect kisEnsureInRect(QRect rc, const QRect &bounds)
{
    if(rc.right() > bounds.right()) {
        rc.translate(bounds.right() - rc.right(), 0);
    }

    if(rc.left() < bounds.left()) {
        rc.translate(bounds.left() - rc.left(), 0);
    }

    if(rc.bottom() > bounds.bottom()) {
        rc.translate(0, bounds.bottom() - rc.bottom());
    }

    if(rc.top() < bounds.top()) {
        rc.translate(0, bounds.top() - rc.top());
    }

    return rc;
}

inline QRect kisTrimLeft( int width, QRect &toTakeFrom)
{
    QPoint trimmedOrigin = toTakeFrom.topLeft();
    QSize trimmedSize = QSize(width, toTakeFrom.height());
    toTakeFrom.setWidth(toTakeFrom.width() - width);
    toTakeFrom.translate(width, 0);
    return QRect(trimmedOrigin, trimmedSize);
}

#include "kis_pointer_utils.h"

/**
 * A special wrapper object that converts Qt-style mutexes and locks
 * into an object that supports Std's (and Boost's) "Lockable"
 * concept. Basically, it converts tryLock() into try_lock() to comply
 * with the syntax.
 */

template <class T>
struct StdLockableWrapper {
    StdLockableWrapper(T *lock) : m_lock(lock) {}

    void lock() {
        m_lock->lock();
    }

    bool try_lock() {
        return m_lock->tryLock();
    }

    void unlock() {
        m_lock->unlock();
    }

private:
    T *m_lock;
};

#endif // KISGLOBAL_H_

