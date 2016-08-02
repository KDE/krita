/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_CONSTRAINED_RECT_H
#define __KIS_CONSTRAINED_RECT_H

#include <QObject>
#include <QRect>



class KisConstrainedRect : public QObject
{
    Q_OBJECT

public:
    enum HandleType {
        None = 0,
        UpperLeft,
        UpperRight,
        LowerLeft,
        LowerRight,
        Upper,
        Lower,
        Left,
        Right,
        Inside,
        Creation
    };

public:
    KisConstrainedRect();
    ~KisConstrainedRect();

    void setRectInitial(const QRect &rect);
    void setCropRect(const QRect &cropRect);

    bool centered() const;
    void setCentered(bool value);

    bool canGrow() const;
    void setCanGrow(bool value);

    QRect rect() const;

    qreal ratio() const;

    void moveHandle(HandleType handle, const QPoint &offset, const QRect &oldRect);
    QPointF handleSnapPoint(HandleType handle, const QPointF &cursorPos);

    void setRatio(qreal value);
    void setOffset(const QPoint &offset);
    void setWidth(int value);
    void setHeight(int value);

    bool widthLocked() const;
    void setWidthLocked(bool value);

    bool heightLocked() const;
    void setHeightLocked(bool value);

    bool ratioLocked() const;
    void setRatioLocked(bool value);

    void normalize();

Q_SIGNALS:
    void sigValuesChanged();
    void sigLockValuesChanged();

private:

    int widthFromHeightUnsignedRatio(int height, qreal ratio, int oldWidth) const;
    int heightFromWidthUnsignedRatio(int width, qreal ratio, int oldHeight) const;

    void assignNewSize(const QSize &newSize);
    void storeRatioSafe(const QSize &newSize);
private:
    bool m_centered;
    bool m_canGrow;
    QRect m_rect;
    qreal m_ratio;

    bool m_lockingEnabled;

    bool m_widthLocked;
    bool m_heightLocked;
    bool m_ratioLocked;

    QRect m_cropRect;
};

#endif /* __KIS_CONSTRAINED_RECT_H */
