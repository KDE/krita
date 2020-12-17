/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
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

