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

#ifndef KEXIDB_RECORDDATA_H
#define KEXIDB_RECORDDATA_H

#include <QVector>
#include <QVariant>
#include "calligradb_export.h"

namespace KexiDB
{

//! @short Structure for storing single record with type information.
//! @todo consider using something like QVector<QVariant*> ?
class CALLIGRADB_EXPORT RecordData : public QVector<QVariant>
{
public:
    /*! Creates a new record data with no columns. */
    inline RecordData() : QVector<QVariant>() {}

    /*! Creates a new record data with \a numCols columns. */
    inline RecordData(int numCols) : QVector<QVariant>(numCols) {}

    /*! Clears existing column values and inits new \a numCols
     columns with empty values. The vector is resized to \a numCols. */
    inline void init(int numCols) {
        clear();
        resize(numCols);
    }

    /*! Clears existing column values, current number of columns is preserved. */
    inline void clearValues() {
        init(count());
    }

    /*! @return debug string for this record. */
    QString debugString() const;

    /*! Prints debug string for this record. */
    void debug() const;
};
}

#endif
