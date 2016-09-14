/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "ora_export.h"

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

KisImportExportFilter::ConversionStatus OraExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
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

    OraConverter oraConverter(document);

    KisImageBuilder_Result res;

    if ((res = oraConverter.buildFile(io, image, document->activeNodes())) == KisImageBuilder_RESULT_OK) {
        dbgFile << "success !";
        return KisImportExportFilter::OK;
    }
    dbgFile << " Result =" << res;
    return KisImportExportFilter::InternalError;
}

#include <ora_export.moc>

