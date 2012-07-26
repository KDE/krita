/* This file is part of the KDE project
   Copyright (C) 2005 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KEXIDB_PREPAREDSTATEMENT_H
#define KEXIDB_PREPAREDSTATEMENT_H

#include <QVariant>
#include <QStringList>
#include <KSharedPtr>

#include "field.h"

namespace KexiDB
{

class ConnectionInternal;
class TableSchema;
class FieldList;

/*! @short Prepared database command for optimizing sequences of multiple database actions

  Currently INSERT and SELECT statements are supported.
  For example, wher using PreparedStatement for INSERTs,
  you can gain about 30% speedup compared to using multiple
  connection.insertRecord(*tabelSchema, dbRowBuffer).

  To use PreparedStatement, create is using KexiDB::Connection:prepareStatement(),
  providing table schema; set up arguments using operator << ( const QVariant& value );
  and call execute() when ready. PreparedStatement objects are accessed
  using KDE shared pointers, i.e KexiDB::PreparedStatement::Ptr, so you do not need
  to remember about destroying them. However, when underlying Connection object
  is destroyed, PreparedStatement should not be used.

  Let's assume tableSchema contains two columns NUMBER integer and TEXT text.
  Following code inserts 10000 records with random numbers and text strings
  obtained elsewhere using getText(i).
  \code
  bool insertMultiple(KexiDB::Connection &conn, KexiDB::TableSchema& tableSchema)
  {
    KexiDB::PreparedStatement::Ptr prepared = conn.prepareStatement(
      KexiDB::PreparedStatement::InsertStatement, tableSchema);
    for (i=0; i<10000; i++) {
      prepared << rand() << getText(i);
      if (!prepared.execute())
        return false;
      prepared.clearArguments();
    }
    return true;
  }
  \endcode

  If you do not call clearArguments() after every insert, you can insert
  the same value multiple times using execute() what increases efficiency even more.

  Another use case is inserting large objects (BLOBs or CLOBs).
  Depending on database backend, you can avoid escaping BLOBs.
  See KexiFormView::storeData() for example use.
*/
class CALLIGRADB_EXPORT PreparedStatement : public KShared
{
public:
    typedef KSharedPtr<PreparedStatement> Ptr;

    //! Defines type of the prepared statement.
    enum StatementType {
        SelectStatement, //!< SELECT statement will be prepared end executed
        InsertStatement  //!< INSERT statement will be prepared end executed
    };

    //! Creates Prepared statement. In your code use KexiDB::Connection:prepareStatement() instead.
    PreparedStatement(StatementType type, ConnectionInternal& conn, FieldList& fields,
                      const QStringList& where = QStringList());

    virtual ~PreparedStatement();

    //! Appends argument \a value to the statement.
    PreparedStatement& operator<< (const QVariant& value);

    //! Clears arguments of the prepared statement. Usually used after execute()
    void clearArguments();

    /*! Executes the prepared statement. In most cases you will need to clear
     arguments after executing, using clearArguments().
     A number arguments set up for the statement must be the same as a number of fields
     defined in the underlying database table.
     \return false on failure. Detailed error status can be obtained
     from KexiDB::Connection object used to create this statement. */
    virtual bool execute() = 0;

protected:
//! @todo is this portable across backends?
    QByteArray generateStatementString();

    StatementType m_type;
    FieldList *m_fields;
    QList<QVariant> m_args;
    QStringList m_where;
    Field::List* m_whereFields;
};

} //namespace KexiDB

#endif
