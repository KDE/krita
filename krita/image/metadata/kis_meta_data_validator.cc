/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_meta_data_validator.h"

#include "kis_meta_data_store.h"
#include "kis_meta_data_value.h"
#include "kis_meta_data_entry.h"
#include "kis_meta_data_schema.h"
#include "kis_meta_data_type_info.h"

using namespace KisMetaData;

//------------------- Validator::Reason -------------------//

struct Q_DECL_HIDDEN Validator::Reason::Private {
    Type type;
};

Validator::Reason::Reason(Type _type) : d(new Private)
{
    d->type = _type;
}

Validator::Reason::Reason(const Validator::Reason& _rhs) : d(new Private(*_rhs.d))
{
}

Validator::Reason& Validator::Reason::operator=(const Validator::Reason & _rhs)
{
    *d = *_rhs.d;
    return *this;
}

Validator::Reason::~Reason()
{
    delete d;
}

Validator::Reason::Type Validator::Reason::type() const
{
    return d->type;
}

//------------------- Validator -------------------//

struct Q_DECL_HIDDEN Validator::Private {
    Private() : countValidEntries(0) {
    }
    int countValidEntries;
    QMap<QString, Reason> invalidEntries;
    const Store* store;
};

Validator::Validator(const Store* store) : d(new Private)
{
    d->store = store;
    revalidate();
}

Validator::~Validator()
{
    delete d;
}

void Validator::revalidate()
{
    QList<Entry> entries = d->store->entries();
    d->countValidEntries = 0;
    d->invalidEntries.clear();
    Q_FOREACH (const Entry& entry, entries) {
        const TypeInfo* typeInfo = entry.schema()->propertyType(entry.name());
        if (typeInfo) {
            if (typeInfo->hasCorrectType(entry.value())) {
                if (typeInfo->hasCorrectValue(entry.value())) {
                    ++d->countValidEntries;
                } else {
                    d->invalidEntries[entry.qualifiedName()] = Reason(Reason::INVALID_VALUE);
                }
            } else {
                d->invalidEntries[entry.qualifiedName()] = Reason(Reason::INVALID_TYPE);
            }
        } else {
            d->invalidEntries[entry.qualifiedName()] = Reason(Reason::UNKNOWN_ENTRY);
        }
    }
}

int Validator::countInvalidEntries() const
{
    return d->invalidEntries.size();
}
int Validator::countValidEntries() const
{
    return d->countValidEntries;
}
const QMap<QString, Validator::Reason>& Validator::invalidEntries() const
{
    return d->invalidEntries;
}


