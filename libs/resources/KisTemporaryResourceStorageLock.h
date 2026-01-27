/*
 *  SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISTEMPORARYRESOURCESTORAGELOCK_H
#define KISTEMPORARYRESOURCESTORAGELOCK_H

#include <kritaresources_export.h>
#include <KisAdaptedLock.h>

#include <QString>


/**
 * A RAII-based locker for creation of a temporary resource storage
 */
class KRITARESOURCES_EXPORT KisTemporaryResourceStorageLockAdapter
{
public:
    KisTemporaryResourceStorageLockAdapter(const QString &temporaryStorageLocationTemplate);

    bool try_lock();
    void lock();
    void unlock();

    QString storageLocation() const;

private:
    QString m_temporaryStorageLocationTemplate;
    QString m_temporaryStorageLocation;
};

class KisTemporaryResourceStorageLock : public KisAdaptedLock<KisTemporaryResourceStorageLockAdapter>
{
public:
    using BaseClass = KisAdaptedLock<KisTemporaryResourceStorageLockAdapter>;
    using BaseClass::BaseClass;

    using BaseClass::storageLocation;
};

#endif // KISTEMPORARYRESOURCESTORAGELOCK_H
