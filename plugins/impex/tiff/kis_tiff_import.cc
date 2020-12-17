/*
 *  SPDX-FileCopyrightText: 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tiff_import.h"

#include <QFileInfo>

#include <kpluginfactory.h>

#include <KisDocument.h>
#include <kis_image.h>

#include <KisViewManager.h>

#include "kis_tiff_converter.h"

K_PLUGIN_FACTORY_WITH_JSON(TIFFImportFactory, "krita_tiff_import.json", registerPlugin<KisTIFFImport>();)

KisTIFFImport::KisTIFFImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisTIFFImport::~KisTIFFImport()
{
}

KisImportExportErrorCode KisTIFFImport::convert(KisDocument *document, QIODevice */*io*/,  KisPropertiesConfigurationSP /*configuration*/)
{
    KisTIFFConverter tiffConverter(document);
    KisImportExportErrorCode result = tiffConverter.buildImage(filename());
    if (result.isOk()) {
        document->setCurrentImage(tiffConverter.image());
    }
    return result;
}

#include <kis_tiff_import.moc>

