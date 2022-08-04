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
    static inline int xToWrappedX(int x, const QRect &wrapRect, int wrapAxis) {
        if (wrapAxis == 2) { // vertical only
            return x;
        }
        x = (x - wrapRect.x()) % wrapRect.width();
        if (x < 0) x += wrapRect.width();
        return x;
    }

    static inline int yToWrappedY(int y, const QRect &wrapRect, int wrapAxis) {
        if (wrapAxis == 1) { // horizontal only
            return y;
        }
        y = (y - wrapRect.y()) % wrapRect.height();
        if (y < 0) y += wrapRect.height();
        return y;
    }

    static inline QPoint ptToWrappedPt(QPoint pt, const QRect &wrapRect, int wrapAxis) {
        pt.rx() = xToWrappedX(pt.x(), wrapRect, wrapAxis);
        pt.ry() = yToWrappedY(pt.y(), wrapRect, wrapAxis);
        return pt;
    }

    static inline QRect clipToWrapRect(QRect rc, const QRect &wrapRect, int wrapAxis) {
        switch (wrapAxis) {
            case 1 /*horizontal only*/:
                {
                if (rc.left() < wrapRect.left()) {
                    rc.setLeft(wrapRect.left());
                }
                if (rc.right() > wrapRect.right()) {
                    rc.setRight(wrapRect.right());
                }
                return rc;
                }
            case 2 /*vertical only*/:
                {
                if (rc.top() < wrapRect.top()) {
                    rc.setTop(wrapRect.top());
                }
                if (rc.bottom() > wrapRect.bottom()) {
                    rc.setBottom(wrapRect.bottom());
                }
                return rc;
                }
            default /*0: both axes*/:
                return rc & wrapRect;
        }
    }

    static inline bool wrapRectContains(QRect rc, const QRect &wrapRect, int wrapAxis) {
        switch (wrapAxis) {
            case 1 /*horizontal only*/:
                {
                int topX = rc.x();
                int bottomX = topX + rc.width();
                return (topX >= 0 && topX < wrapRect.width() &&
                        bottomX >= 0 && bottomX < wrapRect.width());
                }
            case 2 /*vertical only*/:
                {
                int topY = rc.y();
                int bottomY = topY + rc.height();
                return (topY >= 0 && topY < wrapRect.height() &&
                        bottomY >= 0 && bottomY < wrapRect.height());
                }
            default /*0: both axes*/:
                return wrapRect.contains(rc);
        }
    }

    /**
     * Return origins at which we should paint \p rc with crop rect set to \p wrapRect,
     * so that the final image would look "wrapped".
     */
    static inline QVector<QPoint> normalizationOriginsForRect(const QRect &rc, const QRect &wrapRect, int wrapAxis) {
        QVector<QPoint> result;

        if (wrapRectContains(rc, wrapRect, wrapAxis)) {
                result.append(rc.topLeft());
        }
        else {
            int x = xToWrappedX(rc.x(), wrapRect, wrapAxis);
            int y = yToWrappedY(rc.y(), wrapRect, wrapAxis);
            int w = wrapAxis != 2 ? qMin(rc.width(), wrapRect.width()) : rc.width();
            int h = wrapAxis != 1 ? qMin(rc.height(), wrapRect.height()) : rc.height();

            // we ensure that the top/left of the rect belongs to the
            // visible rectangle
            if (wrapAxis != 2) { // if not vertical only
                Q_ASSERT(x >= 0 && x < wrapRect.width());
            }
            if (wrapAxis != 1) { // if not horizontal only
                Q_ASSERT(y >= 0 && y < wrapRect.height());
            }

            QRect newRect(x, y, w, h);

            if (!clipToWrapRect(newRect, wrapRect, wrapAxis).isEmpty()) {
                result.append(QPoint(x, y)); // tl
            }

            if (wrapAxis != 2 && !clipToWrapRect(newRect.translated(-wrapRect.width(), 0), wrapRect, wrapAxis).isEmpty()) { // tr
                result.append(QPoint(x - wrapRect.width(), y));
            }

            if (wrapAxis != 1 && !clipToWrapRect(newRect.translated(0, -wrapRect.height()), wrapRect, wrapAxis).isEmpty()) { // bl
                result.append(QPoint(x, y - wrapRect.height()));
            }

            if (wrapAxis == 0 && !clipToWrapRect(newRect.translated(-wrapRect.width(), -wrapRect.height()), wrapRect, wrapAxis).isEmpty()) { // br
                result.append(QPoint(x - wrapRect.width(), y - wrapRect.height()));
            }
        }

        return result;
    }

    static QVector<QRect> multiplyWrappedRect(const QRect &rc,
                                              const QRect &wrapRect,
                                              const QRect &limitRect,
                                              int wrapAxis) {

        QVector<QRect> result;

        const int firstCol = qFloor(qreal(limitRect.x() - wrapRect.x()) / wrapRect.width());
        const int firstRow = qFloor(qreal(limitRect.y() - wrapRect.y()) / wrapRect.height());

        const int lastCol = qFloor(qreal(limitRect.right() - wrapRect.x()) / wrapRect.width());
        const int lastRow = qFloor(qreal(limitRect.bottom() - wrapRect.y()) / wrapRect.height());

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

    KisWrappedRect(const QRect &rc, const QRect &wrapRect, int wrapAxis)
        : m_wrapRect(wrapRect),
          m_originalRect(rc)
    {
        if (wrapRectContains(rc, wrapRect, wrapAxis)) {
            append(rc);
        } else {
            int x = xToWrappedX(rc.x(), wrapRect, wrapAxis);
            int y = yToWrappedY(rc.y(), wrapRect, wrapAxis);
            int w = wrapAxis != 2 ? qMin(rc.width(), wrapRect.width()) : rc.width();
            int h = wrapAxis != 1 ? qMin(rc.height(), wrapRect.height()) : rc.height();

            // we ensure that the top/left of the rect belongs to the
            // visible rectangle
            if (wrapAxis != 2) { // if not vertical only
                Q_ASSERT(x >= 0 && x < wrapRect.width());
            }
            if (wrapAxis != 1) { // if not horizontal only
                Q_ASSERT(y >= 0 && y < wrapRect.height());
            }

            QRect newRect(x, y, w, h);

            append(clipToWrapRect(newRect, wrapRect, wrapAxis)); // tl
            append(wrapAxis != 2 ?
                clipToWrapRect(newRect.translated(-wrapRect.width(), 0), wrapRect, wrapAxis) : QRect()); // tr
            append(wrapAxis != 1 ?
                clipToWrapRect(newRect.translated(0, -wrapRect.height()), wrapRect, wrapAxis) : QRect()); // bl
            append(wrapAxis == 0 ?
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
