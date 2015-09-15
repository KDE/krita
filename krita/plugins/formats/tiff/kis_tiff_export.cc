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
#include <QUrl>

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

KisImportExportFilter::ConversionStatus KisTIFFExport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile << "Tiff export! From:" << from << ", To:" << to << "";

    if (from != "application/x-krita")
        return KisImportExportFilter::NotImplemented;


    KisDlgOptionsTIFF* kdb = new KisDlgOptionsTIFF(0);

    KisDocument *input = m_chain->inputDocument();
    if (!input)
        return KisImportExportFilter::NoDocumentCreated;

    const KoColorSpace* cs = input->image()->colorSpace();
    KoChannelInfo::enumChannelValueType type = cs->channels()[0]->channelValueType();
    if (type == KoChannelInfo::FLOAT16 || type == KoChannelInfo::FLOAT32) {
        kdb->optionswdg->kComboBoxPredictor->removeItem(1);
    } else {
        kdb->optionswdg->kComboBoxPredictor->removeItem(2);
    }

    if (cs->colorModelId() == CMYKAColorModelID) {
        kdb->optionswdg->alpha->setChecked(false);
        kdb->optionswdg->alpha->setEnabled(false);
    }
    if (!m_chain->manager()->getBatchMode()) {
        if (kdb->exec() == QDialog::Rejected) {
            return KisImportExportFilter::UserCancelled;
        }
    }
    else {
        qApp->processEvents(); // For vector layers to be updated

    }
    input->image()->waitForDone();

    KisTIFFOptions options = kdb->options();

    if ((type == KoChannelInfo::FLOAT16 || type == KoChannelInfo::FLOAT32) && options.predictor == 2) { // FIXME THIS IS AN HACK FIX THAT IN 2.0 !!
        options.predictor = 3;
    }
    delete kdb;

    QString filename = m_chain->outputFile();

    if (filename.isEmpty()) return KisImportExportFilter::FileNotFound;

    QUrl url = QUrl::fromLocalFile(filename);

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

    image->refreshGraph();
    image->lock();

    KisTIFFConverter ktc(input);
    /*    vKisAnnotationSP_it beginIt = image->beginAnnotations();
        vKisAnnotationSP_it endIt = image->endAnnotations();*/
    KisImageBuilder_Result res;
    if ((res = ktc.buildFile(url, image, options)) == KisImageBuilder_RESULT_OK) {
        dbgFile << "success !";
        image->unlock();
        return KisImportExportFilter::OK;
    }
    image->unlock();
    dbgFile << " Result =" << res;
    return KisImportExportFilter::InternalError;
}

#include <kis_tiff_export.moc>

