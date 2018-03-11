/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#include "kis_meta_data_filter_p.h"
#include <QDate>

#include <klocalizedstring.h>
#include <KritaVersionWrapper.h>

#include "kis_meta_data_entry.h"
#include "kis_meta_data_schema.h"
#include "kis_meta_data_schema_registry.h"
#include "kis_meta_data_store.h"
#include "kis_meta_data_value.h"

#include "kis_debug.h"

using namespace KisMetaData;

AnonymizerFilter::~AnonymizerFilter()
{
}

bool AnonymizerFilter::defaultEnabled() const
{
    return false;
}

QString AnonymizerFilter::id() const
{
    return "Anonymizer";
}

QString AnonymizerFilter::name() const
{
    return i18n("Anonymizer");
}

QString AnonymizerFilter::description() const
{
    return i18n("Remove personal information: author, location...");
}

void AnonymizerFilter::filter(KisMetaData::Store* store) const
{
    dbgMetaData << "Anonymize a store";
    const KisMetaData::Schema* dcSchema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::DublinCoreSchemaUri);
    store->removeEntry(dcSchema, "contributor");
    store->removeEntry(dcSchema, "creator");
    store->removeEntry(dcSchema, "publisher");
    store->removeEntry(dcSchema, "rights");

    const KisMetaData::Schema* psSchema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::PhotoshopSchemaUri);
    store->removeEntry(psSchema, "AuthorsPosition");
    store->removeEntry(psSchema, "CaptionWriter");
    store->removeEntry(psSchema, "Credit");
    store->removeEntry(psSchema, "City");
    store->removeEntry(psSchema, "Country");
}

//------------------------------------//
//---------- ToolInfoFilter ----------//
//------------------------------------//

ToolInfoFilter::~ToolInfoFilter()
{
}

bool ToolInfoFilter::defaultEnabled() const
{
    return true;
}

QString ToolInfoFilter::id() const
{
    return "ToolInfo";
}

QString ToolInfoFilter::name() const
{
    return i18n("Tool information");
}

QString ToolInfoFilter::description() const
{
    return i18n("Add the name of the tool used for creation and the modification date");
}

void ToolInfoFilter::filter(KisMetaData::Store* store) const
{
    const KisMetaData::Schema* xmpSchema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::XMPSchemaUri);
    store->getEntry(xmpSchema, "ModifyDate").value() = Value(QDate::currentDate());
    store->getEntry(xmpSchema, "MetadataDate").value() = Value(QDate::currentDate());
    if (!store->containsEntry(xmpSchema, "CreatorTool")) {
        store->getEntry(xmpSchema, "CreatorTool").value() = Value(i18n("Krita %1", KritaVersionWrapper::versionString()));
    }
}
