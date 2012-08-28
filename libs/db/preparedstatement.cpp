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

#include "preparedstatement.h"
#include "connection.h"
#include "connection_p.h"
#include <kdebug.h>

using namespace KexiDB;

PreparedStatement::PreparedStatement(StatementType type, ConnectionInternal& conn,
                                     FieldList& fields, const QStringList& where)
        : KShared()
        , m_type(type)
        , m_fields(&fields)
        , m_where(where)
        , m_whereFields(0)
{
    Q_UNUSED(conn);
}

PreparedStatement::~PreparedStatement()
{
    delete m_whereFields;
}

QByteArray PreparedStatement::generateStatementString()
{
    QByteArray s;
    s.reserve(1024);
    if (m_type == SelectStatement) {
//! @todo only tables and trivial queries supported for select...
        s = "SELECT ";
        bool first = true;
        foreach(Field *f, *m_fields->fields()) {
            if (first)
                first = false;
            else
                s.append(", ");
            s.append(f->name().toLatin1());
        }
        if (!m_where.isEmpty())
            s.append(" WHERE ");
        delete m_whereFields;
        m_whereFields = new Field::List();
        first = true;
        foreach(const QString& whereItem, m_where) {
            if (first)
                first = false;
            else
                s.append(" AND ");
            Field *f = m_fields->field(whereItem);
            if (!f) {
                KexiDBWarn << "PreparedStatement::generateStatementString(): no '"
                << whereItem << "' field found";
                continue;
            }
            m_whereFields->append(f);
            s.append(whereItem.toLatin1());
            s.append("=?");
        }
    } else if (m_type == InsertStatement /*&& dynamic_cast<TableSchema*>(m_fields)*/) {
        //! @todo only tables supported for insert; what about views?

        TableSchema *table = m_fields->fieldCount() > 0 ? m_fields->field(0)->table() : 0;
        if (!table)
            return ""; //err

        QByteArray namesList;
        bool first = true;
        const bool allTableFieldsUsed = dynamic_cast<TableSchema*>(m_fields); //we are using a selection of fields only
        foreach(Field* f, *m_fields->fields()) {
            if (first) {
                s.append("?");
                if (!allTableFieldsUsed)
                    namesList = f->name().toLatin1();
                first = false;
            } else {
                s.append(",?");
                if (!allTableFieldsUsed)
                    namesList.append(QByteArray(", ") + f->name().toLatin1());
            }
        }
        s.append(")");
        s.prepend(QByteArray("INSERT INTO ") + table->name().toLatin1()
                  + (allTableFieldsUsed ? "" : (" (" + namesList + ")"))
                  + " VALUES (");
    }
    return s;
}

PreparedStatement& PreparedStatement::operator<< (const QVariant& value)
{
    m_args.append(value);
    return *this;
}

/*bool PreparedStatement::insert()
{
  const bool res = m_conn->drv_prepareStatement(this);
  const bool res = m_conn->drv_insertRecord(this);
  clearArguments();
  return res;
}

bool PreparedStatement::select()
{
  const bool res = m_conn->drv_bindArgumentForPreparedStatement(this, m_args.count()-1);
}*/

void PreparedStatement::clearArguments()
{
    m_args.clear();
}
