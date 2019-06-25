/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2017 Victor Wåhlström <victor.wahlstrom@initiali.se>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_heightmap_export.h"

#include <qendian.h>
#include <QDataStream>
#include <QApplication>

#include <kpluginfactory.h>

#include <KoColorSpace.h>
#include <KoColorSpaceConstants.h>
#include <KoColorSpaceTraits.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>

#include <KisImportExportManager.h>
#include <KisExportCheckRegistry.h>

#include <KisDocument.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_properties_configuration.h>
#include <kis_config.h>
#include <kis_iterator_ng.h>
#include <kis_random_accessor_ng.h>
#include <kis_config_widget.h>

#include "kis_wdg_options_heightmap.h"
#include "kis_heightmap_utils.h"

K_PLUGIN_FACTORY_WITH_JSON(KisHeightMapExportFactory, "krita_heightmap_export.json", registerPlugin<KisHeightMapExport>();)

template<typename T>
static void writeData(KisPaintDeviceSP pd, const QRect &bounds, QDataStream &out_stream)
{
    KIS_ASSERT_RECOVER_RETURN(pd);

    KisSequentialConstIterator it(pd, bounds);
    while (it.nextPixel()) {
        out_stream << KoGrayTraits<T>::gray(const_cast<quint8*>(it.rawDataConst()));
    }
}

KisHeightMapExport::KisHeightMapExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisHeightMapExport::~KisHeightMapExport()
{
}

KisPropertiesConfigurationSP KisHeightMapExport::defaultConfiguration(const QByteArray &from, const QByteArray &to) const
{
    Q_UNUSED(from);
    Q_UNUSED(to);
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("endianness", 0);
    return cfg;
}

KisConfigWidget *KisHeightMapExport::createConfigurationWidget(QWidget *parent, const QByteArray &from, const QByteArray &to) const
{
    Q_UNUSED(from);
    Q_UNUSED(to);
    bool export_mode = true;
    KisWdgOptionsHeightmap* wdg = new KisWdgOptionsHeightmap(parent, export_mode);
    return wdg;
}

void KisHeightMapExport::initializeCapabilities()
{
    if (mimeType() == "image/x-r8") {
        QList<QPair<KoID, KoID> > supportedColorModels;
        supportedColorModels << QPair<KoID, KoID>()
                << QPair<KoID, KoID>(GrayAColorModelID, Integer8BitsColorDepthID);
        addSupportedColorModels(supportedColorModels, "R8 Heightmap");
    }
    else if (mimeType() == "image/x-r16") {
        QList<QPair<KoID, KoID> > supportedColorModels;
        supportedColorModels << QPair<KoID, KoID>()
                << QPair<KoID, KoID>(GrayAColorModelID, Integer16BitsColorDepthID);
        addSupportedColorModels(supportedColorModels, "R16 Heightmap");
    }
    else if (mimeType() == "image/x-r32") {
        QList<QPair<KoID, KoID> > supportedColorModels;
        supportedColorModels << QPair<KoID, KoID>()
                << QPair<KoID, KoID>(GrayAColorModelID, Float32BitsColorDepthID);
        addSupportedColorModels(supportedColorModels, "R32 Heightmap");
    }
}

KisImportExportErrorCode KisHeightMapExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration)
{
    KIS_ASSERT_RECOVER_RETURN_VALUE(mimeType() == "image/x-r16" || mimeType() == "image/x-r8" || mimeType() == "image/x-r32", ImportExportCodes::FileFormatIncorrect);

    KisImageSP image = document->savingImage();
    QDataStream::ByteOrder bo = configuration->getInt("endianness", 1) == 0 ? QDataStream::BigEndian : QDataStream::LittleEndian;

    KisPaintDeviceSP pd = new KisPaintDevice(*image->projection());

    QDataStream s(io);
    s.setByteOrder(bo);
    // needed for 32bit float data
    s.setFloatingPointPrecision(QDataStream::SinglePrecision);

    KoID target_co_model = GrayAColorModelID;
    KoID target_co_depth = KisHeightmapUtils::mimeTypeToKoID(mimeType());
    KIS_ASSERT(!target_co_depth.id().isNull());

    if (pd->colorSpace()->colorModelId() != target_co_model || pd->colorSpace()->colorDepthId() != target_co_depth) {
        pd = new KisPaintDevice(*pd.data());
        pd->convertTo(KoColorSpaceRegistry::instance()->colorSpace(target_co_model.id(), target_co_depth.id()));
    }

    if (target_co_depth == Float32BitsColorDepthID) {
        writeData<float>(pd, image->bounds(), s);
    }
    else if (target_co_depth == Integer16BitsColorDepthID) {
        writeData<quint16>(pd, image->bounds(), s);
    }
    else if (target_co_depth == Integer8BitsColorDepthID) {
        writeData<quint8>(pd, image->bounds(), s);
    }
    else {
        KIS_ASSERT_RECOVER_RETURN_VALUE(true, ImportExportCodes::InternalError);
        return ImportExportCodes::InternalError;
    }
    return ImportExportCodes::OK;
}

#include "kis_heightmap_export.moc"
