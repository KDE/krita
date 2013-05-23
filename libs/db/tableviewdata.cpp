/* This file is part of the KDE project
   Copyright (C) 2002   Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2003   Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003-2007 Jarosław Staniek <staniek@kde.org>

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

   Original Author:  Till Busch <till@bux.at>
   Original Project: buX (www.bux.at)
*/

#include "tableviewdata.h"

#include "field.h"
#include "roweditbuffer.h"
#include "cursor.h"
#include "utils.h"
#include "error.h"

#include <kdebug.h>
#include <klocale.h>

#include <QApplication>
#include <QVector>

using namespace KexiDB;

unsigned short charTable[] = {
#include "chartable.txt"
};

//-------------------------------

//! @internal A functor used in qSort() in order to sort by a given column
class LessThanFunctor
{
private:
    bool m_ascendingOrder;
    QVariant m_leftTmp, m_rightTmp;
    int m_sortedColumn;

    bool (*m_lessThanFunction)(const QVariant&, const QVariant&);

#define CAST_AND_COMPARE(casting) \
    return left.casting() < right.casting()

    static bool cmpInt(const QVariant& left, const QVariant& right) {
        CAST_AND_COMPARE(toInt);
    }

    static bool cmpUInt(const QVariant& left, const QVariant& right) {
        CAST_AND_COMPARE(toUInt);
    }

    static bool cmpLongLong(const QVariant& left, const QVariant& right) {
        CAST_AND_COMPARE(toLongLong);
    }

    static bool cmpULongLong(const QVariant& left, const QVariant& right) {
        CAST_AND_COMPARE(toULongLong);
    }

    static bool cmpDouble(const QVariant& left, const QVariant& right) {
        CAST_AND_COMPARE(toDouble);
    }

    static bool cmpDate(const QVariant& left, const QVariant& right) {
        CAST_AND_COMPARE(toDate);
    }

    static bool cmpDateTime(const QVariant& left, const QVariant& right) {
        CAST_AND_COMPARE(toDateTime);
    }

    static bool cmpTime(const QVariant& left, const QVariant& right) {
        CAST_AND_COMPARE(toDate);
    }

    static bool cmpString(const QVariant& left, const QVariant& right) {
        const QString &as = left.toString();
        const QString &bs = right.toString();

        const QChar *a = as.unicode();
        const QChar *b = bs.unicode();

        if (a == b || b == 0)
            return false;
        if (a == 0 && b != 0)
            return true;

        unsigned short au;
        unsigned short bu;
        int len = qMin(as.length(), bs.length());

        forever {
            au = a->unicode();
            bu = b->unicode();
            au = (au <= 0x17e ? charTable[au] : 0xffff);
            bu = (bu <= 0x17e ? charTable[bu] : 0xffff);

            if (len <= 0)
                return false;
            len--;

            if (au != bu)
                return au < bu;
            a++;
            b++;
        }
        return false;
    }

    //! Compare function for BLOB data (QByteArray). Uses size as the weight.
    static bool cmpBLOB(const QVariant& left, const QVariant& right) {
        return left.toByteArray().size() < right.toByteArray().size();
    }

public:
    LessThanFunctor()
            : m_ascendingOrder(true)
            , m_sortedColumn(-1)
            , m_lessThanFunction(0)
    {
    }

    void setColumnType(const Field& field) {
        const Field::Type t = field.type();
        if (field.isTextType())
            m_lessThanFunction = &cmpString;
        if (Field::isFPNumericType(t))
            m_lessThanFunction = &cmpDouble;
        else if (t == Field::Integer && field.isUnsigned())
            m_lessThanFunction = &cmpUInt;
        else if (t == Field::Boolean || Field::isNumericType(t))
            m_lessThanFunction = &cmpInt; //other integers
        else if (t == Field::BigInteger) {
            if (field.isUnsigned())
                m_lessThanFunction = &cmpULongLong;
            else
                m_lessThanFunction = &cmpLongLong;
        } else if (t == Field::Date)
            m_lessThanFunction = &cmpDate;
        else if (t == Field::Time)
            m_lessThanFunction = &cmpTime;
        else if (t == Field::DateTime)
            m_lessThanFunction = &cmpDateTime;
        else if (t == Field::BLOB)
            //! @todo allow users to define BLOB sorting function?
            m_lessThanFunction = &cmpBLOB;
        else
            m_lessThanFunction = &cmpString; //anything else
    }

