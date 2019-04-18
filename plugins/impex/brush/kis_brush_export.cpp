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
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_spacing_selection_widget.h>
#include <kis_gbr_brush.h>
#include <kis_imagepipe_brush.h>
#include <kis_pipebrush_parasite.h>
#include <KisAnimatedBrushAnnotation.h>
#include <KisImportExportManager.h>
#include <kis_config.h>

struct KisBrushExportOptions {
    qreal spacing;
    bool mask;
    int brushStyle;
    int selectionMode;
    QString name;
};


K_PLUGIN_FACTORY_WITH_JSON(KisBrushExportFactory, "krita_brush_export.json", registerPlugin<KisBrushExport>();)

KisBrushExport::KisBrushExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisBrushExport::~KisBrushExport()
{
}

ImportExport::ErrorCode KisBrushExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration)
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
    exportOptions.selectionMode = configuration->getInt("selectionMode");
    exportOptions.brushStyle = configuration->getInt("brushStyle");

    KisGbrBrush *brush = 0;
    if (mimeType() == "image/x-gimp-brush") {
        brush = new KisGbrBrush(filename());
    }
    else if (mimeType() == "image/x-gimp-brush-animated") {
        brush = new KisImagePipeBrush(filename());
    }
    else {
        return ImportExport::ErrorCodeID::FileFormatIncorrect;
    }

    qApp->processEvents(); // For vector layers to be updated

    QRect rc = document->savingImage()->bounds();

    brush->setName(exportOptions.name);
    brush->setSpacing(exportOptions.spacing);
    brush->setUseColorAsMask(exportOptions.mask);




    KisImagePipeBrush *pipeBrush = dynamic_cast<KisImagePipeBrush*>(brush);
    if (pipeBrush) {
        // Create parasite. XXX: share with KisCustomBrushWidget
        QVector< QVector<KisPaintDevice*> > devices;
        devices.push_back(QVector<KisPaintDevice*>());

        KoProperties properties;
        properties.setProperty("visible", true);
        QList<KisNodeSP> layers = document->savingImage()->root()->childNodes(QStringList("KisLayer"), properties);

        Q_FOREACH (KisNodeSP node, layers) {
            devices[0].push_back(node->projection().data());
        }

        QVector<KisParasite::SelectionMode > modes;
        switch (exportOptions.selectionMode) {
        case 0: modes.push_back(KisParasite::Constant); break;
        case 1: modes.push_back(KisParasite::Random); break;
        case 2: modes.push_back(KisParasite::Incremental); break;
        case 3: modes.push_back(KisParasite::Pressure); break;
        case 4: modes.push_back(KisParasite::Angular); break;
        default: modes.push_back(KisParasite::Incremental);
        }

        KisPipeBrushParasite parasite;

        // XXX: share code with KisImagePipeBrush, when we figure out how to support more gih features
        parasite.dim = devices.count();
        // XXX Change for multidim! :
        parasite.ncells = devices.at(0).count();
        parasite.rank[0] = parasite.ncells; // ### This can mask some bugs, be careful here in the future
        parasite.selection[0] = modes.at(0);
        // XXX needs movement!
        parasite.setBrushesCount();
        pipeBrush->setParasite(parasite);
        pipeBrush->setDevices(devices, rc.width(), rc.height());
    }
    else {
        QImage image = document->savingImage()->projection()->convertToQImage(0, 0, 0, rc.width(), rc.height(), KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
        image.save("~/bla.png");
        brush->setImage(image);
        brush->setBrushTipImage(image);
    }

    brush->setWidth(rc.width());
    brush->setHeight(rc.height());

    if (brush->saveToDevice(io)) {
        return ImportExport::ErrorCodeID::OK;
    }
    else {
        return ImportExport::ErrorCodeID::Failure;
    }
}

KisPropertiesConfigurationSP KisBrushExport::defaultConfiguration(const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("spacing", 1.0);
    cfg->setProperty("name", "");
    cfg->setProperty("mask", true);
    cfg->setProperty("selectionMode", 0);
    cfg->setProperty("brushStyle", 0);
    return cfg;
}

KisConfigWidget *KisBrushExport::createConfigurationWidget(QWidget *parent, const QByteArray &/*from*/, const QByteArray &to) const
{
    KisWdgOptionsBrush *wdg = new KisWdgOptionsBrush(parent);
    if (to == "image/x-gimp-brush") {
        wdg->groupBox->setVisible(false);
    }
    else if (to == "image/x-gimp-brush-animated") {
        wdg->groupBox->setVisible(true);
    }
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


void KisWdgOptionsBrush::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    spacingWidget->setSpacing(false, cfg->getDouble("spacing"));
    nameLineEdit->setText(cfg->getString("name"));
    colorAsMask->setChecked(cfg->getBool("mask"));
    brushStyle->setCurrentIndex(cfg->getInt("brushStyle"));
    cmbSelectionMode->setCurrentIndex(cfg->getInt("selectionMode"));
}

KisPropertiesConfigurationSP KisWdgOptionsBrush::configuration() const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("spacing", spacingWidget->spacing());
    cfg->setProperty("name", nameLineEdit->text());
    cfg->setProperty("mask", colorAsMask->isChecked());
    cfg->setProperty("selectionMode", cmbSelectionMode->currentIndex());
    cfg->setProperty("brushStyle", brushStyle->currentIndex());
    return cfg;
}


#include "kis_brush_export.moc"

