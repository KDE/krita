/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_WRAPPED_RECT_H
#define __KIS_WRAPPED_RECT_H

#include <QVector>
#include <QRect>
#include <QtMath>

#include "KisWraparoundAxis.h"

struct KisWrappedRect : public QVector<QRect> {
    static inline int xToWrappedX(int x, const QRect &wrapRect, WrapAroundAxis wrapAxis) {
        if (wrapAxis == WRAPAROUND_VERTICAL) {
            return x;
        }
        x = (x - wrapRect.x()) % wrapRect.width();
        if (x < 0) x += wrapRect.width();
        return x;
    }

    static inline int yToWrappedY(int y, const QRect &wrapRect, WrapAroundAxis wrapAxis) {
        if (wrapAxis == WRAPAROUND_HORIZONTAL) {
            return y;
        }
        y = (y - wrapRect.y()) % wrapRect.height();
        if (y < 0) y += wrapRect.height();
        return y;
    }

    static inline QPoint ptToWrappedPt(QPoint pt, const QRect &wrapRect, WrapAroundAxis wrapAxis) {
        pt.rx() = xToWrappedX(pt.x(), wrapRect, wrapAxis);
        pt.ry() = yToWrappedY(pt.y(), wrapRect, wrapAxis);
        return pt;
    }

    static inline QRect clipToWrapRect(QRect rc, const QRect &wrapRect, WrapAroundAxis wrapAxis) {
        switch (wrapAxis) {
            case WRAPAROUND_HORIZONTAL:
                {
                if (rc.left() < wrapRect.left()) {
                    rc.setLeft(wrapRect.left());
                }
                if (rc.right() > wrapRect.right()) {
                    rc.setRight(wrapRect.right());
                }
                return rc;
                }
            case WRAPAROUND_VERTICAL:
                {
                if (rc.top() < wrapRect.top()) {
                    rc.setTop(wrapRect.top());
                }
                if (rc.bottom() > wrapRect.bottom()) {
                    rc.setBottom(wrapRect.bottom());
                }
                return rc;
                }
            default /*WRAPAROUND_BOTH*/:
                return rc & wrapRect;
        }
    }

    static inline bool wrapRectContains(const QRect &rc, const QRect &wrapRect, WrapAroundAxis wrapAxis) {
        switch (wrapAxis) {
            case WRAPAROUND_HORIZONTAL:
                return (rc.left() >= wrapRect.left() && rc.right() <= wrapRect.right());
            case WRAPAROUND_VERTICAL:
                return (rc.top() >= wrapRect.top() && rc.bottom() <= wrapRect.bottom());
            default /*WRAPAROUND_BOTH*/:
                return wrapRect.contains(rc);
        }
    }
    static inline bool wrapRectContains(QPoint pt, const QRect &wrapRect, WrapAroundAxis wrapAxis) {
        switch (wrapAxis) {
            case WRAPAROUND_HORIZONTAL:
                return (pt.x() >= wrapRect.left() && pt.x() <= wrapRect.right());
            case WRAPAROUND_VERTICAL:
                return (pt.y() >= wrapRect.top() && pt.y() <= wrapRect.bottom());
            default /*WRAPAROUND_BOTH*/:
                return wrapRect.contains(pt);
        }
    }

    /**
     * Return origins at which we should paint \p rc with crop rect set to \p wrapRect,
     * so that the final image would look "wrapped".
     */
    static inline QVector<QPoint> normalizationOriginsForRect(const QRect &rc, const QRect &wrapRect, WrapAroundAxis wrapAxis) {
        QVector<QPoint> result;

        if (wrapRectContains(rc, wrapRect, wrapAxis)) {
                result.append(rc.topLeft());
        }
        else {
            int x = xToWrappedX(rc.x(), wrapRect, wrapAxis);
            int y = yToWrappedY(rc.y(), wrapRect, wrapAxis);
            int w = wrapAxis != WRAPAROUND_VERTICAL ? qMin(rc.width(), wrapRect.width()) : rc.width();
            int h = wrapAxis != WRAPAROUND_HORIZONTAL ? qMin(rc.height(), wrapRect.height()) : rc.height();

            // we ensure that the top/left of the rect belongs to the
            // visible rectangle
            Q_ASSERT(wrapRectContains(QPoint(x,y), wrapRect, wrapAxis));

            QRect newRect(x, y, w, h);

            if (!clipToWrapRect(newRect, wrapRect, wrapAxis).isEmpty()) {
                result.append(QPoint(x, y)); // tl
            }

            if (wrapAxis != WRAPAROUND_VERTICAL &&
                !clipToWrapRect(newRect.translated(-wrapRect.width(), 0), wrapRect, wrapAxis).isEmpty()) { // tr
                result.append(QPoint(x - wrapRect.width(), y));
            }

            if (wrapAxis != WRAPAROUND_HORIZONTAL &&
                !clipToWrapRect(newRect.translated(0, -wrapRect.height()), wrapRect, wrapAxis).isEmpty()) { // bl
                result.append(QPoint(x, y - wrapRect.height()));
            }

            if (wrapAxis == WRAPAROUND_BOTH &&
                !clipToWrapRect(newRect.translated(-wrapRect.width(), -wrapRect.height()), wrapRect, wrapAxis).isEmpty()) { // br
                result.append(QPoint(x - wrapRect.width(), y - wrapRect.height()));
            }
        }

        return result;
    }

