/* This file is part of the KDE project
   Copyright (C) 2003 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KEXIDB_TRANSACTION_H
#define KEXIDB_TRANSACTION_H

#include <QPointer>

#include "calligradb_export.h"

namespace KexiDB
{

class Connection;

/*! Internal prototype for storing transaction handles for Transaction object.
 Only for driver developers: reimplement this class for driver that
 support transaction handles.
*/
class CALLIGRADB_EXPORT TransactionData
{
public:
    TransactionData(Connection *conn);
    ~TransactionData();

    //helper for debugging
    static int globalcount;
    //helper for debugging
    static int globalCount();

    Connection *m_conn;
    bool m_active : 1;
    uint refcount;
};

//! This class encapsulates transaction handle.
/*! Transaction handle is sql driver-dependent,
  but outside Transaction is visible as universal container
  for any handler implementation.

  Transaction object is value-based, internal data (handle) structure,
  reference-counted.
*/
class CALLIGRADB_EXPORT Transaction : public QObject
{
public:
    /*! Constructs uninitialised (null) transaction.
     Only in Conenction code it can be initialised */
    Transaction();

    //! Copy ctor.
    Transaction(const Transaction& trans);

    virtual ~Transaction();

    Transaction& operator=(const Transaction& trans);

    bool operator==(const Transaction& trans) const;

    Connection* connection() const;

    /*! \return true if transaction is avtive (ie. started)
     Returns false also if transaction is uninitialised (null). */
    bool active() const;

    /*! \return true if transaction is uinitialised (null). */
    bool isNull() const;

    /*! shortcut that offers uinitialised (null) transaction */
    //static const Transaction null;

    //helper for debugging
    static int globalCount();
    static int globalcount;
protected:

    TransactionData *m_data;

    friend class Connection;
};

//! Helper class for using inside methods for given connection.
/*! It can be used in two ways:
  - start new transaction in constructor and rollback on destruction (1st constructor),
  - use already started transaction and rollback on destruction (2nd constructor).
  In any case, if transaction is committed or rolled back outside this TransactionGuard
  object in the meantime, nothing happens on TransactionGuard destruction.
  <code>
  Example usage:
  void myclas::my_method()
  {
    Transaction *transaction = connection->beginTransaction();
    TransactionGuard tg(transaction);
    ...some code that operates inside started transaction...
    if (something)
      return //after return from this code block: tg will call
               //connection->rollbackTransaction() automatically
    if (something_else)
      transaction->commit();
    //for now tg won't do anything because transaction does not exist
  }
  </code>
*/
class CALLIGRADB_EXPORT TransactionGuard
{
public:
    /*! Constructor #1: Starts new transaction constructor for \a connection.
     Started transaction handle is available via transaction().*/
    TransactionGuard(Connection& conn);

    /*! Constructor #2: Uses already started transaction. */
    TransactionGuard(const Transaction& trans);

    /*! Constructor #3: Creates TransactionGuard without transaction assigned.
     setTransaction() can be used later to do so. */
    TransactionGuard();

    /*! Rollbacks not committed transaction. */
    ~TransactionGuard();

    /*! Assigns transaction \a trans to this guard.
     Previously assigned transaction will be unassigned from this guard. */
    void setTransaction(const Transaction& trans) {
        m_trans = trans;
    }

    /*! Comits the guarded transaction.
     It is convenient shortcut to connection->commitTransaction(this->transaction()) */
    bool commit();

    /*! Makes guarded transaction not guarded, so nothing will be performed on guard's desctruction. */
    void doNothing();

    /*! Transaction that are controlled by this guard. */
    const Transaction transaction() const {
        return m_trans;
    }

protected:
    Transaction m_trans;
    bool m_doNothing : 1;
};

} //namespace KexiDB

#endif


