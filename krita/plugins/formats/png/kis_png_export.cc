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

#include <kapplication.h>
#include <kdialog.h>
#include <kgenericfactory.h>

#include <KoColorSpace.h>
#include <KoFilterChain.h>

#include <kis_paint_device.h>
#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>

#include <kis_iterators_pixel.h>

#include <kis_meta_data_store.h>
#include <kis_meta_data_filter_registry_model.h>
#include <kis_exif_info_visitor.h>
#include "kis_png_converter.h"

typedef KGenericFactory<KisPNGExport> KisPNGExportFactory;
K_EXPORT_COMPONENT_FACTORY(libkritapngexport, KisPNGExportFactory("kofficefilters"))

KisPNGExport::KisPNGExport(QObject *parent, const QStringList&) : KoFilter(parent)
{
}

KisPNGExport::~KisPNGExport()
{
}

bool hasVisibleWidgets()
{
    QWidgetList wl = QApplication::allWidgets();
    foreach(QWidget* w, wl) {
        if (w->isVisible() && strcmp(w->metaObject()->className(), "QDesktopWidget")) {
            dbgFile << "Widget " << w << " " << w->objectName() << " " << w->metaObject()->className() << " is visible";
            return true;
        }
    }
    return false;
}

KoFilter::ConversionStatus KisPNGExport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile << "Png export! From:" << from << ", To:" << to << "";

    KisDoc2 *output = dynamic_cast<KisDoc2*>(m_chain->inputDocument());
    QString filename = m_chain->outputFile();

    if (!output)
        return KoFilter::CreationError;


    if (filename.isEmpty()) return KoFilter::FileNotFound;

    if (from != "application/x-krita")
        return KoFilter::NotImplemented;

    KDialog* kdb = new KDialog(0);
    kdb->setCaption(i18n("PNG Export Options"));
    kdb->setModal(false);

    KisImageWSP image = output->image();
    image->lock();
    image->refreshGraph();
    KisPaintDeviceSP pd;
    pd = new KisPaintDevice(*image->projection());
    KisPaintLayerSP l = new KisPaintLayer(image, "projection", OPACITY_OPAQUE, pd);
    image->unlock();

    KisRectConstIteratorPixel it = l->paintDevice()->createRectConstIterator(0, 0, image->width(), image->height());
    const KoColorSpace* cs = l->paintDevice()->colorSpace();

    bool isThereAlpha = false;
    while (!it.isDone()) {
        if (cs->alpha(it.rawData()) != 255) {
            isThereAlpha = true;
            break;
        }
        ++it;
    }

    KisWdgOptionsPNG* wdg = new KisWdgOptionsPNG(kdb);
    wdg->alpha->setChecked(isThereAlpha);
    wdg->alpha->setVisible(isThereAlpha);
    wdg->tryToSaveAsIndexed->setVisible(!isThereAlpha);
    if (isThereAlpha) {
        wdg->tryToSaveAsIndexed->setChecked(false);
    }
    kdb->setMainWidget(wdg);
    kapp->restoreOverrideCursor();
    if (hasVisibleWidgets()) {
        if (kdb->exec() == QDialog::Rejected) {
            return KoFilter::OK; // FIXME Cancel doesn't exist :(
        }
    }

    bool alpha = wdg->alpha->isChecked();
    bool interlace = wdg->interlacing->isChecked();
    int compression = wdg->compressionLevel->value();
    bool tryToSaveAsIndexed = wdg->tryToSaveAsIndexed->isChecked();

    delete kdb;

    KUrl url;
    url.setPath(filename);

    KisPNGConverter kpc(output, output->undoAdapter());

    vKisAnnotationSP_it beginIt = image->beginAnnotations();
    vKisAnnotationSP_it endIt = image->endAnnotations();
    KisImageBuilder_Result res;
    KisPNGOptions options;
    options.alpha = alpha;
    options.interlace = interlace;
    options.compression = compression;
    options.tryToSaveAsIndexed = tryToSaveAsIndexed;
    KisExifInfoVisitor eIV;
    eIV.visit(image->rootLayer().data());
    KisMetaData::Store* eI = 0;
    if (eIV.countPaintLayer() == 1)
        eI = eIV.exifInfo();
    if (eI) {
        KisMetaData::Store* copy = new KisMetaData::Store(*eI);
        eI = copy;
    }
    if ((res = kpc.buildFile(url, image, l->paintDevice(), beginIt, endIt, options, eI)) == KisImageBuilder_RESULT_OK) {
        dbgFile << "success !";
        delete eI;
        return KoFilter::OK;
    }
    delete eI;
    dbgFile << " Result =" << res;
    return KoFilter::InternalError;
}

#include "kis_png_export.moc"

