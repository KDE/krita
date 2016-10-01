/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "psd_export.h"

#include <QCheckBox>
#include <QSlider>
#include <QFileInfo>
#include <QApplication>

#include <kpluginfactory.h>

#include <KisExportCheckRegistry.h>
#include <KisImportExportManager.h>
#include <ImageSizeCheck.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceConstants.h>

#include <KisDocument.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>

#include "psd_saver.h"

class KisExternalLayer;

K_PLUGIN_FACTORY_WITH_JSON(ExportFactory, "krita_psd_export.json", registerPlugin<psdExport>();)

bool checkHomogenity(KisNodeSP root, const KoColorSpace* cs)
{
    bool res = true;
    KisNodeSP child = root->firstChild();
    while (child) {
        if (child->childCount() > 0) {
            res = checkHomogenity(child, cs);
            if (res == false) {
                break;
            }
        }
        KisLayer *layer = dynamic_cast<KisLayer*>(child.data());
        if (layer) {
            if (layer->colorSpace() != cs) {
                res = false;
                break;
            }
        }
        child = child->nextSibling();
    }
    return res;
}

psdExport::psdExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

psdExport::~psdExport()
{
}

KisImportExportFilter::ConversionStatus psdExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    PSDSaver psdSaver(document);
    KisImageBuilder_Result res;

    if ((res = psdSaver.buildFile(io)) == KisImageBuilder_RESULT_OK) {
        dbgFile <<"success !";
        return KisImportExportFilter::OK;
    }
    dbgFile <<" Result =" << res;
    return KisImportExportFilter::InternalError;
}

void psdExport::initializeCapabilities()
{
    addCapability(KisExportCheckRegistry::instance()->get("PSDLayerStyleCheck")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("NodeTypeCheck/KisGroupLayer")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("MultiLayerCheck")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("sRGBProfileCheck")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("MultiLayerCheck")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("ColorModelHomogenousCheck")->create(KisExportCheckBase::UNSUPPORTED));

    ImageSizeCheckFactory *factory = dynamic_cast<ImageSizeCheckFactory*>(KisExportCheckRegistry::instance()->get("ImageSizeCheck"));
    if (factory) {
        addCapability(factory->create(30000, 30000, KisExportCheckBase::SUPPORTED));
    }

    QList<KoID> supportedColorModels;
    supportedColorModels << RGBAColorModelID << GrayAColorModelID << LABAColorModelID << CMYKAColorModelID;
    QList<KoID> supportedColorDepths;
    supportedColorDepths << Float32BitsColorDepthID << Float16BitsColorDepthID << Integer8BitsColorDepthID << Integer16BitsColorDepthID;

    QList<KoID> allColorModels = KoColorSpaceRegistry::instance()->colorModelsList(KoColorSpaceRegistry::AllColorSpaces);
    Q_FOREACH(const KoID &colorModelID, allColorModels) {
        QList<KoID> allColorDepths = KoColorSpaceRegistry::instance()->colorDepthList(colorModelID.id(), KoColorSpaceRegistry::AllColorSpaces);
        Q_FOREACH(const KoID &colorDepthID, allColorDepths) {

            KisExportCheckFactory *f1 = KisExportCheckRegistry::instance()->get("ColorModelCheck/" + colorModelID.id() + "/" + colorDepthID.id());
            KisExportCheckFactory *f2 = KisExportCheckRegistry::instance()->get("ColorModelPerLayerCheck/" + colorModelID.id() + "/" + colorDepthID.id());

            if(!f1 || !f2) {
                qDebug() << "No factory for" << colorModelID << colorDepthID;
                continue;
            }

            if (supportedColorModels.contains(colorModelID) && supportedColorDepths.contains(colorDepthID)) {
                addCapability(f1->create(KisExportCheckBase::SUPPORTED));
                addCapability(f2->create(KisExportCheckBase::SUPPORTED));
            }
            else {
                addCapability(f1->create(KisExportCheckBase::UNSUPPORTED,
                                         i18nc("image conversion warning",
                                               "EXR cannot save images with color model <b>%1</b> and depth <b>%2</b>. The image will not be saved.",
                                               colorModelID.name(),
                                               colorDepthID.name())));

                addCapability(f2->create(KisExportCheckBase::UNSUPPORTED,
                                        i18nc("image conversion warning",
                                              "EXR cannot save layers with color model <b>%1</b> and depth <b>%2</b>. The layers will be skipped.",
                                              colorModelID.name(),
                                              colorDepthID.name())));
            }
        }
    }

}

#include <psd_export.moc>

