/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisTemporaryResourceStorageLock.h"

#include <kis_assert.h>
#include <KisResourceLocator.h>

KisTemporaryResourceStorageLockAdapter::KisTemporaryResourceStorageLockAdapter(const QString &temporaryStorageLocationTemplate)
    : m_temporaryStorageLocationTemplate(temporaryStorageLocationTemplate)
{
}

void KisTemporaryResourceStorageLockAdapter::lock()
{
    int counter = 0;
    QString storageLocation = m_temporaryStorageLocationTemplate;

    while (KisResourceLocator::instance()->hasStorage(storageLocation)) {
        storageLocation = QString("%1_%2").arg(m_temporaryStorageLocationTemplate).arg(counter++);
    }

    KisResourceStorageSP newStorage(new KisResourceStorage(storageLocation, KisResourceStorage::StorageType::Memory));
    KisResourceLocator::instance()->addStorage(storageLocation, newStorage);

    m_temporaryStorageLocation = storageLocation;
}

void KisTemporaryResourceStorageLockAdapter::unlock()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!m_temporaryStorageLocation.isEmpty());
    KisResourceLocator::instance()->removeStorage(m_temporaryStorageLocation);
    m_temporaryStorageLocation.clear();
}

QString KisTemporaryResourceStorageLockAdapter::storageLocation() const
{
    return m_temporaryStorageLocation;
}
