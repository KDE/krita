/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
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

#include "ora_import.h"

#include <kpluginfactory.h>
#include <QFileInfo>

#include <KisDocument.h>
#include <kis_image.h>

#include "ora_converter.h"

K_PLUGIN_FACTORY_WITH_JSON(ImportFactory, "krita_ora_import.json", registerPlugin<OraImport>();)

OraImport::OraImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

OraImport::~OraImport()
{
}

KisImportExportErrorCode OraImport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    OraConverter oraConverter(document);
    KisImportExportErrorCode result = oraConverter.buildImage(io);
    if (result.isOk()) {
        document->setCurrentImage(oraConverter.image());
        if (oraConverter.activeNodes().size() > 0) {
            document->setPreActivatedNode(oraConverter.activeNodes()[0]);
        }
    }

    qDebug() << ">>>>>>>>>" << result;

    return result;
}

#include <ora_import.moc>

