/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_xcf_import.h"

#include <ctype.h>

#include <QApplication>
#include <QFile>
#include <qendian.h>

#include <kpluginfactory.h>
#include <QFileInfo>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceTraits.h>
#include <KoCompositeOpRegistry.h>
#include <kis_debug.h>
#include <KisDocument.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_transaction.h>
#include <kis_paint_layer.h>
#include <kis_transparency_mask.h>
#include "kis_iterator_ng.h"
#include "kis_types.h"
#include <KoColorModelStandardIds.h>
extern "C" {

#include "xcftools.h"
#include "pixels.h"

#define GET_RED(x) (x >> RED_SHIFT)
#define GET_GREEN(x) (x >> GREEN_SHIFT)
#define GET_BLUE(x) (x >> BLUE_SHIFT)
#define GET_ALPHA(x) (x >> ALPHA_SHIFT)
}

QString layerModeG2K(GimpLayerModeEffects mode)
{
    switch (mode) {
    case GIMP_NORMAL_MODE:
        return COMPOSITE_OVER;
    case GIMP_DISSOLVE_MODE:
        return COMPOSITE_DISSOLVE;
    case GIMP_MULTIPLY_MODE:
        return COMPOSITE_MULT;
    case GIMP_SCREEN_MODE:
        return COMPOSITE_SCREEN;
    case GIMP_OVERLAY_MODE:
    case GIMP_SOFTLIGHT_MODE:
        return COMPOSITE_OVERLAY;
    case GIMP_DIFFERENCE_MODE:
        return COMPOSITE_DIFF;
    case GIMP_ADDITION_MODE:
        return COMPOSITE_ADD;
    case GIMP_SUBTRACT_MODE:
        return COMPOSITE_SUBTRACT;
    case GIMP_DARKEN_ONLY_MODE:
        return COMPOSITE_DARKEN;
    case GIMP_LIGHTEN_ONLY_MODE:
        return COMPOSITE_LIGHTEN;
    case GIMP_HUE_MODE:
        return COMPOSITE_HUE_HSL;
    case GIMP_SATURATION_MODE:
        return COMPOSITE_SATURATION_HSV;
    case GIMP_COLOR_MODE:
        return COMPOSITE_COLOR_HSL;
    case GIMP_VALUE_MODE:
        return COMPOSITE_VALUE;
    case GIMP_DIVIDE_MODE:
        return COMPOSITE_DIVIDE;
    case GIMP_DODGE_MODE:
        return COMPOSITE_DODGE;
    case GIMP_BURN_MODE:
        return COMPOSITE_BURN;
    case GIMP_ERASE_MODE:
        return COMPOSITE_ERASE;
    case GIMP_REPLACE_MODE:
        return COMPOSITE_COPY;
    case GIMP_HARDLIGHT_MODE:
        return COMPOSITE_HARD_LIGHT;
    case GIMP_COLOR_ERASE_MODE:
    case GIMP_NORMAL_NOPARTIAL_MODE:
    case GIMP_ANTI_ERASE_MODE:
    case GIMP_GRAIN_EXTRACT_MODE:
        return COMPOSITE_GRAIN_EXTRACT;
    case GIMP_GRAIN_MERGE_MODE:
        return COMPOSITE_GRAIN_MERGE;
    case GIMP_BEHIND_MODE:
        break;
    }
    dbgFile << "Unknown mode: " << mode;
    return COMPOSITE_OVER;
}

struct Layer {
    KisLayerSP layer;
    int depth;
    KisMaskSP mask;
};

KisGroupLayerSP findGroup(const QVector<Layer> &layers, const Layer& layer, int i)
{
    for (; i < layers.size(); ++i) {
        KisGroupLayerSP group = dynamic_cast<KisGroupLayer*>(const_cast<KisLayer*>(layers[i].layer.data()));
        if (group && (layers[i].depth == layer.depth -1)) {
            return group;
        }
    }
    return 0;
}

void addLayers(const QVector<Layer> &layers, KisImageSP image, int depth)
{
    for(int i = 0; i < layers.size(); i++) {
        const Layer &layer = layers[i];
        if (layer.depth == depth) {
            KisGroupLayerSP group = (depth == 0 ? image->rootLayer() : findGroup(layers, layer, i));
            image->addNode(layer.layer, group);
            if (layer.mask) {
                image->addNode(layer.mask, layer.layer);
            }
        }
    }
}

K_PLUGIN_FACTORY_WITH_JSON(XCFImportFactory, "krita_xcf_import.json", registerPlugin<KisXCFImport>();)

KisXCFImport::KisXCFImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisXCFImport::~KisXCFImport()
{
}

