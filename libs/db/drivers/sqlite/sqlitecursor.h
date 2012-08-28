/* This file is part of the KDE project
   Copyright (C) 2003-2006 Jarosław Staniek <staniek@kde.org>

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

#ifndef KEXIDB_SQLITECURSOR_H
#define KEXIDB_SQLITECURSOR_H

#include <QString>

#include <db/cursor.h>
#include <db/connection.h>

namespace KexiDB
{

class SQLiteCursorData;

/*!

*/
class SQLiteCursor : public Cursor
{
public:
    virtual ~SQLiteCursor();
    virtual QVariant value(uint i);

    /*! [PROTOTYPE] \return internal buffer data. */
//TODO  virtual const char *** bufferData()
    /*! [PROTOTYPE] \return current record data or NULL if there is no current records. */
    virtual const char ** rowData() const;

    virtual bool drv_storeCurrentRow(RecordData &data) const;

//  virtual bool save(RecordData& data, RowEditBuffer& buf);

    virtual int serverResult();
    virtual QString serverResultName();
    virtual QString serverErrorMsg();

protected:
    /*! Cursor will operate on \a conn, raw \a statement will be used to execute query. */
    SQLiteCursor(Connection* conn, const QString& statement, uint options = NoOptions);

    /*! Cursor will operate on \a conn, \a query schema will be used to execute query. */
    SQLiteCursor(Connection* conn, QuerySchema& query,
                 uint options = NoOptions);

    virtual bool drv_open();

    virtual bool drv_close();
//  virtual bool drv_moveFirst();
    virtual void drv_getNextRecord();
//unused  virtual bool drv_getPrevRecord();

    virtual void drv_appendCurrentRecordToBuffer();
    virtual void drv_bufferMovePointerNext();
    virtual void drv_bufferMovePointerPrev();
    virtual void drv_bufferMovePointerTo(qint64 at);

//TODO  virtual void drv_storeCurrentRecord();

    //PROTOTYPE:
    /*! Method called when cursor's buffer need to be cleared
      (only for buffered cursor type), eg. in close(). */
    virtual void drv_clearBuffer();

    virtual void drv_clearServerResult();

    SQLiteCursorData *d;

    friend class SQLiteConnection;
};

} //namespace KexiDB

#endif


