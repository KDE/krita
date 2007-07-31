/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_meta_data_store.h"

#include <kdebug.h>

#include "kis_meta_data_entry.h"
#include "kis_meta_data_schema.h"
#include "kis_meta_data_value.h"

using namespace KisMetaData;

uint qHash(const Entry& e)
{
    return qHash(e.qualifiedName());
}

struct Store::Private {
    QHash<QString, Entry> entries;
};

Store::Store() : d(new Private)
{
    
}

Store::Store(const Store& s) : d(new Private(*s.d)) {
    // TODO: reaffect all schemas
}

Store::~Store()
{
    delete d;
}

void Store::copyFrom(const Store* store)
{
    for(QHash<QString, Entry>::const_iterator entryIt = store->begin();
        entryIt != store->end(); ++entryIt)
    {
        const Entry& entry = entryIt.value();
        if(entry.value().type() != KisMetaData::Value::Invalid)
        {
            if(hasEntry(entry.qualifiedName()))
            {
                getEntry( entry.qualifiedName() ).value() = entry.value();
            } else {
                addEntry(entry);
            }
        }
    }
}

bool Store::addEntry(const Entry& entry)
{
    if(d->entries.contains(entry.qualifiedName()) and d->entries[entry.qualifiedName()].isValid() )
    {
        kDebug(41001) <<"Entry" << entry.qualifiedName() <<" allready exist in the store, can't be included twice";
        return false;
    }
    d->entries.insert(entry.qualifiedName(), entry);
    return true;
}

bool Store::empty() const
{
    return d->entries.empty();
}

bool Store::containsEntry(QString uri, QString entryName) const
{
    const Schema* schema = SchemaRegistry::instance()->schemaFromUri(uri);
    return d->entries.contains(schema->generateQualifiedName(entryName));
}

Entry& Store::getEntry(QString entryKey)
{
    return d->entries[entryKey];
}

Entry& Store::getEntry(QString uri, QString entryName)
{
    const Schema* schema = SchemaRegistry::instance()->schemaFromUri(uri);
    Q_ASSERT(schema);
    return getEntry(schema, entryName );
}

Entry& Store::getEntry(const KisMetaData::Schema* schema, QString entryName)
{
    return getEntry(schema->generateQualifiedName( entryName ));
}

bool Store::hasEntry(QString uri, QString entryName)
{
    const Schema* schema = SchemaRegistry::instance()->schemaFromUri(uri);
    Q_ASSERT(schema);
    return hasEntry(schema, entryName );
}

bool Store::hasEntry(const KisMetaData::Schema* schema, QString entryName)
{
    return hasEntry(schema->generateQualifiedName(entryName ));
}

bool Store::hasEntry(QString entryKey)
{
    return d->entries.keys().contains( entryKey );
}

const Value& Store::getValue(QString uri, QString entryName)
{
    return getEntry(uri, entryName).value();
}

QHash<QString, Entry>::const_iterator Store::begin() const
{
    return d->entries.begin();
}

QHash<QString, Entry>::const_iterator Store::end() const
{
    return d->entries.end();
}

void Store::debugDump() const
{
    kDebug(41001) <<"=== Dumping MetaData Store ===";
/*    kDebug(41001) <<" - Schemas";
    foreach(Schema* s, d->uri2Schema)
    {
        kDebug(41001) << *s;
    }*/
    kDebug(41001) <<" - Metadata (there are" << d->entries.size() <<" entries)";
    foreach(const Entry& e, d->entries)
    {
        if(e.isValid())
        {
            //kDebug(41001) << e;
        } else {
            kDebug(41001) <<"Invalid entry";
        }
    }
}