    void setAscendingOrder(bool ascending) {
        m_ascendingOrder = ascending;
    }

    void setSortedColumn(int column) {
        m_sortedColumn = column;
    }

#define _IIF(a,b) ((a) ? (b) : !(b))

    //! Main comparison operator that takes column number, type and order into account
    bool operator()(RecordData* record1, RecordData* record2) {
        // compare NULLs : NULL is smaller than everything
        if ((m_leftTmp = record1->at(m_sortedColumn)).isNull())
            return _IIF(m_ascendingOrder, !record2->at(m_sortedColumn).isNull());
        if ((m_rightTmp = record2->at(m_sortedColumn)).isNull())
            return !m_ascendingOrder;

        return _IIF(m_ascendingOrder, m_lessThanFunction(m_leftTmp, m_rightTmp));
    }
};
#undef _IIF
#undef CAST_AND_COMPARE

//! @internal
class TableViewData::Private
{
public:
    Private()
            : sortedColumn(0)
            , realSortedColumn(0)
            , type(1)
            , pRowEditBuffer(0)
            , visibleColumnsCount(0)
            , visibleColumnsIDs(100)
            , globalColumnsIDs(100)
            , readOnly(false)
            , insertingEnabled(true)
            , containsROWIDInfo(false)
            , ascendingOrder(false)
            , descendingOrder(false)
            , autoIncrementedColumn(-2) {
    }

    ~Private() {
        delete pRowEditBuffer;
    }

    //! (logical) sorted column number, set by setSorting()
    //! can differ from realSortedColumn if there's lookup column used
    int sortedColumn;

    //! real sorted column number, set by setSorting(), used by cmp*() methods
    int realSortedColumn;

    LessThanFunctor lessThanFunctor;

    short type;

    RowEditBuffer *pRowEditBuffer;

    QPointer<Cursor> cursor;

    ResultInfo result;

    uint visibleColumnsCount;

    QVector<int> visibleColumnsIDs, globalColumnsIDs;

    bool readOnly;

    bool insertingEnabled;

    /*! Used in acceptEditor() to avoid infinite recursion,
     eg. when we're calling TableViewData::acceptRowEdit() during cell accepting phase. */
//  bool inside_acceptEditor;

    //! @see TableViewData::containsROWIDInfo()
    bool containsROWIDInfo;

    //! true if ascending sort order is set
    bool ascendingOrder;

    //! true if descending sort order is set
    bool descendingOrder;

    int autoIncrementedColumn;
};

//-------------------------------

TableViewData::TableViewData()
        : QObject()
        , TableViewDataBase()
        , d(new Private)
{
    init();
}

// db-aware ctor
TableViewData::TableViewData(Cursor *c)
        : QObject()
        , TableViewDataBase()
        , d(new Private)
{
    init();
    d->cursor = c;
    d->containsROWIDInfo = d->cursor->containsROWIDInfo();
    if (d->cursor && d->cursor->query()) {
        const QuerySchema::FieldsExpandedOptions fieldsExpandedOptions
        = d->containsROWIDInfo ? QuerySchema::WithInternalFieldsAndRowID
          : QuerySchema::WithInternalFields;
        m_itemSize = d->cursor->query()->fieldsExpanded(fieldsExpandedOptions).count();
    } else
        m_itemSize = m_columns.count() + (d->containsROWIDInfo ? 1 : 0);

    // Allocate TableViewColumn objects for each visible query column
    const QueryColumnInfo::Vector fields = d->cursor->query()->fieldsExpanded();
    const uint fieldsCount = fields.count();
    for (uint i = 0;i < fieldsCount;i++) {
        QueryColumnInfo *ci = fields[i];
        if (ci->visible) {
            QueryColumnInfo *visibleLookupColumnInfo = 0;
            if (ci->indexForVisibleLookupValue() != -1) {
                //Lookup field is defined
                visibleLookupColumnInfo = d->cursor->query()->expandedOrInternalField(ci->indexForVisibleLookupValue());
                /* not needed
                if (visibleLookupColumnInfo) {
                  // 2. Create a TableViewData object for each found lookup field
                }*/
            }
            TableViewColumn* col = new TableViewColumn(*d->cursor->query(), *ci, visibleLookupColumnInfo);
            addColumn(col);
        }
    }
}

