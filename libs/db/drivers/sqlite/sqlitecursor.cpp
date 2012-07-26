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

#include "sqlitecursor.h"

#include "sqliteconnection.h"
#include "sqliteconnection_p.h"

#include <db/error.h>
#include <db/driver.h>
#include <db/utils.h>

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <kdebug.h>
#include <klocale.h>

#include <QVector>
#include <QDateTime>
#include <QByteArray>

using namespace KexiDB;

//! safer interpretations of boolean values for SQLite
static bool sqliteStringToBool(const QString& s)
{
    return s.toLower() == "yes" || (s.toLower() != "no" && s != "0");
}

//----------------------------------------------------

class KexiDB::SQLiteCursorData : public SQLiteConnectionInternal
{
public:
    SQLiteCursorData(Connection* conn)
            :
            SQLiteConnectionInternal(conn)
//   : curr_cols(0)
//   errmsg_p(0)
//   , res(SQLITE_OK)
            , curr_coldata(0)
            , curr_colname(0)
            , cols_pointers_mem_size(0)
//   , rec_stored(false)
            /* MOVED TO Cursor:
                  , cols_pointers_mem_size(0)
                  , records_in_buf(0)
                  , buffering_completed(false)
                  , at_buffer(false)*/
//   , rowDataReadyToFetch(false)
    {
        data_owned = false;
    }

    /*#ifdef SQLITE3
        void fetchRowDataIfNeeded()
        {
          if (!rowDataReadyToFetch)
            return true;
          rowDataReadyToFetch = false;
          m_fieldCount = sqlite3_data_count(data);
          for (int i=0; i<m_fieldCount; i++) {

          }
        }
    #endif*/

    QByteArray st;
    //for sqlite:
//  sqlite_struct *data; //! taken from SQLiteConnection
    sqlite3_stmt *prepared_st_handle;

    char *utail;

//  QString errmsg; //<! server-specific message of last operation
//  char *errmsg_p; //<! temporary: server-specific message of last operation
//  int res; //<! result code of last operation on server

//  int curr_cols;
    const char **curr_coldata;
    const char **curr_colname;

    int next_cols;
//  const char **next_coldata;
//  const char **next_colname;
//  bool rec_stored : 1; //! true, current record is stored in next_coldata

    /* MOVED TO Cursor:
        uint cols_pointers_mem_size; //! size of record's array of pointers to values
        int records_in_buf; //! number of records currently stored in the buffer
        bool buffering_completed; //! true if we have already all records stored in the buffer
        QPtrVector<const char*> records; //buffer data
        bool at_buffer; //! true if we already point to the buffer with curr_coldata
    */

    /*  int prev_cols;
        const char **prev_coldata;
        const char **prev_colname;*/

    uint cols_pointers_mem_size; //! size of record's array of pointers to values
    QVector<const char**> records;//! buffer data
//  bool rowDataReadyToFetch : 1;

