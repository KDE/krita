/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KO_DERIVED_RESOURCE_CONVERTER_H
#define __KO_DERIVED_RESOURCE_CONVERTER_H

#include <QScopedPointer>
#include <QSharedPointer>
#include "kritaflake_export.h"

class QVariant;


class KRITAFLAKE_EXPORT KoDerivedResourceConverter
{
public:
    KoDerivedResourceConverter(int key, int sourceKey);
    virtual ~KoDerivedResourceConverter();

    int key() const;
    int sourceKey() const;

    virtual QVariant fromSource(const QVariant &value) = 0;
    virtual QVariant toSource(const QVariant &value, const QVariant &sourceValue) = 0;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

typedef QSharedPointer<KoDerivedResourceConverter> KoDerivedResourceConverterSP;

#endif /* __KO_DERIVED_RESOURCE_CONVERTER_H */
