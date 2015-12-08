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

#include <kis_paint_device.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_config.h>
#include <kis_properties_configuration.h>
#include <kis_meta_data_store.h>
#include <kis_meta_data_filter_registry_model.h>
#include <kis_exif_info_visitor.h>
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

KisImportExportFilter::ConversionStatus KisPNGExport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile << "Png export! From:" << from << ", To:" << to << "";


    KisDocument *input = m_chain->inputDocument();
    QString filename = m_chain->outputFile();

    if (!input)
        return KisImportExportFilter::NoDocumentCreated;


    if (filename.isEmpty()) return KisImportExportFilter::FileNotFound;

    QUrl url = QUrl::fromLocalFile(filename);
    if (from != "application/x-krita")
        return KisImportExportFilter::NotImplemented;


    KoDialog* kdb = new KoDialog(0);
    kdb->setCaption(i18n("PNG Export Options"));
    kdb->setModal(false);
    kdb->setButtons(KoDialog::Ok | KoDialog::Cancel);

    KisImageWSP image = input->image();
    qApp->processEvents(); // For vector layers to be updated
    input->image()->waitForDone();

    image->refreshGraph();
    image->lock();
    KisPaintDeviceSP pd;
    pd = new KisPaintDevice(*image->projection());
    KisPaintLayerSP l = new KisPaintLayer(image, "projection", OPACITY_OPAQUE_U8, pd);
    image->unlock();


    if (!KisPNGConverter::isColorSpaceSupported(pd->colorSpace())) {
        if (!m_chain->manager()->getBatchMode()) {
            QMessageBox::critical(0, i18nc("@title:window", "Krita PNG Export"), i18n("You can only save grayscale and RGB images to PNG. Convert your image before exporting to PNG."));
        }
        return KisImportExportFilter::UsageError;
    }


    KisSequentialConstIterator it(l->paintDevice(), image->bounds());
    const KoColorSpace* cs = l->paintDevice()->colorSpace();

    KisPNGOptions options;
    bool isThereAlpha = false;
    do {
        if (cs->opacityU8(it.oldRawData()) != OPACITY_OPAQUE_U8) {
            isThereAlpha = true;
            break;
        }
    } while (it.nextPixel());

    if (qApp->applicationName() != "qttest") {

        bool sRGB = (cs->profile()->name().contains(QLatin1String("srgb"), Qt::CaseInsensitive)
                     && !cs->profile()->name().contains(QLatin1String("g10")));

        KisWdgOptionsPNG* wdg = new KisWdgOptionsPNG(kdb);

        QString filterConfig = KisConfig().exportConfiguration("PNG");
        KisPropertiesConfiguration cfg;
        cfg.fromXML(filterConfig);

        wdg->alpha->setChecked(cfg.getBool("alpha", isThereAlpha));

        if (cs->colorModelId() == RGBAColorModelID) {
            wdg->tryToSaveAsIndexed->setVisible(true);
            if (wdg->alpha->isChecked()) {
                wdg->tryToSaveAsIndexed->setChecked(false);
            }
            else {
                wdg->tryToSaveAsIndexed->setChecked(cfg.getBool("indexed", false));
            }
        }
        else {
            wdg->tryToSaveAsIndexed->setVisible(false);
        }
        wdg->interlacing->setChecked(cfg.getBool("interlaced", false));
        wdg->compressionLevel->setValue(cfg.getInt("compression", 9));
        wdg->compressionLevel->setRange(1, 9 , 0);

        wdg->alpha->setVisible(isThereAlpha);
        wdg->tryToSaveAsIndexed->setVisible(!isThereAlpha);

        wdg->bnTransparencyFillColor->setEnabled(!wdg->alpha->isChecked());

        wdg->chkSRGB->setVisible(sRGB);
        wdg->chkSRGB->setChecked(cfg.getBool("saveSRGBProfile", true));

        wdg->chkForceSRGB->setVisible(!sRGB);
        wdg->chkForceSRGB->setChecked(cfg.getBool("forceSRGB", false));

        QStringList rgb = cfg.getString("transparencyFillcolor", "0,0,0").split(',');
        wdg->bnTransparencyFillColor->setDefaultColor(Qt::white);
        wdg->bnTransparencyFillColor->setColor(QColor(rgb[0].toInt(), rgb[1].toInt(), rgb[2].toInt()));

        kdb->setMainWidget(wdg);
        QApplication::restoreOverrideCursor();
        if (hasVisibleWidgets()) {
            if (!m_chain->manager()->getBatchMode()) {
                if (kdb->exec() == QDialog::Rejected) {
                    return KisImportExportFilter::UserCancelled;
                }
            }
        }

        bool alpha = wdg->alpha->isChecked();
        bool interlace = wdg->interlacing->isChecked();
        int compression = (int)wdg->compressionLevel->value();
        bool tryToSaveAsIndexed = wdg->tryToSaveAsIndexed->isChecked();
        QColor c = wdg->bnTransparencyFillColor->color();
        bool saveSRGB = wdg->chkSRGB->isChecked();
        bool forceSRGB = wdg->chkForceSRGB->isChecked();

        cfg.setProperty("alpha", alpha);
        cfg.setProperty("indexed", tryToSaveAsIndexed);
        cfg.setProperty("compression", compression);
        cfg.setProperty("interlaced", interlace);
        cfg.setProperty("transparencyFillcolor", QString("%1,%2,%3").arg(c.red()).arg(c.green()).arg(c.blue()));
        cfg.setProperty("saveSRGBProfile", saveSRGB);
        cfg.setProperty("forceSRGB", forceSRGB);
        KisConfig().setExportConfiguration("PNG", cfg);

        options.alpha = alpha;
        options.interlace = interlace;
        options.compression = compression;
        options.tryToSaveAsIndexed = tryToSaveAsIndexed;
        options.transparencyFillColor = c;
        options.saveSRGBProfile = saveSRGB;
        options.forceSRGB = forceSRGB;

    }
    else {
        options.alpha = isThereAlpha;
        options.interlace = false;
        options.compression = 9;
        options.tryToSaveAsIndexed = false;
        options.transparencyFillColor = QColor(0,0,0);
        options.saveSRGBProfile = false;
        options.forceSRGB = false;

    }

    delete kdb;

    KisPNGConverter kpc(input);

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
    if ((res = kpc.buildFile(url, image->bounds(), image->xRes(), image->yRes(), l->paintDevice(), beginIt, endIt, options, eI)) == KisImageBuilder_RESULT_OK) {
        dbgFile << "success !";
        delete eI;
        return KisImportExportFilter::OK;
    }
    delete eI;
    dbgFile << " Result =" << res;
    return KisImportExportFilter::InternalError;
}

#include "kis_png_export.moc"


void KisWdgOptionsPNG::on_alpha_toggled(bool checked)
{
    bnTransparencyFillColor->setEnabled(!checked);
}
