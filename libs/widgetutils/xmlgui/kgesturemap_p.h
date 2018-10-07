/* This file is part of the KDE libraries
    Copyright (C) 2006,2007 Andreas Hartmetz (ahartmetz@gmail.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KGESTUREMAP_H
#define KGESTUREMAP_H

#include <QObject>
#include <QHash>
#include <QTimer>
#include <QPolygon>

#include "kgesture_p.h"

class QApplication;
class QAction;
class QEvent;

class KRITAWIDGETUTILS_EXPORT KGestureMap : public QObject
{
    Q_OBJECT
public:
    static KGestureMap *self();

    bool eventFilter(QObject *obj, QEvent *e) override;
    void setShapeGesture(QAction *kact, const KShapeGesture &gesture);
    void setRockerGesture(QAction *kact, const KRockerGesture &gesture);
    void setDefaultShapeGesture(QAction *kact, const KShapeGesture &gesture);
    void setDefaultRockerGesture(QAction *kact, const KRockerGesture &gesture);
    /**
     * This method will remove all gestures defined for a given action
     */
    void removeAllGestures(QAction *kact);
    QAction *findAction(const KShapeGesture &gesture) const;
    QAction *findAction(const KRockerGesture &gesture) const;
    KShapeGesture shapeGesture(const QAction *kact) const;
    KShapeGesture defaultShapeGesture(const QAction *kact) const;
    KRockerGesture rockerGesture(const QAction *kact) const;
    KRockerGesture defaultRockerGesture(const QAction *kact) const;

private Q_SLOTS:
    void stopAcquisition();

private:
    friend class KGestureMapContainer;
    KGestureMap();
    ~KGestureMap() override;

    friend class KApplicationPrivate;
    //intended to be used at application initialization
    void installEventFilterOnMe(QApplication *app);

    inline int bitCount(int n);
    void handleAction(QAction *kact);
    void matchShapeGesture();

    //this is an internal class so don't bother with a d-pointer
    typedef QHash< KShapeGesture, QAction * > ShapeGestureHash;
    typedef QHash< KRockerGesture, QAction * > RockerGestureHash;
    ShapeGestureHash m_shapeGestures;
    ShapeGestureHash m_defaultShapeGestures;
    RockerGestureHash m_rockerGestures;
    RockerGestureHash m_defaultRockerGestures;
    QPolygon m_points;
    QTimer m_gestureTimeout;
    bool m_acquiring;

    KShapeGesture m_shapeGesture;
    KRockerGesture m_rockerGesture;
};

#endif //KGESTUREMAP_H
