/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISRECTSCOLLISIONSTRACKER_H
#define KISRECTSCOLLISIONSTRACKER_H

#include <QList>
#include <QRect>
#include <QMutex>
#include <QMutexLocker>

#include "kis_assert.h"


class KisRectsCollisionsTracker
{
public:

    void startAccessingRect(const QRect &rc) {
        QMutexLocker l(&m_mutex);

        checkUniqueAccessImpl(rc, "start");
        m_rectsInProgress.append(rc);
    }

    void endAccessingRect(const QRect &rc) {
        QMutexLocker l(&m_mutex);
        const bool result = m_rectsInProgress.removeOne(rc);
        KIS_SAFE_ASSERT_RECOVER_NOOP(result);
        checkUniqueAccessImpl(rc, "end");
    }

private:

    bool checkUniqueAccessImpl(const QRect &rect, const QString &tag) {

        Q_FOREACH (const QRect &rc, m_rectsInProgress) {
            if (rc != rect && rect.intersects(rc)) {
                ENTER_FUNCTION() << "FAIL: concurrent access from" << rect << "to" << rc << tag;
                return false;
            }
        }

        return true;
    }

private:
    QList<QRect> m_rectsInProgress;
    QMutex m_mutex;
};

#endif // KISRECTSCOLLISIONSTRACKER_H
