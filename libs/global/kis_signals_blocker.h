/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
