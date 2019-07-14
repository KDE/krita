/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_brush_export.h"

#include <QCheckBox>
#include <QSlider>
#include <QBuffer>

#include <KoProperties.h>
#include <KoDialog.h>
#include <kpluginfactory.h>
#include <QFileInfo>

#include <KisExportCheckRegistry.h>
#include <kis_paint_device.h>
#include <KisViewManager.h>
#include <kis_image.h>
#include <KoProperties.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_spacing_selection_widget.h>
#include <kis_gbr_brush.h>
#include <kis_imagepipe_brush.h>
#include <kis_pipebrush_parasite.h>
#include <KisAnimatedBrushAnnotation.h>
#include <KisWdgOptionsBrush.h>
#include <KisImportExportManager.h>
#include <kis_config.h>

struct KisBrushExportOptions {
    qreal spacing;
    bool mask;
    int brushStyle;
    int dimensions;
    qint32 ranks[KisPipeBrushParasite::MaxDim];
    qint32 selectionModes[KisPipeBrushParasite::MaxDim];
    QString name;
};


K_PLUGIN_FACTORY_WITH_JSON(KisBrushExportFactory, "krita_brush_export.json", registerPlugin<KisBrushExport>();)

KisBrushExport::KisBrushExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisBrushExport::~KisBrushExport()
{
}

