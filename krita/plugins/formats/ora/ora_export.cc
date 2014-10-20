/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
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

#include <kpluginfactory.h>
#include <kmessagebox.h>

#include <KoFilterChain.h>
#include <KoFilterManager.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>

#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_node.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_shape_layer.h>
#include <KoProperties.h>

#include "ora_converter.h"

class KisExternalLayer;

K_PLUGIN_FACTORY(ExportFactory, registerPlugin<OraExport>();)
K_EXPORT_PLUGIN(ExportFactory("calligrafilters"))

OraExport::OraExport(QObject *parent, const QVariantList &) : KoFilter(parent)
{
}

OraExport::~OraExport()
{
}


bool hasShapeLayerChild(KisNodeSP node)
{
    if (!node) return false;

    foreach(KisNodeSP child, node->childNodes(QStringList(), KoProperties())) {
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

KoFilter::ConversionStatus OraExport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile << "ORA export! From:" << from << ", To:" << to << "";

    if (from != "application/x-krita")
        return KoFilter::NotImplemented;

    KisDoc2 *input = dynamic_cast<KisDoc2*>(m_chain->inputDocument());
    QString filename = m_chain->outputFile();

    if (!input)
        return KoFilter::NoDocumentCreated;

    qApp->processEvents(); // For vector layers to be updated
    input->image()->waitForDone();

    if (filename.isEmpty()) return KoFilter::FileNotFound;

    KUrl url;
    url.setPath(filename);

    KisImageWSP image = input->image();
    Q_CHECK_PTR(image);

    KisPaintDeviceSP pd = image->projection();
    QStringList supportedColorModelIds;
    supportedColorModelIds << RGBAColorModelID.id() << GrayAColorModelID.id() << GrayColorModelID.id();
    QStringList supportedColorDepthIds;
    supportedColorDepthIds << Integer8BitsColorDepthID.id() << Integer16BitsColorDepthID.id();
    if (!supportedColorModelIds.contains(pd->colorSpace()->colorModelId().id()) ||
            !supportedColorDepthIds.contains(pd->colorSpace()->colorDepthId().id())) {
        if (!m_chain->manager()->getBatchMode()) {
            KMessageBox::error(0, i18n("Cannot export images in this colorspace or channel depth to OpenRaster"), i18n("Krita OpenRaster Export"));
        }
        return KoFilter::UsageError;
    }


    if (hasShapeLayerChild(image->root()) && !m_chain->manager()->getBatchMode()) {
        KMessageBox::information(0,
                                 i18n("This image contains vector, clone or fill layers.\nThese layers will be saved as raster layers."),
                                 i18n("Warning"),
                                 "krita/ora/vector");
    }

    OraConverter kpc(input);

    KisImageBuilder_Result res;

    if ((res = kpc.buildFile(url, image, input->activeNodes())) == KisImageBuilder_RESULT_OK) {
        dbgFile << "success !";
        return KoFilter::OK;
    }
    dbgFile << " Result =" << res;
    return KoFilter::InternalError;
}

#include <ora_export.moc>

