/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2007 Andreas Hartmetz (ahartmetz@gmail.com)

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kgesturemap_p.h"

#include <qapplication.h>
#include <QAction>
#include <QActionEvent>
#include <QDebug>

/*
 This is a class for internal use by the KDE libraries only. This class
 may change or go away without notice so don't try to use it in non-kdelibs
 code.
 */

class KGestureMapContainer
{
public:
    KGestureMap gestureMap;
};

Q_GLOBAL_STATIC(KGestureMapContainer, g_instance)

KGestureMap::~KGestureMap()
{
}

KGestureMap *KGestureMap::self()
{
    return &g_instance()->gestureMap;
}

KGestureMap::KGestureMap()
{
    m_gestureTimeout.setSingleShot(true);
    connect(&m_gestureTimeout, SIGNAL(timeout()), this, SLOT(stopAcquisition()));
    //It would be nice to install the filter on demand. Unfortunately,
    //undesired behavior might result due to changing invocation
    //orders of different event filters.
    if (qApp) {
        qApp->installEventFilter(this);
    }
}

void KGestureMap::setShapeGesture(QAction *act, const KShapeGesture &gesture)
{
    if (!gesture.isValid() || !act) {
        return;
    }
    qDebug() << "KGestureMap::addGesture(KShapeGesture ...)";
    if (m_shapeGestures.contains(gesture)) {
        qWarning() << "Replacing an action for a gesture already taken";
    }
    m_shapeGestures.insert(gesture, act);
}

void KGestureMap::setRockerGesture(QAction *act, const KRockerGesture &gesture)
{
    if (!gesture.isValid() || !act) {
        return;
    }
    qDebug() << "KGestureMap::addGesture(KRockerGesture ...)";
    if (m_rockerGestures.contains(gesture)) {
        qWarning() << "Replacing an action for a gesture already taken";
    }
    m_rockerGestures.insert(gesture, act);
}

void KGestureMap::setDefaultShapeGesture(QAction *act, const KShapeGesture &gesture)
{
    if (!gesture.isValid() || !act) {
        return;
    }
    qDebug() << "KGestureMap::addGesture(KShapeGesture ...)";
    if (m_defaultShapeGestures.contains(gesture)) {
        qWarning() << "Replacing an action for a gesture already taken";
    }
    m_defaultShapeGestures.insert(gesture, act);
}

void KGestureMap::setDefaultRockerGesture(QAction *act, const KRockerGesture &gesture)
{
    if (!gesture.isValid() || !act) {
        return;
    }
    qDebug() << "KGestureMap::addGesture(KRockerGesture ...)";
    if (m_defaultRockerGestures.contains(gesture)) {
        qWarning() << "Replacing an action for a gesture already taken";
    }
    m_defaultRockerGestures.insert(gesture, act);
}

void KGestureMap::removeAllGestures(QAction *kact)
{
    KShapeGesture activeGesture;
    ShapeGestureHash::iterator si = m_shapeGestures.begin();
    ShapeGestureHash::iterator send = m_shapeGestures.end();
    for (; si != send; ++si) {
        if (si.value() == kact) {
            m_shapeGestures.remove(si.key());
            break;
        }
    }

    si = m_defaultShapeGestures.begin();
    send = m_defaultShapeGestures.end();
    for (; si != send; ++si) {
        if (si.value() == kact) {
            m_defaultShapeGestures.remove(si.key());
            break;
        }
    }

    RockerGestureHash::iterator ri = m_rockerGestures.begin();
    RockerGestureHash::iterator rend = m_rockerGestures.end();
    for (; ri != rend; ++ri) {
        if (ri.value() == kact) {
            m_rockerGestures.remove(ri.key());
            break;
        }
    }

    ri = m_defaultRockerGestures.begin();
    rend = m_defaultRockerGestures.end();
    for (; ri != rend; ++ri) {
        if (ri.value() == kact) {
            m_defaultRockerGestures.remove(ri.key());
            break;
        }
    }
}

QAction *KGestureMap::findAction(const KShapeGesture &gesture) const
{
    return m_shapeGestures.value(gesture);
}

QAction *KGestureMap::findAction(const KRockerGesture &gesture) const
{
    return m_rockerGestures.value(gesture);
}

void KGestureMap::installEventFilterOnMe(QApplication *app)
{
    app->installEventFilter(this);
}

KShapeGesture KGestureMap::shapeGesture(const QAction *kact) const
{
    KShapeGesture activeGesture;
    ShapeGestureHash::const_iterator it = m_shapeGestures.constBegin();
    ShapeGestureHash::const_iterator end = m_shapeGestures.constEnd();
    for (; it != end; ++it) {
        if (it.value() == kact) {
            activeGesture = it.key();
            break;
        }
    }
    return activeGesture;
}

KShapeGesture KGestureMap::defaultShapeGesture(const QAction *kact) const
{
    KShapeGesture defaultGesture;
    ShapeGestureHash::const_iterator it = m_defaultShapeGestures.constBegin();
    ShapeGestureHash::const_iterator end = m_defaultShapeGestures.constEnd();
    for (; it != end; ++it) {
        if (it.value() == kact) {
            defaultGesture = it.key();
            break;
        }
    }
    return defaultGesture;
}

