/*
 *  SPDX-FileCopyrightText: 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_png_import.h"

#include <kpluginfactory.h>
#include <QFileInfo>

#include <KisImportExportManager.h>

#include <KisDocument.h>
#include <kis_image.h>

#include <KisViewManager.h>

#include "kis_png_converter.h"

K_PLUGIN_FACTORY_WITH_JSON(PNGImportFactory, "krita_png_import.json", registerPlugin<KisPNGImport>();)

KisPNGImport::KisPNGImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisPNGImport::~KisPNGImport()
{
}

KisImportExportErrorCode KisPNGImport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    KisPNGConverter ib(document, batchMode());
    KisImportExportErrorCode res = ib.buildImage(io);
    if (res.isOk()){
        document->setCurrentImage(ib.image());
    }
    return res;

}

#include <kis_png_import.moc>

