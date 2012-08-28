/* This file is part of the KDE project
   Copyright (C) 2006-2012 Jarosław Staniek <staniek@kde.org>

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

#ifndef KEXIDB_QUERYSCHEMAPARAMETER_H
#define KEXIDB_QUERYSCHEMAPARAMETER_H

#include "queryschema.h"

namespace KexiDB
{

//! @short A single parameter of a query schema
class CALLIGRADB_EXPORT QuerySchemaParameter
{
public:
    QuerySchemaParameter();
    ~QuerySchemaParameter();

    QString debugString() const;

    Field::Type type; //!< A datatype of the parameter
    QString message; //!< A user-visible message that will be displayed to ask for value of the parameter
};

typedef QList<QuerySchemaParameter>::Iterator QuerySchemaParameterListIterator;
typedef QList<QuerySchemaParameter>::ConstIterator QuerySchemaParameterListConstIterator;

//! Shows debug information for \a list
CALLIGRADB_EXPORT void debug(const QuerySchemaParameterList& list);

//! @short An iterator for a list of values of query schema parameters
//! Allows to iterate over parameters and returns QVariant value or well-formatted string.
//! The iterator is initially set to the last item because of the parser requirements
class CALLIGRADB_EXPORT QuerySchemaParameterValueListIterator
{
public:
    QuerySchemaParameterValueListIterator(
        const Driver* driver, const QList<QVariant>& params);
    ~QuerySchemaParameterValueListIterator();

    //! \return previous value
    QVariant getPreviousValue();

    //! \return previous value as string formatted using driver's escaping
    QString getPreviousValueAsString(Field::Type type);
protected:
    class Private;
    Private * const d;
};

} //namespace KexiDB

#endif
