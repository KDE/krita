/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
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


