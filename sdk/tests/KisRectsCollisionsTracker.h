/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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
