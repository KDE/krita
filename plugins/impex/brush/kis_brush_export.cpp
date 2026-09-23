/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include <KisDocument.h>
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
    bool preserveAlpha;
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
    } else {
        exportOptions.spacing = configuration->getDouble(KEY_SPACING);
    }

    if (!configuration->getString(KEY_NAME).isEmpty()) {
        exportOptions.name = configuration->getString(KEY_NAME);
    } else {
        exportOptions.name = document->savingImage()->objectName();
    }

    exportOptions.mask = configuration->getBool(KEY_MASK);
    exportOptions.preserveAlpha = configuration->getBool(KEY_PRESERVE_ALPHA);
    exportOptions.brushStyle = configuration->getInt(KEY_BRUSH_STYLE);
    exportOptions.dimensions = configuration->getInt(KEY_DIMENSIONS);

    for (int i = 0; i < KisPipeBrushParasite::MaxDim; ++i) {
        exportOptions.selectionModes[i] = configuration->getInt(KEY_SELECTION_MODE + QString::number(i));
        exportOptions.ranks[i] = configuration->getInt(KEY_RANK + QString::number(i));
    }

    KisGbrBrush *brush = 0;
    if (mimeType() == "image/x-gimp-brush") {
        brush = new KisGbrBrush(filename());
    } else if (mimeType() == "image/x-gimp-brush-animated") {
        brush = new KisImagePipeBrush(filename());
    } else {
        return ImportExportCodes::FileFormatIncorrect;
    }

    qApp->processEvents(); // For vector layers to be updated

    QRect rc = document->savingImage()->bounds();

    brush->setSpacing(exportOptions.spacing);

    KisImagePipeBrush *pipeBrush = dynamic_cast<KisImagePipeBrush*>(brush);
    if (pipeBrush) {
        // Create parasite.
        QVector< QVector<KisPaintDevice*> > devices;
        devices.append(QVector<KisPaintDevice*>());

        KoProperties properties;
        properties.setProperty("visible", true);
        QList<KisNodeSP> layers = document->savingImage()->root()->childNodes(QStringList("KisLayer"), properties);
        Q_FOREACH (KisNodeSP node, layers) {
            // push_front to behave exactly as gimp for gih creation
            devices[0].prepend(node->projection().data());
        }

        QVector<KisParasite::SelectionMode> modes;
        for (int i = 0; i < KisPipeBrushParasite::MaxDim; ++i) {
            modes.append((KisParasite::SelectionMode)exportOptions.selectionModes[i]);
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
    } else {
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
    enumBrushApplication maskMode;
    if (exportOptions.mask) {
        brush->makeMaskImage(exportOptions.preserveAlpha);
        maskMode = exportOptions.preserveAlpha ? LIGHTNESSMAP : ALPHAMASK;
    } else {
        maskMode = IMAGESTAMP;
    }
    brush->setBrushApplication(maskMode);
    brush->setWidth(rc.width());
    brush->setHeight(rc.height());

    if (brush->saveToDevice(io)) {
        return ImportExportCodes::OK;
    } else {
        return ImportExportCodes::Failure;
    }
}

KisPropertiesConfigurationSP KisBrushExport::defaultConfiguration(const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty(KEY_SPACING, 1.0);
    cfg->setProperty(KEY_NAME, "");
    cfg->setProperty(KEY_MASK, true);
    cfg->setProperty(KEY_PRESERVE_ALPHA, false);
    cfg->setProperty(KEY_BRUSH_STYLE, 0);
    cfg->setProperty(KEY_DIMENSIONS, 1);

    for (int i = 0; i < KisPipeBrushParasite::MaxDim; ++i) {
        cfg->setProperty(KEY_SELECTION_MODE + QString::number(i), KisParasite::Incremental);
        cfg->getInt(KEY_RANK + QString::number(i), 0);
    }
    return cfg;
}

KisConfigWidget *KisBrushExport::createConfigurationWidget(QWidget *parent, const QByteArray &/*from*/, const QByteArray &to) const
{
    KisWdgOptionsBrush *wdg = new KisWdgOptionsBrush(parent);
    if (to == "image/x-gimp-brush") {
        wdg->styleGroupBox->setVisible(false);
        wdg->animStyleGroup->setVisible(false);
    } else if (to == "image/x-gimp-brush-animated") {
        wdg->styleGroupBox->setVisible(true);
        wdg->animStyleGroup->setVisible(true);
    }

    wdg->setConfiguration(defaultConfiguration());

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
        addCapability(KisExportCheckRegistry::instance()->get("LayerOpacityCheck")->create(KisExportCheckBase::SUPPORTED));
    }
}


#include "kis_brush_export.moc"

