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

#ifndef __KIS_SIGNALS_BLOCKER_H
#define __KIS_SIGNALS_BLOCKER_H

#include <QObject>
#include <QVector>

/**
 * Block QObject's signals in a safe and sane way.
 *
 * Avoid using direct calls to QObject::blockSignals(bool),
 * because:
 *
 * 1) They are not safe. One beautifully sunny day someone (it might
 *    easily be you yourself) will forget about these call and will put
 *    a 'return' statement somewhere among the lines. Surely this is
 *    not what you expect to happen.
 *
 * 2) Two lines of blocking for every line of access can easily make
 *    the code unreadable.
 */

class KisSignalsBlocker
{
public:
    /**
     * Six should be enough for all usage cases! (c)
     */
    KisSignalsBlocker(QObject *o1,
                      QObject *o2,
                      QObject *o3 = 0,
                      QObject *o4 = 0,
                      QObject *o5 = 0,
                      QObject *o6 = 0)
    {
        if (o1) addObject(o1);
        if (o2) addObject(o2);
        if (o3) addObject(o3);
        if (o4) addObject(o4);
        if (o5) addObject(o5);
        if (o6) addObject(o6);

        blockObjects();
    }

    KisSignalsBlocker(QObject *object)
    {
        addObject(object);
        blockObjects();
    }

    ~KisSignalsBlocker()
    {
        QVector<QObject*>::iterator it = m_objects.end();
        QVector<QObject*>::iterator begin = m_objects.begin();

        while (it != begin) {
            --it;
            (*it)->blockSignals(false);
        }
    }

private:
    void blockObjects() {
        Q_FOREACH (QObject *object, m_objects) {
            object->blockSignals(true);
        }
    }

    inline void addObject(QObject *object) {
        m_objects.append(object);
    }

private:
    Q_DISABLE_COPY(KisSignalsBlocker)

private:
    QVector<QObject*> m_objects;
};

#endif /* __KIS_SIGNALS_BLOCKER_H */
