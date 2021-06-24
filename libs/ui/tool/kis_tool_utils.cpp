/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2018 Emmet & Eoin O'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kis_tool_utils.h>

#include <KoMixColorsOp.h>
#include <kis_group_layer.h>
#include <kis_transaction.h>
#include <kis_sequential_iterator.h>
#include <kis_properties_configuration.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include "kis_layer_utils.h"
#include "kis_command_utils.h"
#include "kis_processing_applicator.h"

namespace KisToolUtils {

    bool sampleColor(KoColor &out_color, KisPaintDeviceSP dev, const QPoint &pos,
                   KoColor const *const blendColor, int radius, int blend, bool pure)
    {
        KIS_ASSERT(dev);

        // Bugfix hack forcing pure on first sample to avoid wrong
        // format blendColor on newly initialized Krita.
        static bool firstTime = true;
        if (firstTime == true) {
            pure = true;
            firstTime = false;
        }

        const KoColorSpace *cs = dev->colorSpace();
        KoColor sampledColor(Qt::transparent, cs);

        // Sampling radius.
        if (!pure && radius > 1) {
            QScopedPointer<KoMixColorsOp::Mixer> mixer(cs->mixColorsOp()->createMixer());

            QVector<const quint8*> pixels;
            const int effectiveRadius = radius - 1;

            const QRect sampleRect(pos.x() - effectiveRadius, pos.y() - effectiveRadius,
                                 2 * effectiveRadius + 1, 2 * effectiveRadius + 1);
            KisSequentialConstIterator it(dev, sampleRect);

            const int radiusSq = pow2(effectiveRadius);

            int nConseqPixels = it.nConseqPixels();
            while (it.nextPixels(nConseqPixels)) {
                const QPoint realPos(it.x(),  it.y());
                if (kisSquareDistance(realPos, pos) < radiusSq) {
                    mixer->accumulateAverage(it.oldRawData(), nConseqPixels);
                }
            }

            mixer->computeMixedColor(sampledColor.data());

        } else {
            dev->pixel(pos.x(), pos.y(), &sampledColor);
        }
        
        // Color blending.
        if (!pure && blendColor && blend < 100) {
            //Scale from 0..100% to 0..255 range for mixOp weights.
            quint8 blendScaled = static_cast<quint8>(blend * 2.55f);

            const quint8 *colors[2];
            colors[0] = blendColor->data();
            colors[1] = sampledColor.data();
            qint16 weights[2];
            weights[0] = 255 - blendScaled;
            weights[1] = blendScaled;

            const KoMixColorsOp *mixOp = dev->colorSpace()->mixColorsOp();
            mixOp->mixColors(colors, weights, 2, sampledColor.data());
        }

        sampledColor.convertTo(dev->compositionSourceColorSpace());

        bool validColorSampled = sampledColor.opacityU8() != OPACITY_TRANSPARENT_U8;

        if (validColorSampled) {
            out_color = sampledColor;
        }

        return validColorSampled;
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

    bool clearImage(KisImageSP image, KisNodeList nodes, KisSelectionSP selection)
    {
        KisNodeList masks;

        Q_FOREACH (KisNodeSP node, nodes) {
            if (node->inherits("KisMask"))
                masks.append(node);
        }
        KisLayerUtils::filterMergableNodes(nodes);

        if (nodes.isEmpty() && masks.isEmpty()) {
            return false;
        }

        KisProcessingApplicator applicator(image, 0, KisProcessingApplicator::NONE,
                                           KisImageSignalVector(), kundo2_i18n("Clear"));

        std::function<void(KisNodeSP)> func = [&applicator, selection] (KisNodeSP node) {

                    if(node && node->hasEditablePaintDevice()) {
                        KUndo2Command *cmd =
                            new KisCommandUtils::LambdaCommand(kundo2_i18n("Clear"),
                                [node, selection] () {
                                    KisPaintDeviceSP device = node->paintDevice();

                                    KisTransaction transaction(kundo2_noi18n("internal-clear-command"), device);

                                    QRect dirtyRect;
                                    if (selection) {
                                        dirtyRect = selection->selectedRect();
                                        device->clearSelection(selection);
                                    } else {
                                        dirtyRect = device->extent();
                                        device->clear();
                                    }

                                    device->setDirty(dirtyRect);
                                    return transaction.endAndTake();
                                });
                        applicator.applyCommand(cmd, KisStrokeJobData::CONCURRENT);
                    }
                };

        Q_FOREACH (KisNodeSP node, nodes) {
            KisLayerUtils::recursiveApplyNodes(node, [&func] (KisNodeSP node) {
                if (!node->inherits("KisMask")) {
                    func(node);
                }
            });
        }

        // apply to masks only if explicitly selected
        Q_FOREACH (KisNodeSP node, masks) {
            func(node);
        }
        applicator.end();

        return true;
    }
    bool clearImage(KisImageSP image, KisNodeSP node, KisSelectionSP selection)
    {
        if(node && node->hasEditablePaintDevice()) {
            KUndo2Command *cmd =
                new KisCommandUtils::LambdaCommand(kundo2_i18n("Clear"),
                    [node, selection] () {
                        KisPaintDeviceSP device = node->paintDevice();

                        KisTransaction transaction(kundo2_noi18n("internal-clear-command"), device);

                        QRect dirtyRect;
                        if (selection) {
                            dirtyRect = selection->selectedRect();
                            device->clearSelection(selection);
                        } else {
                            dirtyRect = device->extent();
                            device->clear();
                        }

                        device->setDirty(dirtyRect);
                        return transaction.endAndTake();
                    });
            KisProcessingApplicator::runSingleCommandStroke(image, cmd);
            return true;
        }
        return false;
    }

    const QString ColorSamplerConfig::CONFIG_GROUP_NAME = "tool_color_sampler";

    ColorSamplerConfig::ColorSamplerConfig()
        : toForegroundColor(true)
        , updateColor(true)
        , addColorToCurrentPalette(false)
        , normaliseValues(false)
        , sampleMerged(true)
        , radius(1)
        , blend(100)
    {
    }

    void ColorSamplerConfig::save() const
    {
        KisPropertiesConfiguration props;
        props.setProperty("toForegroundColor", toForegroundColor);
        props.setProperty("updateColor", updateColor);
        props.setProperty("addPalette", addColorToCurrentPalette);
        props.setProperty("normaliseValues", normaliseValues);
        props.setProperty("sampleMerged", sampleMerged);
        props.setProperty("radius", radius);
        props.setProperty("blend", blend);

        KConfigGroup config =  KSharedConfig::openConfig()->group(CONFIG_GROUP_NAME);

        config.writeEntry("ColorSamplerDefaultActivation", props.toXML());
    }

    void ColorSamplerConfig::load()
    {
        KisPropertiesConfiguration props;

        KConfigGroup config =  KSharedConfig::openConfig()->group(CONFIG_GROUP_NAME);
        props.fromXML(config.readEntry("ColorSamplerDefaultActivation"));

        toForegroundColor = props.getBool("toForegroundColor", true);
        updateColor = props.getBool("updateColor", true);
        addColorToCurrentPalette = props.getBool("addPalette", false);
        normaliseValues = props.getBool("normaliseValues", false);
        sampleMerged = props.getBool("sampleMerged", true);
        radius = props.getInt("radius", 1);
        blend = props.getInt("blend", 100);
    }
}