TableViewData::TableViewData(
    const QList<QVariant> &keys, const QList<QVariant> &values,
    Field::Type keyType, Field::Type valueType)
        : QObject()
        , TableViewDataBase()
        , d(new Private)
{
    init(keys, values, keyType, valueType);
}

TableViewData::TableViewData(
    Field::Type keyType, Field::Type valueType)
        : QObject()
        , TableViewDataBase()
        , d(new Private)
{
    const QList<QVariant> empty;
    init(empty, empty, keyType, valueType);
}

TableViewData::~TableViewData()
{
    emit destroying();
    clearInternal(false /* !processEvents */);
    qDeleteAll(m_columns);
    delete d;
}

void TableViewData::init(
    const QList<QVariant> &keys, const QList<QVariant> &values,
    Field::Type keyType, Field::Type valueType)
{
    init();
    Field *keyField = new Field("key", keyType);
    keyField->setPrimaryKey(true);
    TableViewColumn *keyColumn = new TableViewColumn(*keyField, true);
    keyColumn->setVisible(false);
    addColumn(keyColumn);

    Field *valueField = new Field("value", valueType);
    TableViewColumn *valueColumn = new TableViewColumn(*valueField, true);
    addColumn(valueColumn);

    uint cnt = qMin(keys.count(), values.count());
    QList<QVariant>::ConstIterator it_keys = keys.constBegin();
    QList<QVariant>::ConstIterator it_values = values.constBegin();
    for (;cnt > 0;++it_keys, ++it_values, cnt--) {
        RecordData *record = new RecordData(2);
        (*record)[0] = (*it_keys);
        (*record)[1] = (*it_values);
        append(record);
    }
}

void TableViewData::init()
{
    m_itemSize = 0;
}

void TableViewData::deleteLater()
{
    d->cursor = 0;
    QObject::deleteLater();
}

void TableViewData::addColumn(TableViewColumn* col)
{
    m_columns.append(col);
    col->setData(this);
    if (d->globalColumnsIDs.size() < (int)m_columns.count()) {//sanity
        d->globalColumnsIDs.resize(d->globalColumnsIDs.size()*2);
    }
    if (col->isVisible()) {
        d->visibleColumnsCount++;
        if ((uint)d->visibleColumnsIDs.size() < d->visibleColumnsCount) {//sanity
            d->visibleColumnsIDs.resize(d->visibleColumnsIDs.size()*2);
        }
        d->visibleColumnsIDs[ m_columns.count()-1 ] = d->visibleColumnsCount - 1;
        d->globalColumnsIDs[ d->visibleColumnsCount-1 ] = m_columns.count() - 1;
    } else {
        d->visibleColumnsIDs[ m_columns.count()-1 ] = -1;
    }
    d->autoIncrementedColumn = -2; //clear cache;
    if (!d->cursor || !d->cursor->query())
        m_itemSize = m_columns.count() + (d->containsROWIDInfo ? 1 : 0);
}

int TableViewData::globalColumnID(int visibleID) const
{
    return d->globalColumnsIDs.value(visibleID, -1);
}

int TableViewData::visibleColumnID(int globalID) const
{
    return d->visibleColumnsIDs.value(globalID, -1);
}

bool TableViewData::isDBAware() const
{
    return d->cursor != 0;
}

Cursor* TableViewData::cursor() const
{
    return d->cursor;
}

bool TableViewData::isInsertingEnabled() const
{
    return d->insertingEnabled;
}

RowEditBuffer* TableViewData::rowEditBuffer() const
{
    return d->pRowEditBuffer;
}

const ResultInfo& TableViewData::result() const
{
    return d->result;
}

bool TableViewData::containsROWIDInfo() const
{
    return d->containsROWIDInfo;
}

QString TableViewData::dbTableName() const
{
    if (d->cursor && d->cursor->query() && d->cursor->query()->masterTable())
        return d->cursor->query()->masterTable()->name();
    return QString();
}

