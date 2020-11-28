/*
 *  SPDX-FileCopyrightText: 2016 Laszlo Fazekas <mneko@freemail.hu>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_csv_export.h"

#include <QCheckBox>
#include <QSlider>

#include <kpluginfactory.h>
#include <QFileInfo>
#include <QApplication>

#include <KisExportCheckRegistry.h>
#include <KisImportExportManager.h>
#include <KoColorSpaceConstants.h>

#include <KisDocument.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>

#include "csv_saver.h"

K_PLUGIN_FACTORY_WITH_JSON(KisCSVExportFactory, "krita_csv_export.json", registerPlugin<KisCSVExport>();)

KisCSVExport::KisCSVExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisCSVExport::~KisCSVExport()
{
}

KisImportExportErrorCode KisCSVExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    CSVSaver kpc(document, batchMode());

    KisImportExportErrorCode res = kpc.buildAnimation(io);
    return res;
}

void KisCSVExport::initializeCapabilities()
{
    addCapability(KisExportCheckRegistry::instance()->get("MultiLayerCheck")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("AnimationCheck")->create(KisExportCheckBase::SUPPORTED));
    QList<QPair<KoID, KoID> > supportedColorModels;
    supportedColorModels << QPair<KoID, KoID>()
            << QPair<KoID, KoID>(RGBAColorModelID, Integer8BitsColorDepthID);
    addSupportedColorModels(supportedColorModels, "CSV");
    addCapability(KisExportCheckRegistry::instance()->get("ColorModelPerLayerCheck/" + RGBAColorModelID.id() + "/" + Integer8BitsColorDepthID.id())->create(KisExportCheckBase::SUPPORTED));
}

#include "kis_csv_export.moc"
