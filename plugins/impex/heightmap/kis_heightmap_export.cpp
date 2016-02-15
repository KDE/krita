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
#include <KisFilterChain.h>
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

#include "ui_kis_wdg_options_heightmap.h"

K_PLUGIN_FACTORY_WITH_JSON(KisHeightMapExportFactory, "krita_heightmap_export.json", registerPlugin<KisHeightMapExport>();)

KisHeightMapExport::KisHeightMapExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisHeightMapExport::~KisHeightMapExport()
{
}

KisImportExportFilter::ConversionStatus KisHeightMapExport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile << "HeightMap export! From:" << from << ", To:" << to;

    if (from != "application/x-krita")
        return KisImportExportFilter::NotImplemented;

    KisDocument *inputDoc = m_chain->inputDocument();
    QString filename = m_chain->outputFile();

    if (!inputDoc)
        return KisImportExportFilter::NoDocumentCreated;

    if (filename.isEmpty()) return KisImportExportFilter::FileNotFound;

    KisImageWSP image = inputDoc->image();
    Q_CHECK_PTR(image);

    if (inputDoc->image()->width() != inputDoc->image()->height()) {
        inputDoc->setErrorMessage(i18n("Cannot export this image to a heightmap: it is not square"));
        return KisImportExportFilter::WrongFormat;
    }

    if (inputDoc->image()->colorSpace()->colorModelId() != GrayAColorModelID) {
        inputDoc->setErrorMessage(i18n("Cannot export this image to a heightmap: it is not grayscale"));
        return KisImportExportFilter::WrongFormat;
    }

    KoDialog* kdb = new KoDialog(0);
    kdb->setWindowTitle(i18n("HeightMap Export Options"));
    kdb->setButtons(KoDialog::Ok | KoDialog::Cancel);

    Ui::WdgOptionsHeightMap optionsHeightMap;

    QWidget* wdg = new QWidget(kdb);
    optionsHeightMap.setupUi(wdg);

    kdb->setMainWidget(wdg);
    QApplication::restoreOverrideCursor();

    QString filterConfig = KisConfig().exportConfiguration("HeightMap");
    KisPropertiesConfiguration cfg;
    cfg.fromXML(filterConfig);

    optionsHeightMap.intSize->setValue(image->width());

    int endianness = cfg.getInt("endianness", 0);
    QDataStream::ByteOrder bo = QDataStream::LittleEndian;
    optionsHeightMap.radioPC->setChecked(true);

    if (endianness == 0) {
        bo = QDataStream::BigEndian;
        optionsHeightMap.radioMac->setChecked(true);
    }

    if (!m_chain->manager()->getBatchMode()) {
        if (kdb->exec() == QDialog::Rejected) {
            return KisImportExportFilter::UserCancelled;
        }
    }
    else {
        qApp->processEvents(); // For vector layers to be updated
    }
    inputDoc->image()->waitForDone();

    if (optionsHeightMap.radioMac->isChecked()) {
        cfg.setProperty("endianness", 0);
        bo = QDataStream::BigEndian;
    }
    else {
        cfg.setProperty("endianness", 1);
        bo = QDataStream::LittleEndian;
    }
    KisConfig().setExportConfiguration("HeightMap", cfg);

    bool downscale = false;
    if (to == "image/x-r8" && image->colorSpace()->colorDepthId() == Integer16BitsColorDepthID) {

        downscale = (QMessageBox::question(0,
                                           i18nc("@title:window", "Downscale Image"),
                                           i18n("You specified the .r8 extension for a 16 bit/channel image. Do you want to save as 8 bit? Your image data will not be changed."),
                                           QMessageBox::Yes | QMessageBox::No)
                          == QMessageBox::Yes);
    }

    image->refreshGraph();
    image->lock();
    KisPaintDeviceSP pd = new KisPaintDevice(*image->projection());
    image->unlock();

    QFile f(filename);
    f.open(QIODevice::WriteOnly);
    QDataStream s(&f);
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

    f.close();
    return KisImportExportFilter::OK;
}

#include "kis_heightmap_export.moc"