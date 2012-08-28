/* This file is part of the KDE project
   Copyright (C) 2003, 2006 Jarosław Staniek <staniek@kde.org>

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

#ifndef KEXIDB_ROWEDITBUFFER_H
#define KEXIDB_ROWEDITBUFFER_H

#include <QMap>

#include "field.h"
#include "queryschema.h"

namespace KexiDB
{

/*!  @short provides data for single edited database row
  KexiDB::RowEditBuffer provides data for single edited row,
  needed to perform update at the database backend.
  Its advantage over pasing e.g. KexiDB::FieldList object is that
  EditBuffer contains only changed values.

  EditBuffer offers two modes: db-aware and not-db-aware.
  Db-aware buffer addresses a field using references to QueryColumnInfo object,
  while not-db-aware buffer addresses a field using its name.

  Example usage of not-db-aware buffer:
  <code>
  QuerySchema *query = .....
  EditBuffer buf;
  buf.insert("name", "Joe");
  buf.insert("surname", "Black");
  buf.at("name"); //returns "Joe"
  buf.at("surname"); //returns "Black"
  buf.at(query->field("surname")); //returns "Black" too
  // Now you can use buf to add or edit records using
  // KexiDB::Connection::updateRow(), KexiDB::Connection::insertRow()
  </code>

  Example usage of db-aware buffer:
  <code>
  QuerySchema *query = .....
  QueryColumnInfo *ci1 = ....... //e.g. can be obtained from QueryScehma::fieldsExpanded()
  QueryColumnInfo *ci2 = .......
  EditBuffer buf;
  buf.insert(*ci1, "Joe");
  buf.insert(*ci2, "Black");
  buf.at(*ci1); //returns "Joe"
  buf.at(*ci2); //returns "Black"
  // Now you can use buf to add or edit records using
  // KexiDB::Connection::updateRow(), KexiDB::Connection::insertRow()
  </code>

  You can use QMap::clear() to clear buffer contents,
  QMap::isEmpty() to see if buffer is empty.
  For more, see QMap documentation.

  Notes: added fields should come from the same (common) QuerySchema object.
  However, this isn't checked at QValue& EditBuffer::operator[]( const Field& f ) level.
*/
class CALLIGRADB_EXPORT RowEditBuffer
{
public:
    typedef QMap<QString, QVariant> SimpleMap;
    typedef QMap<QueryColumnInfo*, QVariant> DBMap;

    RowEditBuffer(bool dbAwareBuffer);
    ~RowEditBuffer();

    inline bool isDBAware() const {
        return m_dbBuffer != 0;
    }

    void clear();

    bool isEmpty() const;

    //! Inserts value \a val for db-aware buffer's column \a ci
    inline void insert(QueryColumnInfo& ci, QVariant &val) {
        if (m_dbBuffer) {
            m_dbBuffer->insert(&ci, val);
            m_defaultValuesDbBuffer->remove(&ci);
        }
    }

    //! Inserts value \a val for not-db-aware buffer's column \a fname
    inline void insert(const QString& fname, QVariant &val) {
        if (m_simpleBuffer) m_simpleBuffer->insert(fname, val);
    }

    //! Removes value from db-aware buffer's column \a ci
    void removeAt(QueryColumnInfo& ci);

    //! Removes value from not-db-aware buffer's column \a fname
    void removeAt(Field& f);

    //! Removes value from not-db-aware buffer's column \a fname
    void removeAt(const QString& fname);

    /*! Useful only for db-aware buffer. \return value for column \a ci
     If there is no value assigned for the buffer, this method tries to remember and return
     default value obtained from \a ci if \a useDefaultValueIfPossible is true.
     Note that if the column is declared as unique (especially: primary key),
     default value will not be used. */
    const QVariant* at(QueryColumnInfo& ci, bool useDefaultValueIfPossible = true) const;

    //! Useful only for not-db-aware buffer. \return value for field \a f
    const QVariant* at(Field& f) const;

    //! Useful only for not-db-aware buffer. \return value for field \a fname
    const QVariant* at(const QString& fname) const;

    //! Useful only for db-aware buffer: \return true if the value available as
    //! at( ci ) is obtained from column's default value
    inline bool hasDefaultValueAt(QueryColumnInfo& ci) const {
        return m_defaultValuesDbBuffer->contains(&ci) && (*m_defaultValuesDbBuffer)[ &ci ];
    }

    inline const SimpleMap simpleBuffer() {
        return *m_simpleBuffer;
    }
    inline const DBMap dbBuffer() {
        return *m_dbBuffer;
    }

    //! For debugging purposes
    void debug();

protected:
    SimpleMap *m_simpleBuffer;
    SimpleMap::ConstIterator *m_simpleBufferIt;
    DBMap *m_dbBuffer;
    DBMap::Iterator *m_dbBufferIt;
    QMap<QueryColumnInfo*, bool> *m_defaultValuesDbBuffer;
    QMap<QueryColumnInfo*, bool>::ConstIterator *m_defaultValuesDbBufferIt;
};

} //namespace KexiDB

#endif
