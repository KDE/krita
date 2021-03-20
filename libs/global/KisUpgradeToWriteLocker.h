/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
