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

#ifndef KISUPGRADETOWRITELOCKER_H
#define KISUPGRADETOWRITELOCKER_H

#include <QReadLocker>
#include <QReadWriteLock>

/**
 * @brief The KisUpgradeToWriteLocker class is use for RAII style unlocking
 * the read lock and then locking the lock for write. We basically "upgrade"
 * the lock to a write one.
 *
 * WARNING: during the *upgrade* the lock passes the "unlocked" state, so
 *          all the protected data you acquired during the "read" phase might
 *          have become invalidated!
 */
class KisUpgradeToWriteLocker
{
public:
    KisUpgradeToWriteLocker(QReadLocker *locker)
        : m_locker(locker)
    {
        m_locker->unlock();
        m_locker->readWriteLock()->lockForWrite();
    }

    ~KisUpgradeToWriteLocker() {
        m_locker->readWriteLock()->unlock();
        m_locker->relock();
    }

private:
    QReadLocker *m_locker;
};



#endif // KISUPGRADETOWRITELOCKER_H