    static QVector<QRect> multiplyWrappedRect(const QRect &rc,
                                              const QRect &wrapRect,
                                              const QRect &limitRect,
                                              WrapAroundAxis wrapAxis) {

        QVector<QRect> result;

        const int firstCol =
            wrapAxis != WRAPAROUND_VERTICAL ? qFloor(qreal(limitRect.x() - wrapRect.x()) / wrapRect.width()) : 0;
        const int firstRow =
            wrapAxis != WRAPAROUND_HORIZONTAL ? qFloor(qreal(limitRect.y() - wrapRect.y()) / wrapRect.height()) : 0;

        const int lastCol =
            wrapAxis != WRAPAROUND_VERTICAL ? qFloor(qreal(limitRect.right() - wrapRect.x()) / wrapRect.width()) : 0;
        const int lastRow =
            wrapAxis != WRAPAROUND_HORIZONTAL ? qFloor(qreal(limitRect.bottom() - wrapRect.y()) / wrapRect.height()) : 0;

        KisWrappedRect wrappedRect(rc, wrapRect, wrapAxis);

        Q_FOREACH (const QRect &rect,  wrappedRect) {
            if (rect.isEmpty()) continue;

            for (int y = firstRow; y <= lastRow; y++) {
                for (int x = firstCol; x <= lastCol; x++) {
                    const QPoint offset(x * wrapRect.width(), y * wrapRect.height());
                    const QRect currentRect =
                            rect.translated(offset + wrapRect.topLeft()) & limitRect;

                    if (!currentRect.isEmpty()) {
                        result << currentRect;
                    }
                }
            }
        }

        return result;
    }

public:

    enum {
        TOPLEFT = 0,
        TOPRIGHT,
        BOTTOMLEFT,
        BOTTOMRIGHT
    };

    KisWrappedRect(const QRect &rc, const QRect &wrapRect, WrapAroundAxis wrapAxis)
        : m_wrapRect(wrapRect),
          m_originalRect(rc)
    {
        if (wrapRectContains(rc, wrapRect, wrapAxis)) {
            append(rc);
        } else {
            int x = xToWrappedX(rc.x(), wrapRect, wrapAxis);
            int y = yToWrappedY(rc.y(), wrapRect, wrapAxis);
            int w = wrapAxis != WRAPAROUND_VERTICAL ? qMin(rc.width(), wrapRect.width()) : rc.width();
            int h = wrapAxis != WRAPAROUND_HORIZONTAL ? qMin(rc.height(), wrapRect.height()) : rc.height();

            // we ensure that the top/left of the rect belongs to the
            // visible rectangle
            Q_ASSERT(wrapRectContains(QPoint(x,y), wrapRect, wrapAxis));

            QRect newRect(x, y, w, h);

            // We add empty QRects here because a "splitRect" is expected to contain exactly 4 rects.
            // Functions such as KisPaintDeviceWrappedStrategy::readBytes() and
            // KisWrappedLineIteratorBase() will not work properly (read: crash) otherwise.
            append(clipToWrapRect(newRect, wrapRect, wrapAxis)); // tl
            append(wrapAxis != WRAPAROUND_VERTICAL ?
                clipToWrapRect(newRect.translated(-wrapRect.width(), 0), wrapRect, wrapAxis) : QRect()); // tr
            append(wrapAxis != WRAPAROUND_HORIZONTAL ?
                clipToWrapRect(newRect.translated(0, -wrapRect.height()), wrapRect, wrapAxis) : QRect()); // bl
            append(wrapAxis == WRAPAROUND_BOTH ?
                clipToWrapRect(newRect.translated(-wrapRect.width(), -wrapRect.height()), wrapRect, wrapAxis) : QRect()); // br
        }
    }

    bool isSplit() const {
        int size = this->size();

        // we can either split or not split only
        Q_ASSERT(size == 1 || size == 4);

        return size > 1;
    }

    QRect topLeft() const {
        return this->at(TOPLEFT);
    }

    QRect topRight() const {
        return this->at(TOPRIGHT);
    }

    QRect bottomLeft() const {
        return this->at(BOTTOMLEFT);
    }

    QRect bottomRight() const {
        return this->at(BOTTOMRIGHT);
    }

    QRect wrapRect() const {
        return m_wrapRect;
    }

    QRect originalRect() const {
        return m_originalRect;
    }

private:
    QRect m_wrapRect;
    QRect m_originalRect;
};

#endif /* __KIS_WRAPPED_RECT_H */
