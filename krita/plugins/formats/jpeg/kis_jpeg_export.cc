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

#include <kapplication.h>
#include <kdialog.h>
#include <kpluginfactory.h>
#include <kcolorbutton.h>

#include <KoFilterManager.h>
#include <KoFilterChain.h>
#include <KoColorSpaceConstants.h>

#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>
#include <kis_properties_configuration.h>
#include <kis_config.h>
#include <kis_meta_data_store.h>
#include <kis_meta_data_filter_registry_model.h>
#include <kis_exif_info_visitor.h>
#include <generator/kis_generator_layer.h>
#include "kis_jpeg_converter.h"


#include "ui_kis_wdg_options_jpeg.h"

class KisExternalLayer;

K_PLUGIN_FACTORY(KisJPEGExportFactory, registerPlugin<KisJPEGExport>();)
K_EXPORT_PLUGIN(KisJPEGExportFactory("calligrafilters"))

KisJPEGExport::KisJPEGExport(QObject *parent, const QVariantList &) : KoFilter(parent)
{
}

KisJPEGExport::~KisJPEGExport()
{
}

KoFilter::ConversionStatus KisJPEGExport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile << "JPEG export! From:" << from << ", To:" << to << "";

    if (from != "application/x-krita")
        return KoFilter::NotImplemented;


    KDialog* kdb = new KDialog(0);
    kdb->setWindowTitle(i18n("JPEG Export Options"));
    kdb->setButtons(KDialog::Ok | KDialog::Cancel);

    Ui::WdgOptionsJPEG wdgUi;
    QWidget* wdg = new QWidget(kdb);
    wdgUi.setupUi(wdg);
    KisMetaData::FilterRegistryModel frm;
    wdgUi.metaDataFilters->setModel(&frm);

    // until we've got an actual quality preview here!
    wdgUi.chkPreview->setVisible(false);

    QString filterConfig = KisConfig().exportConfiguration("JPEG");
    KisPropertiesConfiguration cfg;
    cfg.fromXML(filterConfig);

    wdgUi.progressive->setChecked(cfg.getBool("progressive", false));
    wdgUi.qualityLevel->setValue(cfg.getInt("quality", 80));
    wdgUi.optimize->setChecked(cfg.getBool("optimize", true));
    wdgUi.smoothLevel->setValue(cfg.getInt("smoothing", 0));
    wdgUi.baseLineJPEG->setChecked(cfg.getBool("baseline", true));
    wdgUi.subsampling->setCurrentIndex(cfg.getInt("subsampling", 0));
    wdgUi.exif->setChecked(cfg.getBool("exif", true));
    wdgUi.iptc->setChecked(cfg.getBool("iptc", true));
    wdgUi.xmp->setChecked(cfg.getBool("xmp", true));

    QStringList rgb = cfg.getString("transparencyFillcolor", "255,255,255").split(",");
    wdgUi.bnTransparencyFillColor->setDefaultColor(Qt::white);
    wdgUi.bnTransparencyFillColor->setColor(QColor(rgb[0].toInt(), rgb[1].toInt(), rgb[2].toInt()));

    frm.setEnabledFilters(cfg.getString("filters").split(","));

    kdb->setMainWidget(wdg);
    kapp->restoreOverrideCursor();

    if (!m_chain->manager()->getBatchMode()) {
        if (kdb->exec() == QDialog::Rejected) {
            return KoFilter::OK; // FIXME Cancel doesn't exist :(
        }
    }
    
    KisJPEGOptions options;
    options.progressive = wdgUi.progressive->isChecked();
    cfg.setProperty("progressive", options.progressive);

    options.quality = wdgUi.qualityLevel->value();
    cfg.setProperty("quality", options.quality);

    // Advanced
    options.optimize = wdgUi.optimize->isChecked();
    cfg.setProperty("optimize", options.optimize);

    options.smooth = wdgUi.smoothLevel->value();
    cfg.setProperty("smoothing", options.smooth);

    options.baseLineJPEG = wdgUi.baseLineJPEG->isChecked();
    cfg.setProperty("baseline", options.baseLineJPEG);

    options.subsampling = wdgUi.subsampling->currentIndex();
    cfg.setProperty("subsampling", options.subsampling);
    // Jpeg
    options.exif = wdgUi.exif->isChecked();
    cfg.setProperty("exif", options.exif);

    options.iptc = wdgUi.iptc->isChecked();
    cfg.setProperty("iptc", options.iptc);

    options.xmp = wdgUi.xmp->isChecked();
    cfg.setProperty("xmp", options.xmp);

    QColor c = wdgUi.bnTransparencyFillColor->color();
    options.transparencyFillColor = c;
    cfg.setProperty("transparencyFillcolor", QString("%1,%2,%3").arg(c.red()).arg(c.green()).arg(c.blue()));

    options.filters = frm.enabledFilters();
    QString enabledFilters;
    foreach(const KisMetaData::Filter* filter, options.filters) {
        enabledFilters = enabledFilters + filter->id() + ",";
    }

    cfg.setProperty("filters", enabledFilters);


    KisConfig().setExportConfiguration("JPEG", cfg);

    delete kdb;
    // XXX: Add dialog about flattening layers here

    KisDoc2 *output = dynamic_cast<KisDoc2*>(m_chain->inputDocument());
    QString filename = m_chain->outputFile();

    if (!output)
        return KoFilter::CreationError;


    if (filename.isEmpty()) return KoFilter::FileNotFound;

    KUrl url;
    url.setPath(filename);

    KisImageWSP image = output->image();
    Q_CHECK_PTR(image);
    image->refreshGraph();
    image->lock();

    KisJPEGConverter kpc(output);

    KisPaintDeviceSP pd = new KisPaintDevice(*image->projection());
    image->unlock();
    KisPaintLayerSP l = new KisPaintLayer(image, "projection", OPACITY_OPAQUE_U8, pd);

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
    if ((res = kpc.buildFile(url, l, beginIt, endIt, options, eI)) == KisImageBuilder_RESULT_OK) {
        dbgFile << "success !";
        delete eI;
        return KoFilter::OK;
    }
    delete eI;
    dbgFile << " Result =" << res;
    return KoFilter::InternalError;
}

#include <kis_jpeg_export.moc>

