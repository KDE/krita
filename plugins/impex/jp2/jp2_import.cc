/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "jp2_import.h"

#include <kpluginfactory.h>
#include <QFileInfo>

#include <KisDocument.h>
#include <kis_image.h>

#include "jp2_converter.h"

K_PLUGIN_FACTORY_WITH_JSON(ImportFactory, "krita_jp2_import.json", registerPlugin<jp2Import>();)

jp2Import::jp2Import(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

jp2Import::~jp2Import()
{
}

KisImportExportErrorCode jp2Import::convert(KisDocument *document, QIODevice */*io*/,  KisPropertiesConfigurationSP /*configuration*/)
{
    JP2Converter converter(document);
    KisImportExportErrorCode result = converter.buildImage(filename());
    if (result.isOk()) {
        document->setCurrentImage(converter.image());
    }
    return result;
}

#include <jp2_import.moc>

