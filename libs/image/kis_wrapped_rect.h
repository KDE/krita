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

struct KisWrappedRect : public QVector<QRect> {
    static inline int xToWrappedX(int x, const QRect &wrapRect, int wrapAxis = 0) {
        if (wrapAxis == 2 && (x > wrapRect.width() || x < 0)) {
            // outside vertical-only bounds
            return -1;
        }
        x = (x - wrapRect.x()) % wrapRect.width();
        if (x < 0) x += wrapRect.width();
        return x;
    }

    static inline int yToWrappedY(int y, const QRect &wrapRect, int wrapAxis = 0) {
        if (wrapAxis == 1 && (y > wrapRect.height() || y < 0)) {
            // outside horizontal-only bounds
            return -1;
        }
        y = (y - wrapRect.y()) % wrapRect.height();
        if (y < 0) y += wrapRect.height();
        return y;
    }

    static inline QPoint ptToWrappedPt(QPoint pt, const QRect &wrapRect, int wrapAxis) {
        if ((wrapAxis == 1 && (pt.y() > wrapRect.height() || pt.y() < 0)) ||
             (wrapAxis == 2 && (pt.x() > wrapRect.width() || pt.x() < 0))) {
            return QPoint(-1, -1);
        }
        pt.rx() = xToWrappedX(pt.x(), wrapRect, wrapAxis);
        pt.ry() = yToWrappedY(pt.y(), wrapRect, wrapAxis);
        return pt;
    }

    /**
     * Return origins at which we should paint \p rc with crop rect set to \p wrapRect,
     * so that the final image would look "wrapped".
     */
    static inline QVector<QPoint> normalizationOriginsForRect(const QRect &rc, const QRect &wrapRect, int wrapAxis) {
        QVector<QPoint> result;

        if (wrapRect.contains(rc)) {
            result.append(rc.topLeft());
        }
        else if (wrapAxis == 0) {   // both axes
            int x = xToWrappedX(rc.x(), wrapRect);
            int y = yToWrappedY(rc.y(), wrapRect);
            int w = qMin(rc.width(), wrapRect.width());
            int h = qMin(rc.height(), wrapRect.height());

            // we ensure that the topleft of the rect belongs to the
            // visible rectangle
            Q_ASSERT(x >= 0 && x < wrapRect.width());
            Q_ASSERT(y >= 0 && y < wrapRect.height());

            QRect newRect(x, y, w, h);

            if (!(newRect & wrapRect).isEmpty()) { // tl
                result.append(QPoint(x, y));
            }

            if (!(newRect.translated(-wrapRect.width(), 0) & wrapRect).isEmpty()) { // tr
                result.append(QPoint(x - wrapRect.width(), y));
            }

            if (!(newRect.translated(0, -wrapRect.height()) & wrapRect).isEmpty()) { // bl
                result.append(QPoint(x, y - wrapRect.height()));
            }

            if (!(newRect.translated(-wrapRect.width(), -wrapRect.height()) & wrapRect).isEmpty()) { // br
                result.append(QPoint(x - wrapRect.width(), y - wrapRect.height()));
            }
        }
        else if (wrapAxis == 1) {    // horizontal only
            int x = xToWrappedX(rc.x(), wrapRect);
            int y = rc.y();
            int w = qMin(rc.width(), wrapRect.width());
            int h = rc.height();

            QRect newRect(x, y, w, h);

            // normal bounds
            if (!(newRect & wrapRect).isEmpty()) {
                result.append(QPoint(x, y));
            }
            // horizontally wrapped bounds
            if (!(newRect.translated(-wrapRect.width(), 0) & wrapRect).isEmpty()) {
                result.append(QPoint(x - wrapRect.width(), y));
            }
        }
        else if (wrapAxis == 2) {  // vertical only
            int x = rc.x();
            int y = yToWrappedY(rc.y(), wrapRect);
            int w = rc.width();
            int h = qMin(rc.height(), wrapRect.height());

            QRect newRect(x, y, w, h);

            // normal bounds
            if (!(newRect & wrapRect).isEmpty()) {
                result.append(QPoint(x, y));
            }

            // vertically wrapped bounds
            if (!(newRect.translated(0, -wrapRect.height()) & wrapRect).isEmpty()) {
                result.append(QPoint(x, y - wrapRect.height()));
            }
        }

        return result;
    }

    static QVector<QRect> multiplyWrappedRect(const QRect &rc,
                                              const QRect &wrapRect,
                                              const QRect &limitRect) {

        QVector<QRect> result;

        const int firstCol = qFloor(qreal(limitRect.x() - wrapRect.x()) / wrapRect.width());
        const int firstRow = qFloor(qreal(limitRect.y() - wrapRect.y()) / wrapRect.height());

        const int lastCol = qFloor(qreal(limitRect.right() - wrapRect.x()) / wrapRect.width());
        const int lastRow = qFloor(qreal(limitRect.bottom() - wrapRect.y()) / wrapRect.height());

        KisWrappedRect wrappedRect(rc, wrapRect);

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

    KisWrappedRect(const QRect &rc, const QRect &wrapRect, int wrapAxis = 0)
        : m_wrapRect(wrapRect),
          m_originalRect(rc)
    {
        if (wrapRect.contains(rc)) {
            append(rc);
        } else {
            int x = xToWrappedX(rc.x(), wrapRect, wrapAxis);
            int y = yToWrappedY(rc.y(), wrapRect, wrapAxis);
            int w = wrapAxis == 2 ? wrapRect.width() : qMin(rc.width(), wrapRect.width());
            int h = wrapAxis == 1 ? wrapRect.height() : qMin(rc.height(), wrapRect.height());

            // we ensure that the topleft of the rect belongs to the
            // visible rectangle
            Q_ASSERT(x >= 0 && x < wrapRect.width());
            Q_ASSERT(y >= 0 && y < wrapRect.height());

            QRect newRect(x, y, w, h);

            append(newRect & wrapRect); // tl
            append(wrapAxis != 2 ? (newRect.translated(-wrapRect.width(), 0) & wrapRect) : QRect()); // tr
            append(wrapAxis != 1 ? (newRect.translated(0, -wrapRect.height()) & wrapRect) : QRect()); // bl
            append(wrapAxis == 0 ? (newRect.translated(-wrapRect.width(), -wrapRect.height()) & wrapRect) : QRect()); // br

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