    inline QVariant getValue(Field *f, int i) {
        int type = sqlite3_column_type(prepared_st_handle, i);
        if (type == SQLITE_NULL) {
            return QVariant();
        } else if (!f || type == SQLITE_TEXT) {
//TODO: support for UTF-16
#define GET_sqlite3_column_text QString::fromUtf8( (const char*)sqlite3_column_text(prepared_st_handle, i) )
            if (!f || f->isTextType())
                return GET_sqlite3_column_text;
            else {
                switch (f->type()) {
                case Field::Date:
                    return QDate::fromString(GET_sqlite3_column_text, Qt::ISODate);
                case Field::Time:
                    //QDateTime - a hack needed because QVariant(QTime) has broken isNull()
                    return KexiDB::stringToHackedQTime(GET_sqlite3_column_text);
                case Field::DateTime: {
                    QString tmp(GET_sqlite3_column_text);
                    tmp[10] = 'T'; //for ISODate compatibility
                    return QDateTime::fromString(tmp, Qt::ISODate);
                }
                case Field::Boolean:
                    return sqliteStringToBool(GET_sqlite3_column_text);
                default:
                    return QVariant(); //TODO
                }
            }
        } else if (type == SQLITE_INTEGER) {
            switch (f->type()) {
            case Field::Byte:
            case Field::ShortInteger:
            case Field::Integer:
                return QVariant(sqlite3_column_int(prepared_st_handle, i));
            case Field::BigInteger:
                return QVariant((qint64)sqlite3_column_int64(prepared_st_handle, i));
            case Field::Boolean:
                return sqlite3_column_int(prepared_st_handle, i) != 0;
            default:;
            }
            if (f->isFPNumericType()) //WEIRD, YEAH?
                return QVariant((double)sqlite3_column_int(prepared_st_handle, i));
            else
                return QVariant(); //TODO
        } else if (type == SQLITE_FLOAT) {
            if (f && f->isFPNumericType())
                return QVariant(sqlite3_column_double(prepared_st_handle, i));
            else if (!f || f->isIntegerType())
                return QVariant((double)sqlite3_column_double(prepared_st_handle, i));
            else
                return QVariant(); //TODO
        } else if (type == SQLITE_BLOB) {
            if (f && f->type() == Field::BLOB) {
//! @todo efficient enough?
                return QByteArray((const char*)sqlite3_column_blob(prepared_st_handle, i),
                                  sqlite3_column_bytes(prepared_st_handle, i));
            } else
                return QVariant(); //TODO
        }
        return QVariant();
    }
};

SQLiteCursor::SQLiteCursor(Connection* conn, const QString& statement, uint options)
        : Cursor(conn, statement, options)
        , d(new SQLiteCursorData(conn))
{
    d->data = static_cast<SQLiteConnection*>(conn)->d->data;
}

SQLiteCursor::SQLiteCursor(Connection* conn, QuerySchema& query, uint options)
        : Cursor(conn, query, options)
        , d(new SQLiteCursorData(conn))
{
    d->data = static_cast<SQLiteConnection*>(conn)->d->data;
}

SQLiteCursor::~SQLiteCursor()
{
    close();
    delete d;
}

bool SQLiteCursor::drv_open()
{
// d->st.resize(statement.length()*2);
    //TODO: decode
// d->st = statement.local8Bit();
// d->st = m_conn->driver()->escapeString( statement.local8Bit() );

    if (! d->data) {
        // this may as example be the case if SQLiteConnection::drv_useDatabase()
        // wasn't called before. Normaly sqlite_compile/sqlite3_prepare
        // should handle it, but it crashes in in sqlite3SafetyOn at util.c:786
        kWarning() << "SQLiteCursor::drv_open(): Database handle undefined.";
        return false;
    }

    d->st = m_sql.toUtf8();
    d->res = sqlite3_prepare(
                 d->data,            /* Database handle */
                 d->st.constData(),       /* SQL statement, UTF-8 encoded */
                 d->st.length(),             /* Length of zSql in bytes. */
                 &d->prepared_st_handle,  /* OUT: Statement handle */
                 0/*const char **pzTail*/     /* OUT: Pointer to unused portion of zSql */
             );
    if (d->res != SQLITE_OK) {
        d->storeResult();
        return false;
    }
//cursor is automatically @ first record
// m_beforeFirst = true;

    if (isBuffered()) {
        d->records.resize(128); //TODO: manage size dynamically
    }

    return true;
}

/*bool SQLiteCursor::drv_getFirstRecord()
{
  bool ok = drv_getNextRecord();*/
/* if ((m_options & Buffered) && ok) { //1st record is there:
    //compute parameters for cursor's buffer:
    //-size of record's array of pointer to values
    d->cols_pointers_mem_size = d->curr_cols * sizeof(char*);
    d->records_in_buf = 1;
  }*/
/*return ok;
}*/

