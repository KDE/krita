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

#include "kis_png_export.h"

#include <QCheckBox>
#include <QSlider>
#include <QApplication>

#include <kpluginfactory.h>

#include <KoColorSpace.h>
#include <KisImportExportManager.h>
#include <KoColorProfile.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpaceRegistry.h>

#include <KisExportCheckRegistry.h>

#include <kis_properties_configuration.h>
#include <kis_paint_device.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_config.h>
#include <kis_properties_configuration.h>
#include <metadata/kis_meta_data_store.h>
#include <metadata/kis_meta_data_filter_registry_model.h>
#include <metadata/kis_exif_info_visitor.h>
#include "kis_png_converter.h"
#include <kis_iterator_ng.h>

K_PLUGIN_FACTORY_WITH_JSON(KisPNGExportFactory, "krita_png_export.json", registerPlugin<KisPNGExport>();)

KisPNGExport::KisPNGExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisPNGExport::~KisPNGExport()
{
}

bool hasVisibleWidgets()
{
    QWidgetList wl = QApplication::allWidgets();
    Q_FOREACH (QWidget* w, wl) {
        if (w->isVisible() && strcmp(w->metaObject()->className(), "QDesktopWidget")) {
            dbgFile << "Widget " << w << " " << w->objectName() << " " << w->metaObject()->className() << " is visible";
            return true;
        }
    }
    return false;
}

KisImportExportFilter::ConversionStatus KisPNGExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration)
{
    KisImageSP image = document->savingImage();

    KisPNGOptions options;

    options.alpha = configuration->getBool("alpha", true);
    options.interlace = configuration->getBool("interlaced", false);
    options.compression = configuration->getInt("compression", 3);
    options.tryToSaveAsIndexed = configuration->getBool("indexed", false);
    options.transparencyFillColor = configuration->getColor("transparencyFillColor").toQColor();
    options.saveSRGBProfile = configuration->getBool("saveSRGBProfile", false);
    options.forceSRGB = configuration->getBool("forceSRGB", true);

    vKisAnnotationSP_it beginIt = image->beginAnnotations();
    vKisAnnotationSP_it endIt = image->endAnnotations();

    KisExifInfoVisitor eIV;
    eIV.visit(image->rootLayer().data());
    KisMetaData::Store *eI = 0;
    if (eIV.countPaintLayer() == 1) {
        eI = eIV.exifInfo();
    }
    if (eI) {
        KisMetaData::Store* copy = new KisMetaData::Store(*eI);
        eI = copy;
    }

    KisPNGConverter pngConverter(document);

    KisImageBuilder_Result res = pngConverter.buildFile(io, image->bounds(), image->xRes(), image->yRes(), image->projection(), beginIt, endIt, options, eI);

    if (res == KisImageBuilder_RESULT_OK) {
        delete eI;
        return KisImportExportFilter::OK;
    }

    delete eI;
    dbgFile << " Result =" << res;
    return KisImportExportFilter::InternalError;
}

KisPropertiesConfigurationSP KisPNGExport::defaultConfiguration(const QByteArray &, const QByteArray &) const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("alpha", true);
    cfg->setProperty("indexed", false);
    cfg->setProperty("compression", 3);
    cfg->setProperty("interlaced", false);
    cfg->setProperty("transparencyFillcolor", QString("255,255,255"));
    cfg->setProperty("saveSRGBProfile", false);
    cfg->setProperty("forceSRGB", true);

    return cfg;
}

KisConfigWidget *KisPNGExport::createConfigurationWidget(QWidget *parent, const QByteArray &, const QByteArray &) const
{
    return new KisWdgOptionsPNG(parent);
}

