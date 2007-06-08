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

#include "kis_meta_data_schema.h"

#include <QDebug>
#include <QString>

using namespace KisMetaData;

const QString Schema::TIFFSchemaUri = "http://ns.adobe.com/tiff/1.0/";
const QString Schema::EXIFSchemaUri = "http://ns.adobe.com/exif/1.0/";
const QString Schema::DublinCoreSchemaUri = "http://purl.org/dc/elements/1.1/";
const QString Schema::XMPSchemaUri = "http://ns.adobe.com/xap/1.0/";
const QString Schema::MakerNoteSchemaUri = "http://create.freedesktop.org/xmp/MakerNote/1.0"; // TODO: make it a Create standard

struct Schema::Private {
    QString uri;
    QString prefix;
};

Schema::Schema(QString _uri, QString _ns) : d(new Private)
{
    d->uri = _uri;
    d->prefix = _ns;
}

QString Schema::uri() const
{
    return d->uri;
}

QString Schema::prefix() const
{
    return d->prefix;
}

QDebug operator<<(QDebug dbg, const KisMetaData::Schema &c)
{
    dbg.nospace() << "Uri = " << c.uri() << " Prefix = " << c.prefix();
    return dbg.space();
}

// ---- Schema Registry ---- //

struct SchemaRegistry::Private {
    static SchemaRegistry *singleton;
    QHash<QString, Schema*> uri2Schema;
    QHash<QString, Schema*> prefix2Schema;
};

SchemaRegistry *SchemaRegistry::Private::singleton = 0;

SchemaRegistry* SchemaRegistry::instance()
{
    if(SchemaRegistry::Private::singleton == 0)
    {
        SchemaRegistry::Private::singleton = new SchemaRegistry();
    }
    return SchemaRegistry::Private::singleton;
}

SchemaRegistry::SchemaRegistry() : d(new Private)
{
    create( Schema::TIFFSchemaUri, "tiff");
    create( Schema::EXIFSchemaUri, "exif");
    create( Schema::DublinCoreSchemaUri, "dc");
    create( Schema::XMPSchemaUri, "xmp");
    create( Schema::MakerNoteSchemaUri, "mkn");
}


const Schema* SchemaRegistry::schemaFromUri(QString uri) const
{
    return d->uri2Schema[uri];
}

const Schema* SchemaRegistry::schemaFromPrefix(QString prefix) const
{
    return d->prefix2Schema[prefix];
}

const Schema* SchemaRegistry::create(QString uri, QString prefix)
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

