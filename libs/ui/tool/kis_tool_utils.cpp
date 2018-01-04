/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <kis_tool_utils.h>

#include <KoColorSpace.h>
#include <KoMixColorsOp.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <kis_group_layer.h>
#include <kis_wrapped_rect.h>
#include <kis_image.h>
#include <kis_transaction.h>
#include <kis_sequential_iterator.h>
#include <kis_properties_configuration.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

namespace KisToolUtils {

    bool pick(KisPaintDeviceSP dev, const QPoint& pos, KoColor *color, int radius)
    {
        KIS_ASSERT(dev);
        KoColor pickedColor;

        if (radius <= 1) {
            dev->pixel(pos.x(), pos.y(), &pickedColor);
        } else {
            const KoColorSpace* cs = dev->colorSpace();
            pickedColor = KoColor(Qt::transparent, cs);

            QVector<const quint8*> pixels;

            const int effectiveRadius = radius - 1;

            const QRect pickRect(pos.x() - effectiveRadius, pos.y() - effectiveRadius,
                                 2 * effectiveRadius + 1, 2 * effectiveRadius + 1);
            KisSequentialConstIterator it(dev, pickRect);

            const int radiusSq = pow2(effectiveRadius);

            while (it.nextPixel()) {
                const QPoint realPos(it.x(),  it.y());
                const QPoint pt = realPos - pos;
                if (pow2(pt.x()) + pow2(pt.y()) < radiusSq) {
                    pixels << it.oldRawData();
                }
            }

            const quint8** cpixels = const_cast<const quint8**>(pixels.constData());
            cs->mixColorsOp()->mixColors(cpixels, pixels.size(), pickedColor.data());
        }

        pickedColor.convertTo(dev->compositionSourceColorSpace());

        bool validColorPicked =
            pickedColor.opacityU8() != OPACITY_TRANSPARENT_U8;

        if (validColorPicked) {
            pickedColor.setOpacity(OPACITY_OPAQUE_U8);
            *color = pickedColor;
        }

        return validColorPicked;
    }

    KisNodeSP findNode(KisNodeSP node, const QPoint &point, bool wholeGroup, bool editableOnly)
    {
        KisNodeSP foundNode = 0;
        while (node) {
            KisLayerSP layer = qobject_cast<KisLayer*>(node.data());

            if (!layer || !layer->isEditable()) {
                node = node->prevSibling();
                continue;
            }

            KoColor color(layer->projection()->colorSpace());
            layer->projection()->pixel(point.x(), point.y(), &color);

            KisGroupLayerSP group = dynamic_cast<KisGroupLayer*>(layer.data());

            if ((group && group->passThroughMode()) ||  color.opacityU8() != OPACITY_TRANSPARENT_U8) {
                if (layer->inherits("KisGroupLayer") && (!editableOnly || layer->isEditable())) {
                    // if this is a group and the pixel is transparent, don't even enter it
                    foundNode = findNode(node->lastChild(), point, wholeGroup, editableOnly);
                }
                else {
                    foundNode = !wholeGroup ? node : node->parent();
                }

            }

            if (foundNode) break;

            node = node->prevSibling();
        }

        return foundNode;
    }

    bool clearImage(KisImageSP image, KisNodeSP node, KisSelectionSP selection)
    {
        if(node && node->hasEditablePaintDevice()) {
            KisPaintDeviceSP device = node->paintDevice();

            image->barrierLock();
            KisTransaction transaction(kundo2_i18n("Clear"), device);

            QRect dirtyRect;
            if (selection) {
                dirtyRect = selection->selectedRect();
                device->clearSelection(selection);
            }
            else {
                dirtyRect = device->extent();
                device->clear();
            }

            transaction.commit(image->undoAdapter());
            device->setDirty(dirtyRect);
            image->unlock();
            return true;
        }
        return false;
    }

    const QString ColorPickerConfig::CONFIG_GROUP_NAME = "tool_color_picker";

    ColorPickerConfig::ColorPickerConfig()
        : toForegroundColor(true)
        , updateColor(true)
        , addPalette(false)
        , normaliseValues(false)
        , sampleMerged(true)
        , radius(1)
    {
    }

    inline QString getConfigKey(bool defaultActivation) {
        return defaultActivation ?
            "ColorPickerDefaultActivation" : "ColorPickerTemporaryActivation";
    }

    void ColorPickerConfig::save(bool defaultActivation) const
    {
        KisPropertiesConfiguration props;
        props.setProperty("toForegroundColor", toForegroundColor);
        props.setProperty("updateColor", updateColor);
        props.setProperty("addPalette", addPalette);
        props.setProperty("normaliseValues", normaliseValues);
        props.setProperty("sampleMerged", sampleMerged);
        props.setProperty("radius", radius);

        KConfigGroup config =  KSharedConfig::openConfig()->group(CONFIG_GROUP_NAME);

        config.writeEntry(getConfigKey(defaultActivation), props.toXML());
    }

    void ColorPickerConfig::load(bool defaultActivation)
    {
        KisPropertiesConfiguration props;

        KConfigGroup config =  KSharedConfig::openConfig()->group(CONFIG_GROUP_NAME);
        props.fromXML(config.readEntry(getConfigKey(defaultActivation)));

        toForegroundColor = props.getBool("toForegroundColor", true);
        updateColor = props.getBool("updateColor", true);
        addPalette = props.getBool("addPalette", false);
        normaliseValues = props.getBool("normaliseValues", false);
        sampleMerged = props.getBool("sampleMerged", !defaultActivation ? false : true);
        radius = props.getInt("radius", 1);
    }

}
