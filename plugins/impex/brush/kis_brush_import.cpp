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

#include "kis_brush_import.h"

#include <QCheckBox>
#include <QBuffer>
#include <QSlider>
#include <QApplication>

#include <kpluginfactory.h>
#include <QFileInfo>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>

#include <KisDocument.h>

#include <kis_transaction.h>
#include <kis_paint_device.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_node.h>
#include <kis_group_layer.h>

#include <kis_gbr_brush.h>
#include <kis_imagepipe_brush.h>
#include <KisAnimatedBrushAnnotation.h>

K_PLUGIN_FACTORY_WITH_JSON(KisBrushImportFactory, "krita_brush_import.json", registerPlugin<KisBrushImport>();)

KisBrushImport::KisBrushImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisBrushImport::~KisBrushImport()
{
}


KisImportExportErrorCode KisBrushImport::convert(KisDocument *document, QIODevice *io, KisPropertiesConfigurationSP /*configuration*/)
{
    KisBrushSP brush;

    if (mimeType() == "image/x-gimp-brush") {
        brush = KisBrushSP(new KisGbrBrush(filename()));
    }
    else if (mimeType() == "image/x-gimp-brush-animated") {
        brush = KisBrushSP(new KisImagePipeBrush(filename()));
    }
    else {
        return ImportExportCodes::FileFormatIncorrect;
    }

    if (!brush->loadFromDevice(io)) {
        delete brush;
        return ImportExportCodes::FileFormatIncorrect;
    }

    if (!brush->valid()) {
        delete brush;
        return ImportExportCodes::FileFormatIncorrect;;
    }

    const KoColorSpace *colorSpace = 0;
    if (brush->hasColor()) {
        colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    }
    else {
        colorSpace = KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), "");
    }

    KisImageSP image = new KisImage(document->createUndoStore(), brush->width(), brush->height(), colorSpace, brush->name());
    image->setProperty("brushspacing", brush->spacing());

    KisImagePipeBrushSP pipeBrush = brush.dynamicCast<KisImagePipeBrush>();
    if (pipeBrush) {
        QVector<KisGbrBrushSP> brushes = pipeBrush->brushes();
        for(int i = brushes.size(); i > 0; i--) {
            KisGbrBrushSP subbrush = brushes.at(i - 1);
            const KoColorSpace *subColorSpace = 0;
            if (brush->hasColor()) {
                subColorSpace = KoColorSpaceRegistry::instance()->rgb8();
            }
            else {
                subColorSpace = KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), "");
            }
            KisPaintLayerSP layer = new KisPaintLayer(image, image->nextLayerName(), 255, subColorSpace);
            layer->paintDevice()->convertFromQImage(subbrush->brushTipImage(), 0, 0, 0);
            image->addNode(layer, image->rootLayer());
        }
        KisAnnotationSP ann = new KisAnimatedBrushAnnotation(pipeBrush->parasite());
        image->addAnnotation(ann);
    }
    else {
        KisPaintLayerSP layer = new KisPaintLayer(image, image->nextLayerName(), 255, colorSpace);
        layer->paintDevice()->convertFromQImage(brush->brushTipImage(), 0, 0, 0);
        image->addNode(layer, image->rootLayer(), 0);
    }

    document->setCurrentImage(image);
    delete brush;
    return ImportExportCodes::OK;

}
#include "kis_brush_import.moc"
