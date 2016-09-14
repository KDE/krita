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

#include <KoDialog.h>
#include <kpluginfactory.h>
#include <QMessageBox>

#include <KoColorSpace.h>
#include <KisFilterChain.h>
#include <KisImportExportManager.h>
#include <KoColorProfile.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpaceRegistry.h>

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
    KisImageWSP image = document->image();

    // the image must be locked at the higher levels
    KIS_SAFE_ASSERT_RECOVER_NOOP(image->locked());
    KisPaintDeviceSP pd;
    pd = new KisPaintDevice(*image->projection());
    KisPaintLayerSP l = new KisPaintLayer(image, "projection", OPACITY_OPAQUE_U8, pd);


    if (!KisPNGConverter::isColorSpaceSupported(pd->colorSpace())) {
        if (!getBatchMode()) {
            QMessageBox::critical(0, i18nc("@title:window", "Krita PNG Export"), i18n("You can only save grayscale and RGB images to PNG. Convert your image before exporting to PNG."));
        }
        return KisImportExportFilter::UsageError;
    }


    KisSequentialConstIterator it(l->paintDevice(), image->bounds());
    const KoColorSpace* cs = l->paintDevice()->colorSpace();

    KisPNGOptions options;
    bool isThereAlpha = false;
    KisPropertiesConfigurationSP cfg = defaultConfiguration();
    do {
        if (cs->opacityU8(it.oldRawData()) != OPACITY_OPAQUE_U8) {
            isThereAlpha = true;
            break;
        }
    } while (it.nextPixel());

    if (!qApp->applicationName().toLower().contains("test")) {

        KoDialog kdb;
        kdb.setCaption(i18n("PNG Export Options"));
        kdb.setButtons(KoDialog::Ok | KoDialog::Cancel);
        KisConfigWidget *wdg = createConfigurationWidget(&kdb, KisDocument::nativeFormatMimeType(), "image/png");
        kdb.setMainWidget(wdg);

        // If a configuration object was passed to the convert method, we use that, otherwise we load from the settings
        if (configuration) {
            cfg->fromXML(configuration->toXML());
        }
        else {
            cfg = lastSavedConfiguration(KisDocument::nativeFormatMimeType(), "image/png");
        }

        cfg->setProperty("ColorModelID", cs->colorModelId().id());

        bool sRGB = (cs->profile()->name().contains(QLatin1String("srgb"), Qt::CaseInsensitive)
                     && !cs->profile()->name().contains(QLatin1String("g10")));
        cfg->setProperty("sRGB", sRGB);
        cfg->setProperty("isThereAlpha", isThereAlpha);
        wdg->setConfiguration(cfg);

        QApplication::restoreOverrideCursor();
        if (hasVisibleWidgets()) {
            if (!getBatchMode()) {
                if (kdb.exec() == QDialog::Rejected) {
                    return KisImportExportFilter::UserCancelled;
                }
                cfg = wdg->configuration();
                KisConfig().setExportConfiguration("PNG", *cfg.data());

            }
        }
    }

    options.alpha = cfg->getBool("alpha", true);
    options.interlace = cfg->getBool("interlaced", false);
    options.compression = cfg->getInt("compression", 0);
    options.tryToSaveAsIndexed = cfg->getBool("indexed", false);
    options.transparencyFillColor = cfg->getColor("transparencyFillColor").toQColor();
    options.saveSRGBProfile = cfg->getBool("saveSRGBProfile", false);
    options.forceSRGB = cfg->getBool("forceSRGB", true);

    vKisAnnotationSP_it beginIt = image->beginAnnotations();
    vKisAnnotationSP_it endIt = image->endAnnotations();
    KisImageBuilder_Result res;


    KisExifInfoVisitor eIV;
    eIV.visit(image->rootLayer().data());
    KisMetaData::Store* eI = 0;
    if (eIV.countPaintLayer() == 1)
        eI = eIV.exifInfo();
    if (eI) {
        KisMetaData::Store* copy = new KisMetaData::Store(*eI);
        eI = copy;
    }
    KisPNGConverter pngConverter(document);
    if ((res = pngConverter.buildFile(io, image->bounds(), image->xRes(), image->yRes(), l->paintDevice(), beginIt, endIt, options, eI)) == KisImageBuilder_RESULT_OK) {
        dbgFile << "success !";
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
    cfg->setProperty("compression", 9);
    cfg->setProperty("interlaced", false);
    cfg->setProperty("transparencyFillcolor", QString("255,255,255"));
    cfg->setProperty("saveSRGBProfile", false);
    cfg->setProperty("forceSRGB", true);

    return cfg;
}

KisPropertiesConfigurationSP KisPNGExport::lastSavedConfiguration(const QByteArray &from, const QByteArray &to) const
{
    QString filterConfig = KisConfig().exportConfiguration("PNG");
    KisPropertiesConfigurationSP cfg = defaultConfiguration(from, to);
    cfg->fromXML(filterConfig, false);
    return cfg;
}

KisConfigWidget *KisPNGExport::createConfigurationWidget(QWidget *parent, const QByteArray &, const QByteArray &) const
{
    return new KisWdgOptionsPNG(parent);
}

void KisWdgOptionsPNG::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    bool isThereAlpha = cfg->getBool("isThereAlpha");

    alpha->setChecked(cfg->getBool("alpha", isThereAlpha));

    if (cfg->getString("ColorModelID") == RGBAColorModelID.id()) {
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
    compressionLevel->setValue(cfg->getInt("compression", 9));
    compressionLevel->setRange(1, 9 , 0);

    alpha->setEnabled(isThereAlpha);
    tryToSaveAsIndexed->setVisible(!isThereAlpha);

    bnTransparencyFillColor->setEnabled(!alpha->isChecked());

    bool sRGB = cfg->getBool("sRGB", false);

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

