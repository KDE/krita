/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_heightmap_export.h"

#include <qendian.h>
#include <QDataStream>
#include <QApplication>
#include <QMessageBox>

#include <kpluginfactory.h>

#include <KoColorSpace.h>
#include <KoColorSpaceConstants.h>
#include <KisImportExportManager.h>
#include <KoColorSpaceTraits.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>

#include <KoDialog.h>

#include <kis_debug.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_properties_configuration.h>
#include <kis_config.h>
#include <kis_iterator_ng.h>
#include <kis_random_accessor_ng.h>
#include <kis_config_widget.h>

K_PLUGIN_FACTORY_WITH_JSON(KisHeightMapExportFactory, "krita_heightmap_export.json", registerPlugin<KisHeightMapExport>();)

KisHeightMapExport::KisHeightMapExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisHeightMapExport::~KisHeightMapExport()
{
}

KisPropertiesConfigurationSP KisHeightMapExport::defaultConfiguration(const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("endianness", 0);
    return cfg;
}

KisPropertiesConfigurationSP KisHeightMapExport::lastSavedConfiguration(const QByteArray &from, const QByteArray &to) const
{
    KisPropertiesConfigurationSP cfg = defaultConfiguration(from, to);
    QString filterConfig = KisConfig().exportConfiguration("HeightMap");
    cfg->fromXML(filterConfig, false);
    return cfg;
}

KisConfigWidget *KisHeightMapExport::createConfigurationWidget(QWidget *parent, const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    return new KisWdgOptionsHeightmap(parent);
}

KisImportExportFilter::ConversionStatus KisHeightMapExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration)
{
    KisImageWSP image = document->image();

    if (document->image()->width() != document->image()->height()) {
        document->setErrorMessage(i18n("Cannot export this image to a heightmap: it is not square"));
        return KisImportExportFilter::WrongFormat;
    }

    if (document->image()->colorSpace()->colorModelId() != GrayAColorModelID) {
        document->setErrorMessage(i18n("Cannot export this image to a heightmap: it is not grayscale"));
        return KisImportExportFilter::WrongFormat;
    }

    KoDialog kdb;
    kdb.setWindowTitle(i18n("HeightMap Export Options"));
    kdb.setButtons(KoDialog::Ok | KoDialog::Cancel);
    KisConfigWidget *wdg = createConfigurationWidget(&kdb, KisDocument::nativeFormatMimeType(), mimeType());
    kdb.setMainWidget(wdg);

    QApplication::restoreOverrideCursor();

    // If a configuration object was passed to the convert method, we use that, otherwise we load from the settings
    KisPropertiesConfigurationSP cfg(new KisPropertiesConfiguration());
    if (configuration) {
        cfg->fromXML(configuration->toXML());
    }
    else {
        cfg = lastSavedConfiguration(KisDocument::nativeFormatMimeType(), mimeType());
    }
    cfg->setProperty("width", image->width());
    wdg->setConfiguration(cfg);

    if (!batchMode()) {
        if (kdb.exec() == QDialog::Rejected) {
            return KisImportExportFilter::UserCancelled;
        }
        cfg = wdg->configuration();
        KisConfig().setExportConfiguration("HeightMap", *cfg.data());
    }

    QDataStream::ByteOrder bo = cfg->getInt("endianness", 0) ? QDataStream::BigEndian : QDataStream::LittleEndian;

    bool downscale = false;
    if (mimeType() == "image/x-r8" && image->colorSpace()->colorDepthId() == Integer16BitsColorDepthID) {

        downscale = (QMessageBox::question(0,
                                           i18nc("@title:window", "Downscale Image"),
                                           i18n("You specified the .r8 extension for a 16 bit/channel image. Do you want to save as 8 bit? Your image data will not be changed."),
                                           QMessageBox::Yes | QMessageBox::No)
                     == QMessageBox::Yes);
    }


    // the image must be locked at the higher levels
    KIS_SAFE_ASSERT_RECOVER_NOOP(image->locked());
    KisPaintDeviceSP pd = new KisPaintDevice(*image->projection());

    QDataStream s(io);
    s.setByteOrder(bo);

    KisRandomConstAccessorSP it = pd->createRandomConstAccessorNG(0, 0);
    bool r16 = ((image->colorSpace()->colorDepthId() == Integer16BitsColorDepthID) && !downscale);
    for (int i = 0; i < image->height(); ++i) {
        for (int j = 0; j < image->width(); ++j) {
            it->moveTo(i, j);
            if (r16) {
                s << KoGrayU16Traits::gray(const_cast<quint8*>(it->rawDataConst()));
            }
            else {
                s << KoGrayU8Traits::gray(const_cast<quint8*>(it->rawDataConst()));
            }
        }
    }
    return KisImportExportFilter::OK;
}


void KisWdgOptionsHeightmap::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    intSize->setValue(cfg->getInt("width"));
    int endianness = cfg->getInt("endianness", 0);
    radioMac->setChecked(endianness == 0);
}

KisPropertiesConfigurationSP KisWdgOptionsHeightmap::configuration() const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    if (radioMac->isChecked()) {
        cfg->setProperty("endianness", 0);
    }
    else {
        cfg->setProperty("endianness", 1);
    }
    return cfg;
}

#include "kis_heightmap_export.moc"
