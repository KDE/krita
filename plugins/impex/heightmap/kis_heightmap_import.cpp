/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2017 Victor Wåhlström <victor.wahlstrom@initiali.se>
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
#include <QToolTip>
#include <QShowEvent>

#include <kpluginfactory.h>
#include <QFileInfo>
#include <KoDialog.h>

#include <KisImportExportManager.h>
#include <KoColorSpaceRegistry.h>
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

#include "kis_wdg_options_heightmap.h"

K_PLUGIN_FACTORY_WITH_JSON(HeightMapImportFactory, "krita_heightmap_import.json", registerPlugin<KisHeightMapImport>();)

template<typename T>
void fillData(KisPaintDeviceSP pd, int w, int h, QDataStream &stream) {
    KIS_ASSERT_RECOVER_RETURN(pd);

    T pixel;

    for (int i = 0; i < h; ++i) {
        KisHLineIteratorSP it = pd->createHLineIteratorNG(0, i, w);
        do {
            stream >> pixel;
            KoGrayTraits<T>::setGray(it->rawData(), pixel);
            KoGrayTraits<T>::setOpacity(it->rawData(), OPACITY_OPAQUE_F, 1);
        } while(it->nextPixel());
    }
}

KisHeightMapImport::KisHeightMapImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisHeightMapImport::~KisHeightMapImport()
{
}

KisImportExportFilter::ConversionStatus KisHeightMapImport::convert(KisDocument *document, QIODevice *io, KisPropertiesConfigurationSP configuration)
{
    Q_UNUSED(configuration);
    KoID depthId;
    if (mimeType() == "image/x-r8") {
        depthId = Integer8BitsColorDepthID;
    }
    else if (mimeType() == "image/x-r16") {
        depthId = Integer16BitsColorDepthID;
    }
    else {
        document->setErrorMessage(i18n("The file is not 8 or 16 bits raw"));
        return KisImportExportFilter::WrongFormat;
    }

    QApplication::restoreOverrideCursor();

    KoDialog* kdb = new KoDialog(0);
    kdb->setWindowTitle(i18n("R16 HeightMap Import Options"));
    kdb->setButtons(KoDialog::Ok | KoDialog::Cancel);

    KisWdgOptionsHeightmap* wdg = new KisWdgOptionsHeightmap(kdb);

    kdb->setMainWidget(wdg);

    connect(wdg, SIGNAL(statusUpdated(bool)), kdb, SLOT(enableButtonOk(bool)));

    KisConfig config;

    QString filterConfig = config.importConfiguration(mimeType());
    KisPropertiesConfigurationSP cfg(new KisPropertiesConfiguration);
    cfg->fromXML(filterConfig);

    int w = 0;
    int h = 0;

    int endianness = cfg->getInt("endianness", 1);
    if (endianness == 0) {
        wdg->radioBig->setChecked(true);
    }
    else {
        wdg->radioLittle->setChecked(true);
    }

    KIS_ASSERT(io->isOpen());
    quint64 size = io->size();

    wdg->fileSizeLabel->setText(QString::number(size));

    int bpp = mimeType() == "image/x-r16" ? 16 : 8;

    wdg->bppLabel->setText(QString::number(bpp));

    if (!batchMode()) {
        if (kdb->exec() == QDialog::Rejected) {
            return KisImportExportFilter::UserCancelled;
        }
    }

    cfg->setProperty("endianness", wdg->radioBig->isChecked() ? 0 : 1);

    config.setImportConfiguration(mimeType(), cfg);

    w = wdg->widthInput->value();
    h = wdg->heightInput->value();

    QDataStream::ByteOrder bo = QDataStream::LittleEndian;
    cfg->setProperty("endianness", 1);
    if (wdg->radioBig->isChecked()) {
        bo = QDataStream::BigEndian;
        cfg->setProperty("endianness", 0);
    }
    KisConfig().setExportConfiguration(mimeType(), cfg);

    QDataStream s(io);
    s.setByteOrder(bo);

    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(), depthId.id(), 0);
    KisImageSP image = new KisImage(document->createUndoStore(), w, h, colorSpace, "imported heightmap");
    KisPaintLayerSP layer = new KisPaintLayer(image, image->nextLayerName(), 255);

    bool r16 = (depthId == Integer16BitsColorDepthID);
    if (r16) {
        fillData<quint16>(layer->paintDevice(), w, h, s);
    }
    else {
        fillData<quint8>(layer->paintDevice(), w, h, s);
    }

    image->addNode(layer.data(), image->rootLayer().data());
    document->setCurrentImage(image);
    return KisImportExportFilter::OK;
}

#include "kis_heightmap_import.moc"