void TableViewData::setSorting(int column, bool ascending)
{
    if (column >= 0 && column < (int)m_columns.count()) {
        d->ascendingOrder = ascending;
        d->descendingOrder = !ascending;
    } else {
        d->ascendingOrder = false;
        d->descendingOrder = false;
        d->sortedColumn = -1;
        d->realSortedColumn = -1;
        return;
    }
    // find proper column information for sorting (lookup column points to alternate column with visible data)
    const TableViewColumn *tvcol = m_columns.at(column);
    QueryColumnInfo* visibleLookupColumnInfo = tvcol->visibleLookupColumnInfo();
    const Field *field = visibleLookupColumnInfo ? visibleLookupColumnInfo->field : tvcol->field();
    d->sortedColumn = column;
    d->realSortedColumn = tvcol->columnInfo()->indexForVisibleLookupValue() != -1
                          ? tvcol->columnInfo()->indexForVisibleLookupValue() : d->sortedColumn;

    // setup compare functor
    d->lessThanFunctor.setColumnType(*field);
    d->lessThanFunctor.setAscendingOrder(ascending);
    d->lessThanFunctor.setSortedColumn(column);
}

int TableViewData::sortedColumn() const
{
    return d->sortedColumn;
}

int TableViewData::sortingOrder() const
{
    return d->ascendingOrder ? 1 : (d->descendingOrder ? -1 : 0);
}

void TableViewData::sort()
{
    if (0 != sortingOrder())
        qSort(begin(), end(), d->lessThanFunctor);
}

void TableViewData::setReadOnly(bool set)
{
    if (d->readOnly == set)
        return;
    d->readOnly = set;
    if (d->readOnly)
        setInsertingEnabled(false);
}

void TableViewData::setInsertingEnabled(bool set)
{
    if (d->insertingEnabled == set)
        return;
    d->insertingEnabled = set;
    if (d->insertingEnabled)
        setReadOnly(false);
}

void TableViewData::clearRowEditBuffer()
{
    //init row edit buffer
    if (!d->pRowEditBuffer)
        d->pRowEditBuffer = new RowEditBuffer(isDBAware());
    else
        d->pRowEditBuffer->clear();
}

bool TableViewData::updateRowEditBufferRef(RecordData *record,
        int colnum, TableViewColumn* col, QVariant& newval, bool allowSignals,
        QVariant *visibleValueForLookupField)
{
    d->result.clear();
    if (allowSignals)
        emit aboutToChangeCell(record, colnum, newval, &d->result);
    if (!d->result.success)
        return false;

    //kDebug() << "column #" << colnum << " = " << newval.toString();
    if (!col) {
        kWarning() << "column #" << colnum << "not found! col==0";
        return false;
    }
    if (!d->pRowEditBuffer)
        d->pRowEditBuffer = new RowEditBuffer(isDBAware());
    if (d->pRowEditBuffer->isDBAware()) {
        if (!(col->columnInfo())) {
            kWarning() << "column #" << colnum << " not found!";
            return false;
        }
        d->pRowEditBuffer->insert(*col->columnInfo(), newval);

        if (col->visibleLookupColumnInfo() && visibleValueForLookupField) {
            //this is value for lookup table: update visible value as well
            d->pRowEditBuffer->insert(*col->visibleLookupColumnInfo(), *visibleValueForLookupField);
        }
        return true;
    }
    if (!(col->field())) {
        kDebug() << "column #" << colnum << "not found!";
        return false;
    }
    //not db-aware:
    const QString colname = col->field()->name();
    if (colname.isEmpty()) {
        kDebug() << "column #" << colnum << "not found!";
        return false;
    }
    d->pRowEditBuffer->insert(colname, newval);
    return true;
}

//get a new value (if present in the buffer), or the old one, otherwise
//(taken here for optimization)
#define GET_VALUE if (!pval) { \
        pval = d->cursor \
              ? d->pRowEditBuffer->at( *(*it_f)->columnInfo(), (*it_r).isNull() /* useDefaultValueIfPossible */ ) \
              : d->pRowEditBuffer->at( *f ); \
        val = pval ? *pval : *it_r; /* get old value */ \
        /*kDebug() << col << (*it_f)->columnInfo()->debugString() << "val:" << val;*/ \
    }

