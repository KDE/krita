/*
 *  Copyright (c) 2007,2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_meta_data_schema_registry.h"

#include <QString>

#include <kcomponentdata.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include "kis_debug.h"
#include "kis_meta_data_schema_p.h"

using namespace KisMetaData;

// ---- Schema Registry ---- //

struct SchemaRegistry::Private {
    static SchemaRegistry *singleton;
    QHash<QString, Schema*> uri2Schema;
    QHash<QString, Schema*> prefix2Schema;
};

SchemaRegistry *SchemaRegistry::Private::singleton = 0;

SchemaRegistry* SchemaRegistry::instance()
{
    if (SchemaRegistry::Private::singleton == 0) {
        SchemaRegistry::Private::singleton = new SchemaRegistry();
    }
    return SchemaRegistry::Private::singleton;
}

SchemaRegistry::SchemaRegistry() : d(new Private)
{

    KGlobal::mainComponent().dirs()->addResourceType("metadata_schema", "data", "krita/metadata/schemas/");

    QStringList schemasFilenames;
    schemasFilenames += KGlobal::mainComponent().dirs()->findAllResources("metadata_schema", "*.schema");

    foreach(const QString& fileName, schemasFilenames) {
        Schema* schema = new Schema();
        schema->d->load(fileName);
        if (schemaFromUri(schema->uri())) {
            errImage << "Schema already exist uri: " << schema->uri();
        } else if (schemaFromPrefix(schema->prefix())) {
            errImage << "Schema already exist prefix: " << schema->prefix();
        } else {
            d->uri2Schema[schema->uri()] = schema;
            d->prefix2Schema[schema->prefix()] = schema;
        }
    }

    // DEPRECATED WRITE A SCHEMA FOR EACH OF THEM
    create(Schema::XMPMediaManagementUri, "xmpMM");
    create(Schema::MakerNoteSchemaUri, "mkn");
    create(Schema::IPTCSchemaUri, "Iptc4xmpCore");
    create(Schema::PhotoshopSchemaUri, "photoshop");
}


const Schema* SchemaRegistry::schemaFromUri(const QString & uri) const
{
    return d->uri2Schema[uri];
}

const Schema* SchemaRegistry::schemaFromPrefix(const QString & prefix) const
{
    return d->prefix2Schema[prefix];
}

const Schema* SchemaRegistry::create(const QString & uri, const QString & prefix)
{
    // First search for the schema
    const Schema* schema = schemaFromUri(uri);
    if (schema) {
        return schema;
    }
    // Second search for the prefix
    schema = schemaFromPrefix(prefix);
    if (schema) {
        return 0; // A schema with the same prefix already exist
    }
    // The schema doesn't exist yet, create it
    Schema* nschema = new Schema(uri, prefix);
    d->uri2Schema[uri] = nschema;
    d->prefix2Schema[prefix] = nschema;
    return nschema;
}

