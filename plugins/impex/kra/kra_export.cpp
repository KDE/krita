/*
 * Copyright (C) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kra_export.h"

#include <QCheckBox>
#include <QSlider>
#include <QMessageBox>

#include <kpluginfactory.h>
#include <QFileInfo>
#include <QApplication>

#include <KisFilterChain.h>
#include <KisImportExportManager.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>

#include <KisDocument.h>
#include <kis_image.h>
#include <kis_node.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_shape_layer.h>
#include <KoProperties.h>

#include "kra_converter.h"

class KisExternalLayer;

K_PLUGIN_FACTORY_WITH_JSON(ExportFactory, "krita_kra_export.json", registerPlugin<KraExport>();)

KraExport::KraExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KraExport::~KraExport()
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

KisImportExportFilter::ConversionStatus KraExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    KisImageWSP image = document->image();
    Q_CHECK_PTR(image);

    KisPaintDeviceSP pd = image->projection();
    QStringList supportedColorModelIds;
    supportedColorModelIds << RGBAColorModelID.id() << GrayAColorModelID.id() << GrayColorModelID.id();
    QStringList supportedColorDepthIds;
    supportedColorDepthIds << Integer8BitsColorDepthID.id() << Integer16BitsColorDepthID.id();
    if (!supportedColorModelIds.contains(pd->colorSpace()->colorModelId().id()) ||
            !supportedColorDepthIds.contains(pd->colorSpace()->colorDepthId().id())) {
        if (!getBatchMode()) {
            QMessageBox::critical(0, i18nc("@title:window", "Krita OpenRaster Export"), i18n("Cannot export images in this colorspace or channel depth to OpenRaster"));
        }
        return KisImportExportFilter::UsageError;
    }


    if (hasShapeLayerChild(image->root()) && !getBatchMode()) {
        QMessageBox::information(0,
                                 i18nc("@title:window", "Krita:Warning"),
                                 i18n("This image contains vector, clone or fill layers.\nThese layers will be saved as raster layers."));
    }

    KraConverter kraConverter(document);

    KisImageBuilder_Result res;

    if ((res = kraConverter.buildFile(io, image, document->activeNodes())) == KisImageBuilder_RESULT_OK) {
        dbgFile << "success !";
        return KisImportExportFilter::OK;
    }
    dbgFile << " Result =" << res;
    return KisImportExportFilter::InternalError;
}

#include <kra_export.moc>

