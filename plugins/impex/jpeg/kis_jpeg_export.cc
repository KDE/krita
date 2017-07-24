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

#include "kis_jpeg_export.h"

#include <QCheckBox>
#include <QSlider>
#include <QColor>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>

#include <kpluginfactory.h>

#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoColorSpaceConstants.h>
#include <KoColorSpaceRegistry.h>

#include <KisImportExportManager.h>
#include <kis_slider_spin_box.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>
#include <kis_properties_configuration.h>
#include <kis_config.h>
#include <metadata/kis_meta_data_store.h>
#include <metadata/kis_meta_data_filter_registry_model.h>
#include <metadata/kis_exif_info_visitor.h>
#include <generator/kis_generator_layer.h>
#include <KisImportExportManager.h>
#include <KisExportCheckRegistry.h>
#include "kis_jpeg_converter.h"

class KisExternalLayer;

K_PLUGIN_FACTORY_WITH_JSON(KisJPEGExportFactory, "krita_jpeg_export.json", registerPlugin<KisJPEGExport>();)

KisJPEGExport::KisJPEGExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisJPEGExport::~KisJPEGExport()
{
}

KisImportExportFilter::ConversionStatus KisJPEGExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration)
{
    KisImageSP image = document->savingImage();
    Q_CHECK_PTR(image);

    // An extra option to pass to the config widget to set the state correctly, this isn't saved
    const KoColorSpace* cs = image->projection()->colorSpace();
    bool sRGB = cs->profile()->name().contains(QLatin1String("srgb"), Qt::CaseInsensitive);
    configuration->setProperty("is_sRGB", sRGB);

    KisJPEGOptions options;
    options.progressive = configuration->getBool("progressive", false);
    options.quality = configuration->getInt("quality", 80);
    options.forceSRGB = configuration->getBool("forceSRGB", false);
    options.saveProfile = configuration->getBool("saveProfile", true);
    options.optimize = configuration->getBool("optimize", true);
    options.smooth = configuration->getInt("smoothing", 0);
    options.baseLineJPEG = configuration->getBool("baseline", true);
    options.subsampling = configuration->getInt("subsampling", 0);
    options.exif = configuration->getBool("exif", true);
    options.iptc = configuration->getBool("iptc", true);
    options.xmp = configuration->getBool("xmp", true);
    KoColor c(KoColorSpaceRegistry::instance()->rgb8());
    c.fromQColor(Qt::white);
    options.transparencyFillColor = configuration->getColor("transparencyFillcolor", c).toQColor();
    KisMetaData::FilterRegistryModel m;
    m.setEnabledFilters(configuration->getString("filters").split(","));
    options.filters = m.enabledFilters();

    KisPaintDeviceSP pd = new KisPaintDevice(*image->projection());

    KisJPEGConverter kpc(document, batchMode());
    KisPaintLayerSP l = new KisPaintLayer(image, "projection", OPACITY_OPAQUE_U8, pd);

    KisExifInfoVisitor eIV;
    eIV.visit(image->rootLayer().data());

    KisMetaData::Store* eI = 0;
    if (eIV.countPaintLayer() == 1) {
        eI = eIV.exifInfo();
    }
    if (eI) {
        KisMetaData::Store* copy = new KisMetaData::Store(*eI);
        eI = copy;
    }

    KisImageBuilder_Result res = kpc.buildFile(io, l, options, eI);

    if (res == KisImageBuilder_RESULT_OK) {
        delete eI;
        return KisImportExportFilter::OK;
    }
    delete eI;
    dbgFile << " Result =" << res;
    return KisImportExportFilter::InternalError;
}

KisPropertiesConfigurationSP KisJPEGExport::defaultConfiguration(const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("progressive", false);
    cfg->setProperty("quality", 80);
    cfg->setProperty("forceSRGB", false);
    cfg->setProperty("saveProfile", true);
    cfg->setProperty("optimize", true);
    cfg->setProperty("smoothing", 0);
    cfg->setProperty("baseline", true);
    cfg->setProperty("subsampling", 0);
    cfg->setProperty("exif", true);
    cfg->setProperty("iptc", true);
    cfg->setProperty("xmp", true);

    KoColor fill_color(KoColorSpaceRegistry::instance()->rgb8());
    fill_color = KoColor();
    fill_color.fromQColor(Qt::white);
    QVariant v;
    v.setValue(fill_color);

    cfg->setProperty("transparencyFillcolor", v);
    cfg->setProperty("filters", "");

    return cfg;
}

