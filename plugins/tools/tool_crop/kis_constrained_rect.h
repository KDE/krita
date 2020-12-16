/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    ~KisConstrainedRect() override;

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
