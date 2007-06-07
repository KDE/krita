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

using namespace KisMetaData;

uint qHash(const Entry& e)
{
    return qHash(e.qualifiedName());
}

struct Store::Private {
    QHash<QString, Entry> entries;
    QHash<QString, Schema*> uri2Schema;
    QHash<QString, Schema*> prefix2Schema;
};

Store::Store() : d(new Private)
{
    
}

bool Store::addEntry(const Entry& entry)
{
    if(d->entries.contains(entry.qualifiedName()))
    {
        kDebug() << "Entry " << entry.qualifiedName() << " allready exist in the store, can't be included twice" << endl;
        return false;
    }
    const Schema* schema = schemaFromUri(entry.schema()->uri());
    if(schema == 0)
    {
        schema = createSchema(entry.schema()->uri(), entry.schema()->prefix());
        if(schema == 0)
        {
            kDebug() << "Schema (" << entry.schema() << ") couldn't be added to the store" << endl;
            return false;
        }
    }
    QHash<QString, Entry>::iterator insertedIt = d->entries.insert(entry.qualifiedName(), entry);
    insertedIt->setSchema( schema);
    return true;
}

bool Store::empty() const
{
    return d->entries.empty();
}

bool Store::containsEntry(QString uri, QString entryName) const
{
    const Schema* schema = schemaFromUri(uri);
    return d->entries.contains(schema->prefix() + ":" + entryName);
}

Entry& Store::getEntry(QString entryKey)
{
    return d->entries[entryKey];
}

Entry& Store::getEntry(QString uri, QString entryName)
{
    const Schema* schema = schemaFromUri(uri);
    return d->entries[schema->prefix() + ":" + entryName];
}

QHash<QString, Entry>::const_iterator Store::begin() const
{
    return d->entries.begin();
}

QHash<QString, Entry>::const_iterator Store::end() const
{
    return d->entries.end();
}

const Schema* Store::schemaFromUri(QString uri) const
{
    return d->uri2Schema[uri];
}

const Schema* Store::schemaFromPrefix(QString prefix) const
{
    return d->prefix2Schema[prefix];
}

const KisMetaData::Schema* Store::createSchema(const KisMetaData::Schema* schema)
{
    return createSchema(schema->uri(), schema->prefix());
}

const Schema* Store::createSchema(QString uri, QString prefix)
{
    // First search for the schema
    const Schema* schema = schemaFromUri(uri);
    if(schema)
    {
        return schema;
    }
    // Second search for the prefix
    schema = schemaFromPrefix(prefix);
    if(schema)
    {
        return 0; // A schema with the same prefix allready exist
    }
    // The schema doesn't exist yet, create it
    Schema* nschema = new Schema(uri, prefix);
    d->uri2Schema[uri] = nschema;
    d->prefix2Schema[prefix] = nschema;
    return nschema;
}

void Store::debugDump() const
{
    kDebug() << "=== Dumping MetaData Store ===" << endl;
    kDebug() << " - Schemas " << endl;
    foreach(Schema* s, d->uri2Schema)
    {
        kDebug() << *s << endl;
    }
    kDebug() << " - Metadata (there are " << d->entries.size() << " entries)" << endl;
    foreach(const Entry& e, d->entries)
    {
        kDebug() << e << endl;
    }
}
