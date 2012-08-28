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

#include "transaction.h"
#include "connection.h"

#include <kdebug.h>

#include <assert.h>

//remove debug
#undef KexiDBDbg
#define KexiDBDbg if (0) kDebug()

using namespace KexiDB;

//helper for debugging
CALLIGRADB_EXPORT int Transaction::globalcount = 0;
CALLIGRADB_EXPORT int Transaction::globalCount()
{
    return Transaction::globalcount;
}
CALLIGRADB_EXPORT int TransactionData::globalcount = 0;
CALLIGRADB_EXPORT int TransactionData::globalCount()
{
    return TransactionData::globalcount;
}

TransactionData::TransactionData(Connection *conn)
        : m_conn(conn)
        , m_active(true)
        , refcount(1)
{
    assert(conn);
    Transaction::globalcount++; //because refcount(1) init.
    TransactionData::globalcount++;
    KexiDBDbg << "-- TransactionData::globalcount == " << TransactionData::globalcount;
}

TransactionData::~TransactionData()
{
    TransactionData::globalcount--;
    KexiDBDbg << "-- TransactionData::globalcount == " << TransactionData::globalcount;
}

//---------------------------------------------------

// not needed // const Transaction Transaction::null;

Transaction::Transaction()
        : QObject(0)
        , m_data(0)
{
// setObjectName("kexidb_transaction");
}

Transaction::Transaction(const Transaction& trans)
        : QObject(0)
        , m_data(trans.m_data)
{
// setObjectName("kexidb_transaction");
    if (m_data) {
        m_data->refcount++;
        Transaction::globalcount++;
    }
}

Transaction::~Transaction()
{
    if (m_data) {
        m_data->refcount--;
        Transaction::globalcount--;
        KexiDBDbg << "~Transaction(): m_data->refcount==" << m_data->refcount;
        if (m_data->refcount == 0)
            delete m_data;
    } else {
        KexiDBDbg << "~Transaction(): null";
    }
    KexiDBDbg << "-- Transaction::globalcount == " << Transaction::globalcount;
}

Transaction& Transaction::operator=(const Transaction & trans)
{
    if (this != &trans) {
        if (m_data) {
            m_data->refcount--;
            Transaction::globalcount--;
            KexiDBDbg << "Transaction::operator=: m_data->refcount==" << m_data->refcount;
            if (m_data->refcount == 0)
                delete m_data;
        }
        m_data = trans.m_data;
        if (m_data) {
            m_data->refcount++;
            Transaction::globalcount++;
        }
    }
    return *this;
}

bool Transaction::operator==(const Transaction& trans) const
{
    return m_data == trans.m_data;
}

Connection* Transaction::connection() const
{
    return m_data ? m_data->m_conn : 0;
}

bool Transaction::active() const
{
    return m_data && m_data->m_active;
}

bool Transaction::isNull() const
{
    return m_data == 0;
}

//---------------------------------------------------

TransactionGuard::TransactionGuard(Connection& conn)
        : m_trans(conn.beginTransaction())
        , m_doNothing(false)
{
}

TransactionGuard::TransactionGuard(const Transaction& trans)
        : m_trans(trans)
        , m_doNothing(false)
{
}

TransactionGuard::TransactionGuard()
        : m_doNothing(false)
{
}

TransactionGuard::~TransactionGuard()
{
    if (!m_doNothing && m_trans.active() && m_trans.connection())
        m_trans.connection()->rollbackTransaction(m_trans);
}

bool TransactionGuard::commit()
{
    if (m_trans.active() && m_trans.connection()) {
        return m_trans.connection()->commitTransaction(m_trans);
    }
    return false;
}

void TransactionGuard::doNothing()
{
    m_doNothing = true;
}

