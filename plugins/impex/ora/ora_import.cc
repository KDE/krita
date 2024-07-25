/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
        KisNodeSP preActivatedNode = !oraConverter.activeNodes().isEmpty() ? oraConverter.activeNodes().first() : nullptr;
        document->setCurrentImage(oraConverter.image(), true, preActivatedNode);
    }
    return result;
}

#include <ora_import.moc>

