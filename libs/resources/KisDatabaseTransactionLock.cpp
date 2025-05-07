/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDatabaseTransactionLock.h"

#include <QDebug>
#include <QSqlError>

#include <kis_assert.h>


namespace detail
{

KisDatabaseTransactionLockAdapter::KisDatabaseTransactionLockAdapter(QSqlDatabase database)
    : m_database(database)
{
}

void KisDatabaseTransactionLockAdapter::lock()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!m_transactionStarted);
    if (!m_database.transaction()) {
        qWarning() << "WARNING: Failed to start a transaction:" << m_database.lastError().text();
    } else {
        m_transactionStarted = true;
    }
}

void KisDatabaseTransactionLockAdapter::unlock()
{
    if (!m_transactionStarted)
        return;

    if (!m_database.rollback()) {
        qWarning() << "WARNING: Failed to rollback a transaction:" << m_database.lastError().text();
    }

    m_transactionStarted = false;
}

void KisDatabaseTransactionLockAdapter::commit()
{
    if (!m_transactionStarted)
        return;

    if (!m_database.commit()) {
        qWarning() << "WARNING: Failed to commit a transaction:" << m_database.lastError().text();
    }

    m_transactionStarted = false;
}

} // namespace detail
