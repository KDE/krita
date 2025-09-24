/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISDATABASETRANSACTIONLOCK_H
#define KISDATABASETRANSACTIONLOCK_H

#include <QSqlDatabase>

#include <kritaresources_export.h>

#include <KisAdaptedLock.h>


namespace detail
{
struct KRITARESOURCES_EXPORT KisDatabaseTransactionLockAdapter {

    KisDatabaseTransactionLockAdapter(QSqlDatabase database);

    void lock();
    void unlock();
    void commit();

private:
    QSqlDatabase m_database;
    bool m_transactionStarted{false};
};
} // namespace detail


/**
 * A RAII-style class for managing transactions over a database.
 *
 * When the lock is created (and locked) a transaction is started. If the
 * lock is destroyed before the commit() is explicitly called, then the
 * transaction is **rolled back**.
 *
 * To successfully finish the transaction the user should explicitly
 * defuse the lock by calling commit() method.
 *
 * Usage:
 *
 *      \code{.cpp}
 *
 *      /// Start the transaction
 *
 *      KisDatabaseTransactionLock transactionLock(database);
 *
 *      QSqlQuery q(sql);
 *      if (!q.exec()) {
 *
 *          /// on leaving the scope transactionLock will automatically
 *          /// rollback the transaction
 *
 *          return false;
 *      }
 *
 *      /// explicitly commit the transaction when we know
 *      /// everything is fine
 *
 *      transactionLock.commit();
 *
 *      \endcode
 *
 * NOTE:
 *
 * As a rule of thumb, always wrap the actions performing multiple
 * database-modifying steps into a transaction. It will perform
 * much (~10 times) faster in SQLite, because otherwise SQLite will
 * implicitly wrap every single action into its own transaction and
 * it will be slow.
 *
 */
class KRITARESOURCES_EXPORT KisDatabaseTransactionLock
    : public KisAdaptedLock<detail::KisDatabaseTransactionLockAdapter>
{
public:
    using BaseClass = KisAdaptedLock<detail::KisDatabaseTransactionLockAdapter>;
    using BaseClass::BaseClass;
    using BaseClass::commit;

    inline void rollback() {
        unlock();
    }
};


#endif /* KISDATABASETRANSACTIONLOCK_H */
