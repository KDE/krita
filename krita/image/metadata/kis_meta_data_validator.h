/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_META_DATA_VALIDATION_RESULT_H_
#define _KIS_META_DATA_VALIDATION_RESULT_H_

#include <QMap>
#include <QString>

#include <krita_export.h>

namespace KisMetaData
{
class Store;
/**
 * This class contains information on the validation results of a \ref KisMetaData::Store .
 */
class KRITAIMAGE_EXPORT Validator
{
public:
    class KRITAIMAGE_EXPORT Reason
    {
        friend class Validator;
        friend class QMap<QString, Reason>;
    public:
        enum Type {
            UNKNOWN_REASON,
            UNKNOWN_ENTRY,
            INVALID_TYPE,
            INVALID_VALUE
        };
    private:
        Reason(Type type = UNKNOWN_REASON);
        Reason(const Reason&);
        Reason& operator=(const Reason&);
    public:
        ~Reason();
        Type type() const;
    private:
        struct Private;
        Private* const d;
    };
public:
    /**
     * Validate a store. This constructore will call the \ref revalidate function.
     */
    Validator(const Store*);
    ~Validator();
    int countInvalidEntries() const;
    int countValidEntries() const;
    const QMap<QString, Reason>& invalidEntries() const;
    /**
     * Call this function to revalidate the store.
     */
    void revalidate();
private:
    struct Private;
    Private* const d;
};
}

#endif
