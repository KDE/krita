/*
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "qml_export.h"

#include <QCheckBox>
#include <QSlider>

#include <kpluginfactory.h>
#include <QFileInfo>
#include <QApplication>

#include <KisExportCheckRegistry.h>
#include <KisDocument.h>
#include <kis_image.h>

#include "qml_converter.h"
#include <KoColorModelStandardIds.h>

K_PLUGIN_FACTORY_WITH_JSON(ExportFactory, "krita_qml_export.json", registerPlugin<QMLExport>();)

QMLExport::QMLExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

QMLExport::~QMLExport()
{
}

KisImportExportErrorCode QMLExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    KisImageSP image = document->savingImage();
    Q_CHECK_PTR(image);

    QMLConverter converter;
    return converter.buildFile(filename(), realFilename(), io, image);
}

void QMLExport::initializeCapabilities()
{
    addCapability(KisExportCheckRegistry::instance()->get("MultiLayerCheck")->create(KisExportCheckBase::SUPPORTED));

    QList<QPair<KoID, KoID> > supportedColorModels;
    supportedColorModels << QPair<KoID, KoID>()
            << QPair<KoID, KoID>(RGBAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(GrayAColorModelID, Integer8BitsColorDepthID);
    addSupportedColorModels(supportedColorModels, "QML");
    }



#include <qml_export.moc>

