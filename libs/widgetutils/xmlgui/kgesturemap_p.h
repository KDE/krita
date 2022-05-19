/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2007 Andreas Hartmetz (ahartmetz@gmail.com)

    SPDX-License-Identifier: LGPL-2.0-or-later
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

class KRITAWIDGETUTILS_EXPORT KisKGestureMap : public QObject
{
    Q_OBJECT
public:
    static KisKGestureMap *self();

    bool eventFilter(QObject *obj, QEvent *e) override;
    void setShapeGesture(QAction *kact, const KisKShapeGesture &gesture);
    void setRockerGesture(QAction *kact, const KisKRockerGesture &gesture);
    void setDefaultShapeGesture(QAction *kact, const KisKShapeGesture &gesture);
    void setDefaultRockerGesture(QAction *kact, const KisKRockerGesture &gesture);
    /**
     * This method will remove all gestures defined for a given action
     */
    void removeAllGestures(QAction *kact);
    QAction *findAction(const KisKShapeGesture &gesture) const;
    QAction *findAction(const KisKRockerGesture &gesture) const;
    KisKShapeGesture shapeGesture(const QAction *kact) const;
    KisKShapeGesture defaultShapeGesture(const QAction *kact) const;
    KisKRockerGesture rockerGesture(const QAction *kact) const;
    KisKRockerGesture defaultRockerGesture(const QAction *kact) const;

private Q_SLOTS:
    void stopAcquisition();

private:
    friend class KisKGestureMapContainer;
    KisKGestureMap();
    ~KisKGestureMap() override;

    friend class KApplicationPrivate;
    //intended to be used at application initialization
    void installEventFilterOnMe(QApplication *app);

    inline int bitCount(int n);
    void handleAction(QAction *kact);
    void matchShapeGesture();

    //this is an internal class so don't bother with a d-pointer
    typedef QHash< KisKShapeGesture, QAction * > ShapeGestureHash;
    typedef QHash< KisKRockerGesture, QAction * > RockerGestureHash;
    ShapeGestureHash m_shapeGestures;
    ShapeGestureHash m_defaultShapeGestures;
    RockerGestureHash m_rockerGestures;
    RockerGestureHash m_defaultRockerGestures;
    QPolygon m_points;
    QTimer m_gestureTimeout;
    bool m_acquiring {false};

    KisKShapeGesture m_shapeGesture;
    KisKRockerGesture m_rockerGesture;
};

#endif //KGESTUREMAP_H