bool SQLiteCursor::drv_close()
{
    d->res = sqlite3_finalize(d->prepared_st_handle);
    if (d->res != SQLITE_OK) {
        d->storeResult();
        return false;
    }
    return true;
}

void SQLiteCursor::drv_getNextRecord()
{
    d->res = sqlite3_step(d->prepared_st_handle);
    if (d->res == SQLITE_ROW) {
        m_result = FetchOK;
        m_fieldCount = sqlite3_data_count(d->prepared_st_handle);
//      //for SQLITE3 data fetching is delayed. Now we even do not take field count information
//      // -- just set a flag that we've a data not fetched but available
//  d->rowDataReadyToFetch = true;
        m_fieldsToStoreInRow = m_fieldCount;
        //(m_logicalFieldCount introduced) m_fieldCount -= (m_containsROWIDInfo ? 1 : 0);
    } else {
//  d->rowDataReadyToFetch = false;
        if (d->res == SQLITE_DONE)
            m_result = FetchEnd;
        else
            m_result = FetchError;
    }

    //debug
    /*
      if ((int)m_result == (int)FetchOK && d->curr_coldata) {
        for (uint i=0;i<m_fieldCount;i++) {
          KexiDBDrvDbg<<"col."<< i<<": "<< d->curr_colname[i]<<" "<< d->curr_colname[m_fieldCount+i]
          << " = " << (d->curr_coldata[i] ? QString::fromLocal8Bit(d->curr_coldata[i]) : "(NULL)");
        }
    //  KexiDBDrvDbg << "SQLiteCursor::drv_getNextRecord(): "<<m_fieldCount<<" col(s) fetched";
      }*/
}

void SQLiteCursor::drv_appendCurrentRecordToBuffer()
{
// KexiDBDrvDbg << "SQLiteCursor::drv_appendCurrentRecordToBuffer():";
    if (!d->curr_coldata)
        return;
    if (!d->cols_pointers_mem_size)
        d->cols_pointers_mem_size = m_fieldCount * sizeof(char*);
    const char **record = (const char**)malloc(d->cols_pointers_mem_size);
    const char **src_col = d->curr_coldata;
    const char **dest_col = record;
    for (uint i = 0; i < m_fieldCount; i++, src_col++, dest_col++) {
//  KexiDBDrvDbg << i <<": '" << *src_col << "'";
//  KexiDBDrvDbg << "src_col: " << src_col;
        *dest_col = *src_col ? strdup(*src_col) : 0;
    }
    d->records[m_records_in_buf] = record;
// KexiDBDrvDbg << "SQLiteCursor::drv_appendCurrentRecordToBuffer() ok.";
}

void SQLiteCursor::drv_bufferMovePointerNext()
{
    d->curr_coldata++; //move to next record in the buffer
}

void SQLiteCursor::drv_bufferMovePointerPrev()
{
    d->curr_coldata--; //move to prev record in the buffer
}

//compute a place in the buffer that contain next record's data
//and move internal buffer pointer to that place
void SQLiteCursor::drv_bufferMovePointerTo(qint64 at)
{
    if (d->records.count() > at)
	d->curr_coldata = d->records.at(at);
}

void SQLiteCursor::drv_clearBuffer()
{
    if (d->cols_pointers_mem_size > 0) {
        const uint records_in_buf = m_records_in_buf;
        const char ***r_ptr = d->records.data();
        for (uint i = 0; i < records_in_buf; i++, r_ptr++) {
            //  const char **record = m_records.at(i);
            const char **field_data = *r_ptr;
            //  for (int col=0; col<d->curr_cols; col++, field_data++) {
            for (uint col = 0; col < m_fieldCount; col++, field_data++) {
                free((void*)*field_data); //free field memory
            }
            free(*r_ptr); //free pointers to fields array
        }
    }
// d->curr_cols=0;
// m_fieldCount=0;
    m_records_in_buf = 0;
    d->cols_pointers_mem_size = 0;
// m_at_buffer=false;
    d->records.clear();
}