//! @todo if there're multiple views for this data, we need multiple buffers!
bool TableViewData::saveRow(RecordData& record, bool insert, bool repaint)
{
    if (!d->pRowEditBuffer)
        return true; //nothing to do

    //check constraints:
    //-check if every NOT NULL and NOT EMPTY field is filled
    TableViewColumn::ListIterator it_f(m_columns.constBegin());
    RecordData::ConstIterator it_r = record.constBegin();
    int col = 0;
    const QVariant *pval = 0;
    QVariant val;
    for (;it_f != m_columns.constEnd() && it_r != record.constEnd();++it_f, ++it_r, col++) {
        Field *f = (*it_f)->field();
        if (f->isNotNull()) {
            GET_VALUE;
            //check it
            if (val.isNull() && !f->isAutoIncrement()) {
                //NOT NULL violated
                d->result.msg = i18n("\"%1\" column requires a value to be entered.",
                                     f->captionOrName()) + "\n\n" + KexiDB::msgYouCanImproveData();
                d->result.desc = i18n("The column's constraint is declared as NOT NULL.");
                d->result.column = col;
                return false;
            }
        }
        if (f->isNotEmpty()) {
            GET_VALUE;
            if (!f->isAutoIncrement() && (val.isNull() || KexiDB::isEmptyValue(f, val))) {
                //NOT EMPTY violated
                d->result.msg = i18n("\"%1\" column requires a value to be entered.",
                                     f->captionOrName()) + "\n\n" + KexiDB::msgYouCanImproveData();
                d->result.desc = i18n("The column's constraint is declared as NOT EMPTY.");
                d->result.column = col;
                return false;
            }
        }
    }

    if (d->cursor) {//db-aware
        if (insert) {
            if (!d->cursor->insertRow(record, *d->pRowEditBuffer,
                                      d->containsROWIDInfo/*also retrieve ROWID*/)) {
                d->result.msg = i18n("Record inserting failed.") + "\n\n"
                                + KexiDB::msgYouCanImproveData();
                KexiDB::getHTMLErrorMesage(d->cursor, &d->result);

                /*   if (desc)
                      *desc =
                js: TODO: use KexiMainWindow::showErrorMessage(const QString &title, Object *obj)
                  after it will be moved somewhere to kexidb (this will require moving other
                    showErrorMessage() methods from KexiMainWindow to libkexiutils....)
                  then: just call: *desc = KexiDB::errorMessage(d->cursor);
                */
                return false;
            }
        } else { // row updating
//   if (d->containsROWIDInfo)
//    ROWID = record[columns.count()].toULongLong();
            if (!d->cursor->updateRow(static_cast<RecordData&>(record), *d->pRowEditBuffer,
                                      d->containsROWIDInfo/*use ROWID*/)) {
                d->result.msg = i18n("Record changing failed.") + "\n\n" + KexiDB::msgYouCanImproveData();
//! @todo set d->result.column if possible
                KexiDB::getHTMLErrorMesage(d->cursor, d->result.desc);
                return false;
            }
        }
    } else {//not db-aware version
        RowEditBuffer::SimpleMap b = d->pRowEditBuffer->simpleBuffer();
        for (RowEditBuffer::SimpleMap::ConstIterator it = b.constBegin();it != b.constEnd();++it) {
            uint i = -1;
            foreach(TableViewColumn *col, m_columns) {
                i++;
                if (col->field()->name() == it.key()) {
                    kDebug() << col->field()->name() << ": " << record.at(i).toString()
                    << " -> " << it.value().toString();
                    record[i] = it.value();
                }
            }
        }
    }

    d->pRowEditBuffer->clear();

    if (repaint)
        emit rowRepaintRequested(record);
    return true;
}

bool TableViewData::saveRowChanges(RecordData& record, bool repaint)
{
    //kDebug() << "...";
    d->result.clear();
    emit aboutToUpdateRow(&record, d->pRowEditBuffer, &d->result);
    if (!d->result.success)
        return false;

    if (saveRow(record, false /*update*/, repaint)) {
        emit rowUpdated(&record);
        return true;
    }
    return false;
}

bool TableViewData::saveNewRow(RecordData& record, bool repaint)
{
    //kDebug() << "...";
    d->result.clear();
    emit aboutToInsertRow(&record, &d->result, repaint);
    if (!d->result.success)
        return false;

    if (saveRow(record, true /*insert*/, repaint)) {
        emit rowInserted(&record, repaint);
        return true;
    }
    return false;
}