KRockerGesture KGestureMap::rockerGesture(const QAction *kact) const
{
    KRockerGesture activeGesture;
    RockerGestureHash::const_iterator it = m_rockerGestures.constBegin();
    RockerGestureHash::const_iterator end = m_rockerGestures.constEnd();
    for (; it != end; ++it) {
        if (it.value() == kact) {
            activeGesture = it.key();
            break;
        }
    }
    return activeGesture;
}

KRockerGesture KGestureMap::defaultRockerGesture(const QAction *kact) const
{
    KRockerGesture defaultGesture;
    RockerGestureHash::const_iterator it = m_defaultRockerGestures.constBegin();
    RockerGestureHash::const_iterator end = m_defaultRockerGestures.constEnd();
    for (; it != end; ++it) {
        if (it.value() == kact) {
            defaultGesture = it.key();
            break;
        }
    }
    return defaultGesture;
}

inline int KGestureMap::bitCount(int n)
{
    int count = 0;
    while (n) {
        n &= (n - 1);
        count++;
    }
    return count;
}

void KGestureMap::handleAction(QAction *kact)
{
    if (!kact) {
        return;
    }
    qDebug() << "handleAction";
    //TODO: only activate in the action's context, just like keyboard shortcuts
    kact->trigger();
    return;
}

void KGestureMap::matchShapeGesture()
{
    //TODO: tune and tweak until satisfied with result :)
    m_shapeGesture.setShape(m_points);
    float dist, minDist = 20.0;
    QAction *bestMatch = 0;

    for (QHash<KShapeGesture, QAction *>::const_iterator it = m_shapeGestures.constBegin();
            it != m_shapeGestures.constEnd(); ++it) {
        dist = m_shapeGesture.distance(it.key(), 1000.0);
        if (dist < minDist) {
            minDist = dist;
            bestMatch = it.value();
        }
    }
    handleAction(bestMatch);
}

//slot
void KGestureMap::stopAcquisition()
{
    m_gestureTimeout.stop();
    m_acquiring = false;
}

//TODO: Probably kwin, kded and others should not have a gesture map.
//Maybe making them friends and providing a private "die()" function would work.
/*
 * Act on rocker gestures immediately and collect movement data for evaluation.
 * The decision when to consume and when to relay an event is quite tricky.
 * I decided to only consume clicks that belong to completed rocker gestures.
 * A user might e.g. go back in a browser several times using rocker gestures,
 * thus changing what's under the cursor every time. This might lead to
 * unintended clicks on links where there was free space before.
 */

bool KGestureMap::eventFilter(QObject *obj, QEvent *e)
{
    //disable until it does not interfere with other input any more
    return false;
    Q_UNUSED(obj);
    int type = e->type();

    //catch right-clicks disguised as context menu events. if we ignore a
    //context menu event caused by a right-click, it should get resent
    //as a right-click event, according to documentation.
    //### this is preliminary
    if (type == QEvent::ContextMenu) {
        QContextMenuEvent *cme = static_cast<QContextMenuEvent *>(e);
        if (cme->reason() == QContextMenuEvent::Mouse) {
            cme->ignore();
            return true;
        }
        return false;
    }

    if (type < QEvent::MouseButtonPress || type > QEvent::MouseMove) {
        return false;
    }

    QMouseEvent *me = static_cast<QMouseEvent *>(e);
    if (type == QEvent::MouseButtonPress) {
        int nButtonsDown = bitCount(me->buttons());
        qDebug() << "number of buttons down:" << nButtonsDown;

        //right button down starts gesture acquisition
        if (nButtonsDown == 1 && me->button() == Qt::RightButton) {
            //"startAcquisition()"
            m_acquiring = true;
            m_gestureTimeout.start(4000);
            qDebug() << "========================";
            m_points.clear();
            m_points.append(me->pos());
            return true;
        } else if (nButtonsDown != 2) {
            return false;
        }

        //rocker gestures. do not trigger any movement gestures from now on.
        stopAcquisition();
        int buttonHeld = me->buttons() ^ me->button();
        m_rockerGesture.setButtons(static_cast<Qt::MouseButton>(buttonHeld), me->button());
        QAction *match = m_rockerGestures.value(m_rockerGesture);
        if (!match) {
            return false;
        }
        handleAction(match);
        return true;
    }

    if (m_acquiring) {
        if (type == QEvent::MouseMove) {
            m_points.append(me->pos());
            //abort to avoid using too much memory. 1010 points should be enough
            //for everyone! :)
            //next reallocation of m_points would happen at 1012 items
            if (m_points.size() > 1010) {
                stopAcquisition();
            }
            return true;
        } else if (type == QEvent::MouseButtonRelease && me->button() == Qt::RightButton) {
            stopAcquisition();

            //TODO: pre-selection of gestures by length (optimization), if necessary
            //possibly apply other heuristics
            //then try all remaining gestures for sufficiently small distance
            int dist = 0;
            for (int i = 1; i < m_points.size(); i++) {
                dist += (m_points[i] - m_points[i - 1]).manhattanLength();
                if (dist > 40) {
                    matchShapeGesture();
                    return true;
                }
                //this was probably a small glitch while right-clicking if we get here.
                //TODO: open the context menu or do whatever happens on right-click (how?)
            }
            return false;
        }
    }
    return false;
}