/*
void SQLiteCursor::drv_storeCurrentRecord()
{
#if 0
  assert(!m_data->rec_stored);
  m_data->rec_stored = true;
  m_data->next_cols = m_data->curr_cols;
  for (int i=0;i<m_data->curr_cols;i++) {
    KexiDBDrvDbg<<"[COPY] "<<i<<": "<< m_data->curr_coldata[i];
    if (m_data->curr_coldata[i])
      m_data->next_coldata[i] = strdup( m_data->curr_coldata[i] );
    else
      m_data->next_coldata[i] = 0;
  }
#endif
}
*/

/*TODO
const char *** SQLiteCursor::bufferData()
{
  if (!isBuffered())
    return 0;
  return m_records.data();
}*/

const char ** SQLiteCursor::rowData() const
{
    return d->curr_coldata;
}

bool SQLiteCursor::drv_storeCurrentRow(RecordData &data) const
{
    //const uint realCount = m_fieldCount + (m_containsROWIDInfo ? 1 : 0);
//not needed data.resize(m_fieldCount);
    if (!m_fieldsExpanded) {//simple version: without types
        for (uint i = 0; i < m_fieldCount; i++) {
            data[i] = QString::fromUtf8((const char*)sqlite3_column_text(d->prepared_st_handle, i));
        }
        return true;
    }

    //const uint fieldsExpandedCount = m_fieldsExpanded->count();
    const uint maxCount = qMin(m_fieldCount, (uint)m_fieldsExpanded->count());
    // i - visible field's index, j - physical index
    for (uint i = 0, j = 0; i < m_fieldCount; ++i, ++j) {
//  while (j < m_detailedVisibility.count() && !m_detailedVisibility[j]) //!m_query->isColumnVisible(j))
//   j++;
        while (j < maxCount && !m_fieldsExpanded->at(j)->visible)
            j++;
        if (j >= (maxCount /*+(m_containsROWIDInfo ? 1 : 0)*/)) {
            //ERR!
            break;
        }
        //(m_logicalFieldCount introduced) Field *f = (m_containsROWIDInfo && i>=m_fieldCount) ? 0 : m_fieldsExpanded->at(j)->field;
        Field *f = (i >= m_fieldCount) ? 0 : m_fieldsExpanded->at(j)->field;
//  KexiDBDrvDbg << "SQLiteCursor::storeCurrentRow(): col=" << (col ? *col : 0);
        data[i] = d->getValue(f, i); //, !f /*!f means ROWID*/);
    }
    return true;
}

QVariant SQLiteCursor::value(uint i)
{
// if (i > (m_fieldCount-1+(m_containsROWIDInfo?1:0))) //range checking
    if (i > (m_fieldCount - 1)) //range checking
        return QVariant();
//TODO: allow disable range checking! - performance reasons
// const KexiDB::Field *f = m_query ? m_query->field(i) : 0;
    KexiDB::Field *f = (m_fieldsExpanded && i < (uint)m_fieldsExpanded->count())
                       ? m_fieldsExpanded->at(i)->field : 0;
    return d->getValue(f, i); //, i==m_logicalFieldCount/*ROWID*/);
}

/*! Stores string value taken from field number \a i to \a str.
 \return false when range checking failed.
bool SQLiteCursor::storeStringValue(uint i, QString &str)
{
  if (i > (m_fieldCount-1)) //range checking
    return false;
  str = d->curr_coldata[i];
  return true;
}*/

int SQLiteCursor::serverResult()
{
    return d->res;
}

QString SQLiteCursor::serverResultName()
{
    return QString::fromLatin1(d->result_name);
}

QString SQLiteCursor::serverErrorMsg()
{
    return d->errmsg;
}

void SQLiteCursor::drv_clearServerResult()
{
    d->res = SQLITE_OK;
    d->errmsg_p = 0;
}