bool TableViewData::deleteRow(RecordData& record, bool repaint)
{
    d->result.clear();
    emit aboutToDeleteRow(record, &d->result, repaint);
    if (!d->result.success)
        return false;

    if (d->cursor) {//db-aware
        d->result.success = false;
        if (!d->cursor->deleteRow(static_cast<RecordData&>(record), d->containsROWIDInfo/*use ROWID*/)) {
            d->result.msg = i18n("Record deleting failed.");
            /*js: TODO: use KexiDB::errorMessage() for description (desc) as in TableViewData::saveRow() */
            KexiDB::getHTMLErrorMesage(d->cursor, &d->result);
            d->result.success = false;
            return false;
        }
    }

    int index = indexOf(&record);
    if (index == -1) {
        //aah - this shouldn't be!
        kWarning() << "!removeRef() - IMPL. ERROR?";
        d->result.success = false;
        return false;
    }
    removeAt(index);
    emit rowDeleted();
    return true;
}

void TableViewData::deleteRows(const QList<int> &rowsToDelete, bool repaint)
{
    Q_UNUSED(repaint);

    if (rowsToDelete.isEmpty())
        return;
    int last_r = 0;
    TableViewData::iterator it(begin());
    for (QList<int>::ConstIterator r_it = rowsToDelete.constBegin(); r_it != rowsToDelete.constEnd(); ++r_it) {
        for (; last_r < (*r_it); last_r++)
            ++it;
        it = erase(it);   /* this will delete *it */
        last_r++;
    }
//DON'T CLEAR BECAUSE KexiTableViewPropertyBuffer will clear BUFFERS!
//--> emit reloadRequested(); //! \todo more effective?
    emit rowsDeleted(rowsToDelete);
}

void TableViewData::insertRow(RecordData& record, uint index, bool repaint)
{
    insert(index = qMin(index, count()), &record);
    emit rowInserted(&record, index, repaint);
}

void TableViewData::clearInternal(bool processEvents)
{
    clearRowEditBuffer();
// qApp->processEvents( 1 );
//TODO: this is time consuming: find better data model
// TableViewDataBase::clear();
    const uint c = count();
#ifndef KEXI_NO_PROCESS_EVENTS
    const bool _processEvents = processEvents && !qApp->closingDown();
#endif
    for (uint i = 0; i < c; i++) {
        removeLast();
#ifndef KEXI_NO_PROCESS_EVENTS
        if (_processEvents && i % 1000 == 0)
            qApp->processEvents(QEventLoop::AllEvents, 1);
#endif
    }
}

bool TableViewData::deleteAllRows(bool repaint)
{
    clearInternal();

    bool res = true;
    if (d->cursor) {
        //db-aware
        res = d->cursor->deleteAllRows();
    }

    if (repaint)
        emit reloadRequested();
    return res;
}

int TableViewData::autoIncrementedColumn()
{
    if (d->autoIncrementedColumn == -2) {
        //find such a column
        d->autoIncrementedColumn = -1;
        foreach(TableViewColumn *col, m_columns) {
            d->autoIncrementedColumn++;
            if (col->field()->isAutoIncrement())
                break;
        }
    }
    return d->autoIncrementedColumn;
}

bool TableViewData::preloadAllRows()
{
    if (!d->cursor)
        return false;

    //const uint fcount = d->cursor->fieldCount() + (d->containsROWIDInfo ? 1 : 0);
    if (!d->cursor->moveFirst() && d->cursor->error())
        return false;

#ifndef KEXI_NO_PROCESS_EVENTS
    const bool processEvents = !qApp->closingDown();
#endif

    for (int i = 0;!d->cursor->eof();i++) {
        RecordData *record = d->cursor->storeCurrentRow();
        if (!record) {
            clear();
            return false;
        }
//  record->debug();
        append(record);
        if (!d->cursor->moveNext() && d->cursor->error()) {
            clear();
            return false;
        }
#ifndef KEXI_NO_PROCESS_EVENTS
        if (processEvents && (i % 1000) == 0)
            qApp->processEvents(QEventLoop::AllEvents, 1);
#endif
    }
    return true;
}

bool TableViewData::isReadOnly() const
{
    return d->readOnly || (d->cursor && d->cursor->connection()->isReadOnly());
}

#include "tableviewdata.moc"