KisImportExportErrorCode KisBrushExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration)
{

// XXX: Loading the parasite itself was commented out -- needs investigation
//    KisAnnotationSP annotation = document->savingImage()->annotation("ImagePipe Parasite");
//    KisPipeBrushParasite parasite;
//    if (annotation) {
//        QBuffer buf(const_cast<QByteArray*>(&annotation->annotation()));
//        buf.open(QBuffer::ReadOnly);
//        parasite.loadFromDevice(&buf);
//        buf.close();
//    }

    KisBrushExportOptions exportOptions;

    if (document->savingImage()->dynamicPropertyNames().contains("brushspacing")) {
        exportOptions.spacing = document->savingImage()->property("brushspacing").toFloat();
    }
    else {
        exportOptions.spacing = configuration->getInt("spacing");
    }
    if (!configuration->getString("name").isEmpty()) {
        exportOptions.name = configuration->getString("name");
    }
    else {
        exportOptions.name = document->savingImage()->objectName();
    }

    exportOptions.mask = configuration->getBool("mask");
    exportOptions.brushStyle = configuration->getInt("brushStyle");
    exportOptions.dimensions = configuration->getInt("dimensions");

    for (int i = 0; i < KisPipeBrushParasite::MaxDim; ++i) {
        exportOptions.selectionModes[i] = configuration->getInt("selectionMode" + QString::number(i));
        exportOptions.ranks[i] = configuration->getInt("rank" + QString::number(i));
    }

    KisGbrBrush *brush = 0;
    if (mimeType() == "image/x-gimp-brush") {
        brush = new KisGbrBrush(filename());
    }
    else if (mimeType() == "image/x-gimp-brush-animated") {
        brush = new KisImagePipeBrush(filename());
    }
    else {
        return ImportExportCodes::FileFormatIncorrect;
    }

    qApp->processEvents(); // For vector layers to be updated

    QRect rc = document->savingImage()->bounds();

    brush->setSpacing(exportOptions.spacing);

    KisImagePipeBrush *pipeBrush = dynamic_cast<KisImagePipeBrush*>(brush);
    if (pipeBrush) {
        // Create parasite. XXX: share with KisCustomBrushWidget
        QVector< QVector<KisPaintDevice*> > devices;
        devices.push_back(QVector<KisPaintDevice*>());

        KoProperties properties;
        properties.setProperty("visible", true);
        QList<KisNodeSP> layers = document->savingImage()->root()->childNodes(QStringList("KisLayer"), properties);

        Q_FOREACH (KisNodeSP node, layers) {
            // push_front to behave exactly as gimp for gih creation
            devices[0].push_front(node->projection().data());
        }

        QVector<KisParasite::SelectionMode > modes;

        for (int i = 0; i < KisPipeBrushParasite::MaxDim; ++i) {
            switch (exportOptions.selectionModes[i]) {
            case 0: modes.push_back(KisParasite::Constant); break;
            case 1: modes.push_back(KisParasite::Random); break;
            case 2: modes.push_back(KisParasite::Incremental); break;
            case 3: modes.push_back(KisParasite::Pressure); break;
            case 4: modes.push_back(KisParasite::Angular); break;
            case 5: modes.push_back(KisParasite::Velocity); break;
            default: modes.push_back(KisParasite::Incremental);
            }
        }

        KisPipeBrushParasite parasite;

        parasite.dim = exportOptions.dimensions;
        parasite.ncells = devices.at(0).count();

        int maxRanks = 0;
        for (int i = 0; i < KisPipeBrushParasite::MaxDim; ++i) {
            // ### This can mask some bugs, be careful here in the future
            parasite.rank[i] = exportOptions.ranks[i];
            parasite.selection[i] = modes.at(i);
            maxRanks += exportOptions.ranks[i];
        }

        if (maxRanks > layers.count()) {
            return ImportExportCodes::FileFormatIncorrect;
        }
        // XXX needs movement!
        parasite.setBrushesCount();
        pipeBrush->setParasite(parasite);
        pipeBrush->setDevices(devices, rc.width(), rc.height());

        if (exportOptions.mask) {
            QVector<KisGbrBrushSP> brushes = pipeBrush->brushes();
            Q_FOREACH(KisGbrBrushSP brush, brushes) {
                brush->setHasColor(false);
            }
        }
    }
    else {
        if (exportOptions.mask) {
            QImage image = document->savingImage()->projection()->convertToQImage(0, 0, 0, rc.width(), rc.height(), KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
            brush->setImage(image);
            brush->setBrushTipImage(image);
        } else {
            brush->initFromPaintDev(document->savingImage()->projection(),0,0,rc.width(), rc.height());
        }
    }

    brush->setName(exportOptions.name);
    // brushes are created after devices are loaded, call mask mode after that
    brush->setUseColorAsMask(exportOptions.mask);
    brush->setWidth(rc.width());
    brush->setHeight(rc.height());

    if (brush->saveToDevice(io)) {
        return ImportExportCodes::OK;
    }
    else {
        return ImportExportCodes::Failure;
    }
}

KisPropertiesConfigurationSP KisBrushExport::defaultConfiguration(const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("spacing", 1.0);
    cfg->setProperty("name", "");
    cfg->setProperty("mask", true);
    cfg->setProperty("brushStyle", 0);
    cfg->setProperty("dimensions", 1);

    for (int i = 0; i < KisPipeBrushParasite::MaxDim; ++i) {
        cfg->setProperty("selectionMode" + QString::number(i), 2);
        cfg->getInt("rank" + QString::number(i), 0);
    }
    return cfg;
}

KisConfigWidget *KisBrushExport::createConfigurationWidget(QWidget *parent, const QByteArray &/*from*/, const QByteArray &to) const
{
    KisWdgOptionsBrush *wdg = new KisWdgOptionsBrush(parent);
    if (to == "image/x-gimp-brush") {
        wdg->groupBox->setVisible(false);
        wdg->animStyleGroup->setVisible(false);
    }
    else if (to == "image/x-gimp-brush-animated") {
        wdg->groupBox->setVisible(true);
        wdg->animStyleGroup->setVisible(true);
    }

    // preload gih name with chosen filename
    QFileInfo fileLocation(filename());
    wdg->nameLineEdit->setText(fileLocation.completeBaseName());
    return wdg;
}

void KisBrushExport::initializeCapabilities()
{
    QList<QPair<KoID, KoID> > supportedColorModels;
    supportedColorModels << QPair<KoID, KoID>()
            << QPair<KoID, KoID>(RGBAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(GrayAColorModelID, Integer8BitsColorDepthID);
    addSupportedColorModels(supportedColorModels, "Gimp Brushes");
    if (mimeType() == "image/x-gimp-brush-animated") {
        addCapability(KisExportCheckRegistry::instance()->get("MultiLayerCheck")->create(KisExportCheckBase::SUPPORTED));
    }
}


#include "kis_brush_export.moc"

