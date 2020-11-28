/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "ora_export.h"

#include <QCheckBox>
#include <QSlider>

#include <kpluginfactory.h>
#include <QFileInfo>
#include <QApplication>
#include <KoStore.h>
#include <KisImportExportManager.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>
#include <KisExportCheckRegistry.h>

#include <KisDocument.h>
#include <kis_image.h>
#include <kis_node.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_shape_layer.h>
#include <KoProperties.h>

#include "ora_converter.h"

class KisExternalLayer;

K_PLUGIN_FACTORY_WITH_JSON(ExportFactory, "krita_ora_export.json", registerPlugin<OraExport>();)

OraExport::OraExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

OraExport::~OraExport()
{
}


bool hasShapeLayerChild(KisNodeSP node)
{
    if (!node) return false;

    Q_FOREACH (KisNodeSP child, node->childNodes(QStringList(), KoProperties())) {
        if (child->inherits("KisShapeLayer")
                || child->inherits("KisGeneratorLayer")
                || child->inherits("KisCloneLayer")) {
            return true;
        }
        else {
            if (hasShapeLayerChild(child)) {
                return true;
            }
        }
    }
    return false;
}

KisImportExportErrorCode OraExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    KisImageSP image = document->savingImage();
    Q_CHECK_PTR(image);
    OraConverter oraConverter(document);

    KisImportExportErrorCode res = oraConverter.buildFile(io, image, {document->preActivatedNode()});
    return res;
}

void OraExport::initializeCapabilities()
{
    addCapability(KisExportCheckRegistry::instance()->get("MultiLayerCheck")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("NodeTypeCheck/KisGroupLayer")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("NodeTypeCheck/KisAdjustmentLayer")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("sRGBProfileCheck")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("ColorModelHomogenousCheck")->create(KisExportCheckBase::SUPPORTED));
    QList<QPair<KoID, KoID> > supportedColorModels;
    supportedColorModels << QPair<KoID, KoID>()
            << QPair<KoID, KoID>(RGBAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(RGBAColorModelID, Integer16BitsColorDepthID)
            << QPair<KoID, KoID>(GrayAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(GrayAColorModelID, Integer16BitsColorDepthID);
    addSupportedColorModels(supportedColorModels, "OpenRaster");
}

QString OraExport::verify(const QString &fileName) const
{
    QString error = KisImportExportFilter::verify(fileName);
    if (error.isEmpty()) {
        return KisImportExportFilter::verifyZiPBasedFiles(fileName,
                                                          QStringList()
                                                          << "mimetype"
                                                          << "stack.xml"
                                                          << "mergedimage.png");
    }

    return error;
}



#include <ora_export.moc>

