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

#include <QString>

#include "kis_debug.h"

using namespace KisMetaData;

const QString Schema::TIFFSchemaUri = "http://ns.adobe.com/tiff/1.0/";
const QString Schema::EXIFSchemaUri = "http://ns.adobe.com/exif/1.0/";
const QString Schema::DublinCoreSchemaUri = "http://purl.org/dc/elements/1.1/";
const QString Schema::XMPSchemaUri = "http://ns.adobe.com/xap/1.0/";
const QString Schema::XMPRightsSchemaUri = "http://ns.adobe.com/xap/1.0/rights/";
const QString Schema::XMPMediaManagementUri = "http://ns.adobe.com/xap/1.0/sType/ResourceRef#";
const QString Schema::MakerNoteSchemaUri = "http://www.koffice.org/krita/xmp/MakerNote/1.0";
const QString Schema::IPTCSchemaUri = "http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/";
const QString Schema::PhotoshopSchemaUri = "http://ns.adobe.com/photoshop/1.0/";



struct Schema::Private {
    QString uri;
    QString prefix;
};

Schema::Schema(const QString & _uri, const QString & _ns)
        : d(new Private)
{
    d->uri = _uri;
    d->prefix = _ns;
}

Schema::~Schema()
{
    dbgImage << "Deleting schema " << d->uri << " " << d->prefix;
    dbgImage << kBacktrace();
    delete d;
}

QString Schema::uri() const
{
    return d->uri;
}

QString Schema::prefix() const
{
    return d->prefix;
}

QString Schema::generateQualifiedName(const QString & name) const
{
    dbgImage << "generateQualifiedName for " << name;
    Q_ASSERT(!name.isEmpty() && !name.isNull());
    return prefix() + ':' + name;
}

QDebug operator<<(QDebug debug, const KisMetaData::Schema &c)
{
    debug.nospace() << "Uri = " << c.uri() << " Prefix = " << c.prefix();
    return debug.space();
}
