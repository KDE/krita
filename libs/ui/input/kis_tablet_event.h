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

#ifndef KIS_TABLET_EVENT_H
#define KIS_TABLET_EVENT_H

#include <QTabletEvent>

class KisTabletEvent : public QInputEvent
{
public:

    enum ExtraEventType {
        TabletMoveEx = QEvent::User,
        TabletPressEx,
        TabletReleaseEx,

        TouchProximityInEx,
        TouchProximityOutEx,
    };

    enum TabletDevice { NoDevice, Puck, Stylus, Airbrush, FourDMouse,
                        XFreeEraser /*internal*/, RotationStylus };
    enum PointerType { UnknownPointer, Pen, Cursor, Eraser };


    KisTabletEvent(ExtraEventType t, const QPoint &pos, const QPoint &globalPos, const QPointF &hiResGlobalPos,
                   int device, int pointerType, qreal pressure, int xTilt, int yTilt,
                   qreal tangentialPressure, qreal rotation, int z,
                   Qt::KeyboardModifiers keyState, qint64 uniqueID,
                   Qt::MouseButton button, Qt::MouseButtons buttons);
    ~KisTabletEvent() override;

    inline const QPoint &pos() const { return mPos; }
    inline const QPoint &globalPos() const { return mGPos; }
    inline const QPointF &hiResGlobalPos() const { return mHiResGlobalPos; }
    inline int x() const { return mPos.x(); }
    inline int y() const { return mPos.y(); }
    inline int globalX() const { return mGPos.x(); }
    inline int globalY() const { return mGPos.y(); }
    inline qreal hiResGlobalX() const { return mHiResGlobalPos.x(); }
    inline qreal hiResGlobalY() const { return mHiResGlobalPos.y(); }
    inline TabletDevice device() const { return TabletDevice(mDev); }
    inline PointerType pointerType() const { return PointerType(mPointerType); }
    inline qint64 uniqueId() const { return mUnique; }
    inline qreal pressure() const { return mPress; }
    inline int z() const { return mZ; }
    inline qreal tangentialPressure() const { return mTangential; }
    inline qreal rotation() const { return mRot; }
    inline int xTilt() const { return mXT; }
    inline int yTilt() const { return mYT; }
    inline Qt::MouseButton button() const { return mouseButton; }
    inline Qt::MouseButtons buttons() const { return mouseButtons; }

    inline QEvent::Type getMouseEventType() const {
        return
            (ExtraEventType) type() == TabletMoveEx ? MouseMove :
            (ExtraEventType) type() == TabletPressEx ? MouseButtonPress :
            (ExtraEventType) type() == TabletReleaseEx ? MouseButtonRelease :
            QEvent::None;
    }

    inline QMouseEvent toQMouseEvent() const {
        QEvent::Type t = getMouseEventType();

        return QMouseEvent(t, pos(), globalPos(),
                           button(), buttons(), modifiers());
    }

    inline QTabletEvent toQTabletEvent() const {
        QEvent::Type t =
            (ExtraEventType) type() == TabletMoveEx ? TabletMove :
            (ExtraEventType) type() == TabletPressEx ? TabletPress :
            (ExtraEventType) type() == TabletReleaseEx ? TabletRelease :
            QEvent::None;

        return QTabletEvent(t, pos(), globalPos(),
                            device(), pointerType(),
                            pressure(), xTilt(), yTilt(), tangentialPressure(),
                            rotation(), z(), modifiers(), uniqueId());
    }

protected:
    QPoint mPos, mGPos;
    QPointF mHiResGlobalPos;
    int mDev, mPointerType, mXT, mYT, mZ;
    qreal mPress, mTangential, mRot;
    qint64 mUnique;
    Qt::MouseButton mouseButton;
    Qt::MouseButtons mouseButtons;


};

#endif // KIS_TABLET_EVENT_H
