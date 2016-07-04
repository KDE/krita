/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_tiff_export.h"

#include <QCheckBox>
#include <QSlider>

#include <kpluginfactory.h>
#include <QFileInfo>

#include <KoDialog.h>
#include <KisFilterChain.h>
#include <KoColorSpace.h>
#include <KoChannelInfo.h>
#include <KoColorModelStandardIds.h>
#include <KisImportExportManager.h>

#include <KisDocument.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>
#include <kis_config.h>

#include "kis_tiff_converter.h"
#include "kis_dlg_options_tiff.h"
#include "ui_kis_wdg_options_tiff.h"

K_PLUGIN_FACTORY_WITH_JSON(KisTIFFExportFactory, "krita_tiff_export.json", registerPlugin<KisTIFFExport>();)

KisTIFFExport::KisTIFFExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisTIFFExport::~KisTIFFExport()
{
}

KisImportExportFilter::ConversionStatus KisTIFFExport::convert(const QByteArray& from, const QByteArray& to, KisPropertiesConfigurationSP configuration)
{
    dbgFile << "Tiff export! From:" << from << ", To:" << to << "";

    if (from != "application/x-krita")
        return KisImportExportFilter::NotImplemented;

    KisDocument *input = inputDocument();
    if (!input) {
        return KisImportExportFilter::NoDocumentCreated;
    }

    KoDialog kdb;
    kdb.setWindowTitle(i18n("TIFF Export Options"));
    kdb.setButtons(KoDialog::Ok | KoDialog::Cancel);
    KisTIFFOptionsWidget *wdg = static_cast<KisTIFFOptionsWidget*>(createConfigurationWidget(&kdb, from, to));
    kdb.setMainWidget(wdg);
    kdb.resize(kdb.minimumSize());

    // If a configuration object was passed to the convert method, we use that, otherwise we load from the settings
    KisPropertiesConfigurationSP cfg(new KisPropertiesConfiguration());
    if (configuration) {
        cfg->fromXML(configuration->toXML());
    }
    else {
        cfg = lastSavedConfiguration(from, to);
    }

    const KoColorSpace* cs = input->image()->colorSpace();
    cfg->setProperty("type", (int)cs->channels()[0]->channelValueType());
    cfg->setProperty("isCMYK", (cs->colorModelId() == CMYKAColorModelID));

    wdg->setConfiguration(cfg);

    if (!getBatchMode()) {
        if (kdb.exec() == QDialog::Rejected) {
            return KisImportExportFilter::UserCancelled;
        }
        cfg = wdg->configuration();
        KisConfig().setExportConfiguration("TIFF", *cfg.data());
    }

    KisTIFFOptions options = wdg->options();

    if ((cs->channels()[0]->channelValueType() == KoChannelInfo::FLOAT16
         || cs->channels()[0]->channelValueType() == KoChannelInfo::FLOAT32) && options.predictor == 2) {
        // FIXME THIS IS AN HACK FIX THAT IN 2.0 !! (62456a7b47636548c6507593df3e2bdf440f7544, BUG:135649)
        options.predictor = 3;
    }

    QString filename = outputFile();

    if (filename.isEmpty()) {
        return KisImportExportFilter::FileNotFound;
    }

    KisImageSP image;

    if (options.flatten) {
        image = new KisImage(0, input->image()->width(), input->image()->height(), input->image()->colorSpace(), "");
        image->setResolution(input->image()->xRes(), input->image()->yRes());
        KisPaintDeviceSP pd = KisPaintDeviceSP(new KisPaintDevice(*input->image()->projection()));
        KisPaintLayerSP l = KisPaintLayerSP(new KisPaintLayer(image.data(), "projection", OPACITY_OPAQUE_U8, pd));
        image->addNode(KisNodeSP(l.data()), image->rootLayer().data());
    } else {
        image = input->image();
    }

    // the image must be locked at the higher levels
    KIS_SAFE_ASSERT_RECOVER_NOOP(input->image()->locked());

    KisTIFFConverter ktc(input);
    KisImageBuilder_Result res;
    if ((res = ktc.buildFile(filename, image, options)) == KisImageBuilder_RESULT_OK) {
        dbgFile << "success !";
        return KisImportExportFilter::OK;
    }

    dbgFile << " Result =" << res;
    return KisImportExportFilter::InternalError;
}

KisPropertiesConfigurationSP KisTIFFExport::defaultConfiguration(const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("compressiontype", 0);
    cfg->setProperty("predictor", 0);
    cfg->setProperty("alpha", true);
    cfg->setProperty("flatten", true);
    cfg->setProperty("quality", 80);
    cfg->setProperty("deflate", 6);
    cfg->setProperty("faxmode", 0);
    cfg->setProperty("pixarlog", 6);
    cfg->setProperty("saveProfile", true);

    return cfg;
}

KisPropertiesConfigurationSP KisTIFFExport::lastSavedConfiguration(const QByteArray &from, const QByteArray &to) const
{
    QString filterConfig = KisConfig().exportConfiguration("TIFF");
    KisPropertiesConfigurationSP cfg = defaultConfiguration(from, to);
    cfg->fromXML(filterConfig, false);
    return cfg;
}

KisConfigWidget *KisTIFFExport::createConfigurationWidget(QWidget *parent, const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    return new KisTIFFOptionsWidget(parent);
}

#include <kis_tiff_export.moc>

