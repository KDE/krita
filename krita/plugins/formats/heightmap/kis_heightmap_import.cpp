/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
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

#include "kis_heightmap_import.h"

#include <ctype.h>

#include <QApplication>
#include <QFile>
#include <qendian.h>
#include <QCursor>

#include <kpluginfactory.h>
#include <QUrl>
#include <KoDialog.h>

#include <KisImportExportManager.h>
#include <KoColorSpaceRegistry.h>
#include <KisFilterChain.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>
#include <KoColorSpaceTraits.h>

#include <kis_debug.h>
#include <KisDocument.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>
#include <kis_transaction.h>
#include <kis_iterator_ng.h>
#include <kis_random_accessor_ng.h>
#include <kis_config.h>

#include "ui_kis_wdg_options_heightmap.h"

K_PLUGIN_FACTORY_WITH_JSON(HeightMapImportFactory, "krita_heightmap_import.json", registerPlugin<KisHeightMapImport>();)

KisHeightMapImport::KisHeightMapImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisHeightMapImport::~KisHeightMapImport()
{
}

KisImportExportFilter::ConversionStatus KisHeightMapImport::convert(const QByteArray& from, const QByteArray& to)
{

    KisDocument * doc = m_chain->outputDocument();

    if (!doc) {
        return KisImportExportFilter::NoDocumentCreated;
    }

    KoID depthId;
    if (from == "image/x-r8") {
        depthId = Integer8BitsColorDepthID;
    }
    else if (from == "image/x-r16") {
        depthId = Integer16BitsColorDepthID;
    }
    else {
        doc->setErrorMessage(i18n("The file is not 8 or 16 bits raw"));
        return KisImportExportFilter::WrongFormat;
    }
    dbgFile << "Importing using HeightMapImport!";

    if (to != "application/x-krita") {
        return KisImportExportFilter::BadMimeType;
    }

    QString filename = m_chain->inputFile();

    if (filename.isEmpty()) {
        return KisImportExportFilter::FileNotFound;
    }

    QUrl url = QUrl::fromLocalFile(filename);


    dbgFile << "Import: " << url;
    if (url.isEmpty())
        return KisImportExportFilter::FileNotFound;

    if (!url.isLocalFile()) {
        return KisImportExportFilter::FileNotFound;
    }

    QApplication::restoreOverrideCursor();

    KoDialog* kdb = new KoDialog(0);
    kdb->setWindowTitle(i18n("R16 HeightMap Import Options"));
    kdb->setButtons(KoDialog::Ok | KoDialog::Cancel);

    Ui::WdgOptionsHeightMap optionsHeightMap;

    QWidget* wdg = new QWidget(kdb);
    optionsHeightMap.setupUi(wdg);

    kdb->setMainWidget(wdg);

    QString filterConfig = KisConfig().exportConfiguration("HeightMap");
    KisPropertiesConfiguration cfg;
    cfg.fromXML(filterConfig);

    QFile f(url.toLocalFile());

    int w = 0;
    int h = 0;

    if (!f.exists()) {
        doc->setErrorMessage(i18n("File does not exist."));
        return KisImportExportFilter::CreationError;
    }

    if (!f.isOpen()) {
        f.open(QFile::ReadOnly);
    }

    optionsHeightMap.intSize->setValue(0);
    int endianness = cfg.getInt("endianness", 0);
    if (endianness == 0) {
        optionsHeightMap.radioMac->setChecked(true);
    }
    else {
        optionsHeightMap.radioPC->setChecked(true);
    }

    if (!m_chain->manager()->getBatchMode()) {
        if (kdb->exec() == QDialog::Rejected) {
            return KisImportExportFilter::UserCancelled;
        }
    }

    w = h = optionsHeightMap.intSize->value();

    if ((w * h * (from == "image/x-r16" ? 2 : 1)) != f.size()) {
        doc->setErrorMessage(i18n("Source file is not the right size for the specified width and height."));
        return KisImportExportFilter::WrongFormat;
    }


    QDataStream::ByteOrder bo = QDataStream::LittleEndian;
    cfg.setProperty("endianness", 1);
    if (optionsHeightMap.radioMac->isChecked()) {
        bo = QDataStream::BigEndian;
        cfg.setProperty("endianness", 0);
    }
    KisConfig().setExportConfiguration("HeightMap", cfg);

    doc->prepareForImport();

    QDataStream s(&f);
    s.setByteOrder(bo);

    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(), depthId.id(), 0);
    KisImageSP image = new KisImage(doc->createUndoStore(), w, h, colorSpace, "imported heightmap");
    KisPaintLayerSP layer = new KisPaintLayer(image, image->nextLayerName(), 255);

    KisRandomAccessorSP it = layer->paintDevice()->createRandomAccessorNG(0, 0);
    bool r16 = (depthId == Integer16BitsColorDepthID);
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            it->moveTo(i, j);

            if (r16) {
                quint16 pixel;
                s >> pixel;
                KoGrayU16Traits::setGray(it->rawData(), pixel);
                KoGrayU16Traits::setOpacity(it->rawData(), OPACITY_OPAQUE_F, 1);
            }
            else {
                quint8 pixel;
                s >> pixel;
                KoGrayU8Traits::setGray(it->rawData(), pixel);
                KoGrayU8Traits::setOpacity(it->rawData(), OPACITY_OPAQUE_F, 1);
            }
        }
    }

    image->addNode(layer.data(), image->rootLayer().data());
    doc->setCurrentImage(image);
    return KisImportExportFilter::OK;
}

#include "kis_heightmap_import.moc"