void KisPNGExport::initializeCapabilities()
{
    addCapability(KisExportCheckRegistry::instance()->get("sRGBProfileCheck")->create(KisExportCheckBase::SUPPORTED));
    QList<QPair<KoID, KoID> > supportedColorModels;
    supportedColorModels << QPair<KoID, KoID>()
            << QPair<KoID, KoID>(RGBAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(RGBAColorModelID, Integer16BitsColorDepthID)
            << QPair<KoID, KoID>(GrayAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(GrayAColorModelID, Integer16BitsColorDepthID);
    addSupportedColorModels(supportedColorModels, "PNG");
}

void KisWdgOptionsPNG::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    // the export manager should have prepared some info for us!
    KIS_SAFE_ASSERT_RECOVER_NOOP(cfg->hasProperty(KisImportExportFilter::ImageContainsTransparencyTag));
    KIS_SAFE_ASSERT_RECOVER_NOOP(cfg->hasProperty(KisImportExportFilter::ColorModelIDTag));
    KIS_SAFE_ASSERT_RECOVER_NOOP(cfg->hasProperty(KisImportExportFilter::sRGBTag));

    const bool isThereAlpha = cfg->getBool(KisImportExportFilter::ImageContainsTransparencyTag);

    alpha->setChecked(cfg->getBool("alpha", isThereAlpha));

    if (cfg->getString(KisImportExportFilter::ColorModelIDTag) == RGBAColorModelID.id()) {
        tryToSaveAsIndexed->setVisible(true);
        if (alpha->isChecked()) {
            tryToSaveAsIndexed->setChecked(false);
        }
        else {
            tryToSaveAsIndexed->setChecked(cfg->getBool("indexed", false));
        }
    }
    else {
        tryToSaveAsIndexed->setVisible(false);
    }
    interlacing->setChecked(cfg->getBool("interlaced", false));
    compressionLevel->setValue(cfg->getInt("compression", 3));
    compressionLevel->setRange(1, 9 , 0);

    alpha->setEnabled(isThereAlpha);
    tryToSaveAsIndexed->setVisible(!isThereAlpha);

    bnTransparencyFillColor->setEnabled(!alpha->isChecked());

    const bool sRGB = cfg->getBool(KisImportExportFilter::sRGBTag, false);

    chkSRGB->setEnabled(sRGB);
    chkSRGB->setChecked(cfg->getBool("saveSRGBProfile", true));

    chkForceSRGB->setEnabled(!sRGB);
    chkForceSRGB->setChecked(cfg->getBool("forceSRGB", false));

    QStringList rgb = cfg->getString("transparencyFillcolor", "0,0,0").split(',');
    KoColor c(KoColorSpaceRegistry::instance()->rgb8());
    c.fromQColor(Qt::white);
    bnTransparencyFillColor->setDefaultColor(c);
    c.fromQColor(QColor(rgb[0].toInt(), rgb[1].toInt(), rgb[2].toInt()));
    bnTransparencyFillColor->setColor(c);

}

KisPropertiesConfigurationSP KisWdgOptionsPNG::configuration() const
{

    KisPropertiesConfigurationSP cfg(new KisPropertiesConfiguration());

    bool alpha = this->alpha->isChecked();
    bool interlace = interlacing->isChecked();
    int compression = (int)compressionLevel->value();
    bool tryToSaveAsIndexed = this->tryToSaveAsIndexed->isChecked();
    QColor c = bnTransparencyFillColor->color().toQColor();
    bool saveSRGB = chkSRGB->isChecked();
    bool forceSRGB = chkForceSRGB->isChecked();

    cfg->setProperty("alpha", alpha);
    cfg->setProperty("indexed", tryToSaveAsIndexed);
    cfg->setProperty("compression", compression);
    cfg->setProperty("interlaced", interlace);
    cfg->setProperty("transparencyFillcolor", QString("%1,%2,%3").arg(c.red()).arg(c.green()).arg(c.blue()));
    cfg->setProperty("saveSRGBProfile", saveSRGB);
    cfg->setProperty("forceSRGB", forceSRGB);

    return cfg;
}

void KisWdgOptionsPNG::on_alpha_toggled(bool checked)
{
    bnTransparencyFillColor->setEnabled(!checked);
}

#include "kis_png_export.moc"

