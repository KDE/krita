/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_jpeg_import.h"

#include <QFileInfo>

#include <kpluginfactory.h>

#include <KisDocument.h>
#include <kis_image.h>
#include <KisViewManager.h>
#include <KisImportExportManager.h>

#include "kis_jpeg_converter.h"

K_PLUGIN_FACTORY_WITH_JSON(JPEGImportFactory, "krita_jpeg_import.json", registerPlugin<KisJPEGImport>();)

KisJPEGImport::KisJPEGImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisJPEGImport::~KisJPEGImport()
{
}

KisImportExportErrorCode KisJPEGImport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    KisJPEGConverter ib(document, batchMode());
    KisImportExportErrorCode result = ib.buildImage(io);
    if (result.isOk()) {
        document->setCurrentImage(ib.image());
    }
    return result;
}

#include <kis_jpeg_import.moc>