ImportExport::ErrorCode KisXCFImport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    dbgFile << "Start decoding file";
    QByteArray data = io->readAll();
    xcf_file = (uint8_t*)data.data();
    xcf_length = data.size();
    io->close();

    // Decode the data
    getBasicXcfInfo() ;

    if (XCF.version < 0 || XCF.version > 3) {
        document->setErrorMessage(i18n("This XCF file is too new; Krita cannot support XCF files written by GIMP 2.9 or newer."));
        return ImportExport::ErrorCodeID::FormatFeaturesUnsupported;
    }

    initColormap();

    dbgFile << XCF.version << "width = " << XCF.width << "height = " << XCF.height << "layers = " << XCF.numLayers;

    // Create the image
    KisImageSP image = new KisImage(document->createUndoStore(), XCF.width, XCF.height, KoColorSpaceRegistry::instance()->rgb8(), "built image");

    QVector<Layer> layers;
    uint maxDepth = 0;

    // Read layers
    for (int i = 0; i < XCF.numLayers; ++i) {

        Layer layer;

        xcfLayer& xcflayer = XCF.layers[i];
        dbgFile << i << " name = " << xcflayer.name << " opacity = " << xcflayer.opacity << "group:" << xcflayer.isGroup << xcflayer.pathLength;
        dbgFile << ppVar(xcflayer.dim.width) << ppVar(xcflayer.dim.height) << ppVar(xcflayer.dim.tilesx) << ppVar(xcflayer.dim.tilesy) << ppVar(xcflayer.dim.ntiles) << ppVar(xcflayer.dim.c.t) << ppVar(xcflayer.dim.c.l) << ppVar(xcflayer.dim.c.r) << ppVar(xcflayer.dim.c.b);

        maxDepth = qMax(maxDepth, xcflayer.pathLength);

        bool isRgbA = false;
        // Select the color space
        const KoColorSpace* colorSpace = 0;
        switch (xcflayer.type) {
        case GIMP_INDEXED_IMAGE:
        case GIMP_INDEXEDA_IMAGE:
        case GIMP_RGB_IMAGE:
        case GIMP_RGBA_IMAGE:
            colorSpace = KoColorSpaceRegistry::instance()->rgb8();
            isRgbA = true;
            break;
        case GIMP_GRAY_IMAGE:
        case GIMP_GRAYA_IMAGE:
            colorSpace = KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), "");
            isRgbA = false;
            break;
        }

        // Create the layer
        KisLayerSP kisLayer;
        if (xcflayer.isGroup) {
            kisLayer = new KisGroupLayer(image, QString::fromUtf8(xcflayer.name), xcflayer.opacity);
        }
        else {
            kisLayer = new KisPaintLayer(image, QString::fromUtf8(xcflayer.name), xcflayer.opacity, colorSpace);
        }

        // Set some properties
        kisLayer->setCompositeOpId(layerModeG2K(xcflayer.mode));
        kisLayer->setVisible(xcflayer.isVisible);
        kisLayer->disableAlphaChannel(xcflayer.mode != GIMP_NORMAL_MODE);

        layer.layer = kisLayer;
        layer.depth = xcflayer.pathLength;

        // Copy the data in the image
        initLayer(&xcflayer);

        int left = xcflayer.dim.c.l;
        int top = xcflayer.dim.c.t;

        if (!xcflayer.isGroup) {

            // Copy the data;
            for (unsigned int x = 0; x < xcflayer.dim.width; x += TILE_WIDTH) {
                for (unsigned int y = 0; y < xcflayer.dim.height; y += TILE_HEIGHT) {
                    rect want;
                    want.l = x + left;
                    want.t = y + top;
                    want.b = want.t + TILE_HEIGHT;
                    want.r = want.l + TILE_WIDTH;
                    Tile* tile = getMaskOrLayerTile(&xcflayer.dim, &xcflayer.pixels, want);
                    KisHLineIteratorSP it = kisLayer->paintDevice()->createHLineIteratorNG(x, y, TILE_WIDTH);
                    rgba* data = tile->pixels;
                    for (int v = 0; v < TILE_HEIGHT; ++v) {
                        if (isRgbA) {
                            // RGB image
                           do {
                                KoBgrTraits<quint8>::setRed(it->rawData(), GET_RED(*data));
                                KoBgrTraits<quint8>::setGreen(it->rawData(), GET_GREEN(*data));
                                KoBgrTraits<quint8>::setBlue(it->rawData(), GET_BLUE(*data));
                                KoBgrTraits<quint8>::setOpacity(it->rawData(), quint8(GET_ALPHA(*data)), 1);
                                ++data;
                            } while (it->nextPixel());
                        } else {
                            // Grayscale image
                            do {
                                it->rawData()[0] = GET_RED(*data);
                                it->rawData()[1] = GET_ALPHA(*data);
                                ++data;
                            } while (it->nextPixel());
                        }
                        it->nextRow();
                    }
                }
            }

            // Move the layer to its position
            kisLayer->paintDevice()->setX(left);
            kisLayer->paintDevice()->setY(top);
        }
        // Create the mask
        if (xcflayer.hasMask) {
            KisTransparencyMaskSP mask = new KisTransparencyMask();
            layer.mask = mask;

            mask->initSelection(kisLayer);
            for (unsigned int x = 0; x < xcflayer.dim.width; x += TILE_WIDTH) {
                for (unsigned int y = 0; y < xcflayer.dim.height; y += TILE_HEIGHT) {
                    rect want;
                    want.l = x + left;
                    want.t = y + top;
                    want.b = want.t + TILE_HEIGHT;
                    want.r = want.l + TILE_WIDTH;
                    Tile* tile = getMaskOrLayerTile(&xcflayer.dim, &xcflayer.mask, want);
                    KisHLineIteratorSP it = mask->paintDevice()->createHLineIteratorNG(x, y, TILE_WIDTH);
                    rgba* data = tile->pixels;
                    for (int v = 0; v < TILE_HEIGHT; ++v) {
                        do {
                            it->rawData()[0] = GET_ALPHA(*data);
                            ++data;
                        } while (it->nextPixel());
                        it->nextRow();
                    }

                }
            }
            mask->paintDevice()->setX(left);
            mask->paintDevice()->setY(top);
        }

        dbgFile << xcflayer.pixels.tileptrs;
        layers.append(layer);
    }

    for (uint i = 0; i <= maxDepth; ++i) {
        addLayers(layers, image, i);
    }

    document->setCurrentImage(image);
    return ImportExport::ErrorCodeID::OK;

}

#include "kis_xcf_import.moc"