KisConfigWidget *KisJPEGExport::createConfigurationWidget(QWidget *parent, const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    return new KisWdgOptionsJPEG(parent);
}

void KisJPEGExport::initializeCapabilities()
{
    addCapability(KisExportCheckRegistry::instance()->get("sRGBProfileCheck")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("ExifCheck")->create(KisExportCheckBase::SUPPORTED));

    QList<QPair<KoID, KoID> > supportedColorModels;
    supportedColorModels << QPair<KoID, KoID>()
            << QPair<KoID, KoID>(RGBAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(GrayAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(CMYKAColorModelID, Integer8BitsColorDepthID);
    addSupportedColorModels(supportedColorModels, "JPEG");
}


KisWdgOptionsJPEG::KisWdgOptionsJPEG(QWidget *parent)
    : KisConfigWidget(parent)
{
    setupUi(this);

    metaDataFilters->setModel(&m_filterRegistryModel);
    qualityLevel->setRange(0, 100, 0);
    qualityLevel->setSuffix("%");
    smoothLevel->setRange(0, 100, 0);
    smoothLevel->setSuffix("%");
}


void KisWdgOptionsJPEG::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    progressive->setChecked(cfg->getBool("progressive", false));
    qualityLevel->setValue(cfg->getInt("quality", 80));
    optimize->setChecked(cfg->getBool("optimize", true));
    smoothLevel->setValue(cfg->getInt("smoothing", 0));
    baseLineJPEG->setChecked(cfg->getBool("baseline", true));
    subsampling->setCurrentIndex(cfg->getInt("subsampling", 0));
    exif->setChecked(cfg->getBool("exif", true));
    iptc->setChecked(cfg->getBool("iptc", true));
    xmp->setChecked(cfg->getBool("xmp", true));
    chkForceSRGB->setVisible(cfg->getBool("is_sRGB"));
    chkForceSRGB->setChecked(cfg->getBool("forceSRGB", false));
    chkSaveProfile->setChecked(cfg->getBool("saveProfile", true));
    KoColor background(KoColorSpaceRegistry::instance()->rgb8());
    background.fromQColor(Qt::white);
    bnTransparencyFillColor->setDefaultColor(background);
    bnTransparencyFillColor->setColor(cfg->getColor("transparencyFillcolor", background));

    m_filterRegistryModel.setEnabledFilters(cfg->getString("filters").split(','));

}

KisPropertiesConfigurationSP KisWdgOptionsJPEG::configuration() const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();

    QVariant transparencyFillcolor;
    transparencyFillcolor.setValue(bnTransparencyFillColor->color());

    cfg->setProperty("progressive", progressive->isChecked());
    cfg->setProperty("quality", (int)qualityLevel->value());
    cfg->setProperty("forceSRGB", chkForceSRGB->isChecked());
    cfg->setProperty("saveProfile", chkSaveProfile->isChecked());
    cfg->setProperty("optimize", optimize->isChecked());
    cfg->setProperty("smoothing", (int)smoothLevel->value());
    cfg->setProperty("baseline", baseLineJPEG->isChecked());
    cfg->setProperty("subsampling", subsampling->currentIndex());
    cfg->setProperty("exif", exif->isChecked());
    cfg->setProperty("iptc", iptc->isChecked());
    cfg->setProperty("xmp", xmp->isChecked());
    cfg->setProperty("transparencyFillcolor", transparencyFillcolor);

    QString enabledFilters;
    Q_FOREACH (const KisMetaData::Filter* filter, m_filterRegistryModel.enabledFilters()) {
        enabledFilters = enabledFilters + filter->id() + ',';
    }
    cfg->setProperty("filters", enabledFilters);

    return cfg;
}
#include <kis_jpeg_export.moc>
