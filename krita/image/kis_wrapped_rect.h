/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_WRAPPED_RECT_H
#define __KIS_WRAPPED_RECT_H

#include <QVector>
#include <QRect>


struct KisWrappedRect : public QVector<QRect> {
    static inline int xToWrappedX(int x, const QRect &wrapRect) {
        x = (x - wrapRect.x()) % wrapRect.width();
        if (x < 0) x += wrapRect.width();
        return x;
    }

    static inline int yToWrappedY(int y, const QRect &wrapRect) {
        y = (y - wrapRect.y()) % wrapRect.height();
        if (y < 0) y += wrapRect.height();
        return y;
    }

    static inline QPoint ptToWrappedPt(QPoint pt, const QRect &wrapRect) {
        pt.rx() = xToWrappedX(pt.x(), wrapRect);
        pt.ry() = yToWrappedY(pt.y(), wrapRect);
        return pt;
    }

    enum {
        TOPLEFT = 0,
        TOPRIGHT,
        BOTTOMLEFT,
        BOTTOMRIGHT
    };

    KisWrappedRect(const QRect &rc, const QRect &wrapRect)
        : m_wrapRect(wrapRect),
          m_originalRect(rc)
    {
        if (wrapRect.contains(rc)) {
            append(rc);
        } else {
            int x = xToWrappedX(rc.x(), wrapRect);
            int y = yToWrappedY(rc.y(), wrapRect);
            int w = qMin(rc.width(), wrapRect.width());
            int h = qMin(rc.height(), wrapRect.height());

            // we ensure that the topleft of the rect belongs to the
            // visible rectangle
            Q_ASSERT(x >= 0 && x < wrapRect.width());
            Q_ASSERT(y >= 0 && y < wrapRect.height());

            QRect newRect(x, y, w, h);

            append(newRect & wrapRect); // tl
            append(newRect.translated(-wrapRect.width(), 0) & wrapRect); // tr
            append(newRect.translated(0, -wrapRect.height()) & wrapRect); // bl
            append(newRect.translated(-wrapRect.width(), -wrapRect.height()) & wrapRect); // br
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
