/* Koffice/Kexi report engine
 * Copyright (C) 2007-2010 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __KOREPORTDATA_H__
#define __KOREPORTDATA_H__

#include "koreport_export.h"
#include <QStringList>

/**

*/
class KOREPORT_EXPORT KoReportData
{

public:
    virtual ~KoReportData() {}

    //! Describes sorting for single field
    /*! By default the order is ascending. */
    class KOREPORT_EXPORT SortedField
    {
    public:
        SortedField() : order(Qt::AscendingOrder) {}
        QString field;
        Qt::SortOrder order;
        //bool group; //probably not required?
    };

    //!Open the dataset
    virtual bool open() = 0;

    //!Close the dataset
    virtual bool close() = 0;

    //!Move to the next record
    virtual bool moveNext() = 0;

    //!Move to the previous record
    virtual bool movePrevious() = 0;

    //!Move to the first record
    virtual bool moveFirst() = 0;

    //!Move to the last record
    virtual bool moveLast() = 0;

    //!Return the current position in the dataset
    virtual long at() const = 0;

    //!Return the total number of records
    virtual long recordCount() const = 0;

    //!Return the index number of the field given by nane field
    virtual unsigned int fieldNumber(const QString &field) const = 0;

    //!Return the list of field names
    virtual QStringList fieldNames() const = 0;
    
    //!Return the list of field keys. Returns fieldNames() by default
    virtual QStringList fieldKeys() const { return fieldNames(); }
    
    //!Return the value of the field at the given position for the current record
    virtual QVariant value(unsigned int) const = 0;

    //!Return the value of the field fir the given name for the current record
    virtual QVariant value(const QString &field) const = 0;

    //!Return the name of this source
    virtual QString sourceName() const {return QString();}

    //!Sets the sorting for the data
    //!Should be called before open() so that the data source can be edited accordingly
    //!Default impl does nothing
    virtual void setSorting(const QList<SortedField>& sorting) { Q_UNUSED(sorting); }

    //!Utility Functions
    //!TODO These are probably eligable to be moved into a new class

    //!Allow the reportdata implementation to return a list of possible scripts for a given language
    virtual QStringList scriptList(const QString& language) const {
        Q_UNUSED(language); return QStringList();
    }

    //!Allow the reportdata implementation to return some script code based on a specific script name
    //!and a language, as set in the report
    virtual QString scriptCode(const QString& script, const QString& language) const {
        Q_UNUSED(script); Q_UNUSED(language); return QString();
    }

    //!Return a list of data sources possible for advanced controls
    virtual QStringList dataSources() const { return QStringList(); }
    //!Return a list of data source names possible for advanced controls.
    //!Returns dataSources() by default
    virtual QStringList dataSourceNames() const { return dataSources(); }

    //!Allow a driver to create a new instance with a new data source
    //!source is a driver specific identifier
    //!Owner of the returned pointer is the caller
    virtual KoReportData* data(const QString &source) {
        Q_UNUSED(source); return 0;
    }
};

#endif
