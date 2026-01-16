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

#include "kis_canvas2.h"
#include <QPainterPath>
#include <kis_shape_layer.h>
#include <KoShapeManager.h>
#include <KoShape.h>
#include <KoPathShape.h>
#include <KoSvgTextShape.h>
#include <KoSvgTextProperties.h>
#include <KisViewManager.h>
#include <kis_node_manager.h>
#include <KoSelection.h>
#include <kis_action_registry.h>
#include <KoToolBase.h>


#include "KisAnimAutoKey.h"

#include <QApplication>

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
        KoColor sampledColor = KoColor::createTransparent(cs);

        // Wrap around color sampling is supported on any paint device
        bool oldSupportsWraparound = dev->supportsWraproundMode();
        dev->setSupportsWraparoundMode(true);

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

        dev->setSupportsWraparoundMode(oldSupportsWraparound);
        
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

    KisNodeList findNodes(KisNodeSP node, const QPoint &point, bool wholeGroup, bool includeGroups, bool editableOnly)
    {
        KisNodeList foundNodes;
        while (node) {
            KisLayerSP layer = qobject_cast<KisLayer*>(node.data());

            if (!layer || !layer->isEditable()) {
                node = node->nextSibling();
                continue;
            }

            KoColor color(layer->projection()->colorSpace());
            layer->projection()->pixel(point.x(), point.y(), &color);
            const bool isTransparent = color.opacityU8() == OPACITY_TRANSPARENT_U8;

            KisGroupLayerSP group = dynamic_cast<KisGroupLayer*>(layer.data());

            if (group) {
                if (!isTransparent || group->passThroughMode()) {
                    foundNodes << findNodes(node->firstChild(), point, wholeGroup, includeGroups, editableOnly);
                    if (includeGroups) {
                        foundNodes << node;
                    }
                }
            } else {
                if (!isTransparent) {
                    if (wholeGroup) {
                        if (!foundNodes.contains(node->parent())) {
                            foundNodes << node->parent();
                        }
                    } else {
                        foundNodes << node;
                    }
                }
            }

            node = node->nextSibling();
        }

        return foundNodes;
    }

    bool clearImage(KisImageSP image, KisNodeList nodes, KisSelectionSP selection)
    {
        KisNodeList masks;

        Q_FOREACH (KisNodeSP node, nodes) {
            if (node->inherits("KisMask")) {
                masks.append(node);
            }
        }

        // To prevent deleting same layer multiple times
        KisLayerUtils::filterMergeableNodes(nodes);
        nodes.append(masks);

        if (nodes.isEmpty()) {
            return false;
        }

        KisProcessingApplicator applicator(image, 0, KisProcessingApplicator::NONE,
                                           KisImageSignalVector(), kundo2_i18n("Clear"));

        Q_FOREACH (KisNodeSP node, nodes) {
            KisLayerUtils::recursiveApplyNodes(node, [&applicator, selection, masks] (KisNodeSP node) {

                // applied on masks if selected explicitly
                if (node->inherits("KisMask") && !masks.contains(node)) {
                    return;
                }

                if(node->hasEditablePaintDevice()) {
                    KUndo2Command *cmd =
                        new KisCommandUtils::LambdaCommand(kundo2_i18n("Clear"),
                            [node, selection] () {
                                KisPaintDeviceSP device = node->paintDevice();

                                QScopedPointer<KisCommandUtils::CompositeCommand> parentCommand(
                                    new KisCommandUtils::CompositeCommand());

                                KUndo2Command *autoKeyframeCommand = KisAutoKey::tryAutoCreateDuplicatedFrame(device);
                                if (autoKeyframeCommand) {
                                    parentCommand->addCommand(autoKeyframeCommand);
                                }

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
                                parentCommand->addCommand(transaction.endAndTake());

                                return parentCommand.take();
                            });
                    applicator.applyCommand(cmd, KisStrokeJobData::CONCURRENT);
                }
            });
        }

        applicator.end();

        return true;
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

    void KRITAUI_EXPORT setCursorPos(const QPoint &point)
    {
        // https://bugreports.qt.io/browse/QTBUG-99009
        QScreen *screen = qApp->screenAt(point);
        if (!screen) {
            screen = qApp->primaryScreen();
        }
        QCursor::setPos(screen, point);
    }

    // get all shape layers with shapes at point. This is a bit coarser than 'FindNodes',
    // note that point is in Document coordinates instead of image coordinates.
    QList<KisShapeLayerSP> findShapeLayers(KisNodeSP root, const QPointF &point, bool editableOnly) {
        QList<KisShapeLayerSP> foundNodes;
        KisLayerUtils::recursiveApplyNodes(root, [&] (KisNodeSP node) {
            if ((node->isEditable(true) && editableOnly) || !editableOnly) {

                KisShapeLayerSP shapeLayer = dynamic_cast<KisShapeLayer*>(node.data());
                if (shapeLayer && shapeLayer->isEditable() && shapeLayer->shapeManager()->shapeAt(point)) {
                    foundNodes.append(shapeLayer);
                }
            }
        });
        return foundNodes;
    }

    QPainterPath shapeHoverInfoCrossLayer(KoCanvasBase *canvas, const QPointF &point, QString &shapeType, bool *isHorizontal, bool skipCurrentShapes)
    {
        QPainterPath p;
        KisCanvas2 *canvas2 = dynamic_cast<KisCanvas2*>(canvas);
        if (!canvas2) return p;

        QList<KoShape*> currentShapes = canvas->shapeManager()->selection()->selectedShapes();
        QList<KisShapeLayerSP> candidates = findShapeLayers(canvas2->image()->root(), point, true);
        KisShapeLayerSP shapeLayer = candidates.isEmpty()? nullptr: candidates.last();

        if (shapeLayer) {
            KoShape *shape = shapeLayer->shapeManager()->shapeAt(point);
            if (shape && !(currentShapes.contains(shape) && skipCurrentShapes)) {
                shapeType = shape->shapeId();
                KoSvgTextShape *t = dynamic_cast<KoSvgTextShape *>(shape);
                if (t) {
                    p.addRect(t->boundingRect());
                    if (isHorizontal) {
                        *isHorizontal = t->writingMode() == KoSvgText::HorizontalTB;
                    }
                    if (!t->shapesInside().isEmpty()) {
                        QPainterPath paths;
                        Q_FOREACH(KoShape *s, t->shapesInside()) {
                            KoPathShape *path = dynamic_cast<KoPathShape *>(s);
                            if (path) {
                                paths.addPath(path->absoluteTransformation().map(path->outline()));
                            }
                        }
                        if (!paths.isEmpty()) {
                            p = paths;
                        }
                    }
                } else {
                    p = shape->absoluteTransformation().map(shape->outline());
                }
            }
        }

        return p;
    }

    bool selectShapeCrossLayer(KoCanvasBase *canvas, const QPointF &point, const QString &shapeType, bool skipCurrentShapes)
    {
        KisCanvas2 *canvas2 = dynamic_cast<KisCanvas2*>(canvas);
        if (!canvas2) return false;

        QList<KoShape*> currentShapes = canvas->shapeManager()->selection()->selectedShapes();
        QList<KisShapeLayerSP> candidates = findShapeLayers(canvas2->image()->root(), point, true);
        KisShapeLayerSP shapeLayer = candidates.isEmpty()? nullptr: candidates.last();

        if (shapeLayer) {
            KoShape *shape = shapeLayer->shapeManager()->shapeAt(point);
            if (shape
                    && !(currentShapes.contains(shape) && skipCurrentShapes)
                    && (shapeType.isEmpty() || shapeType == shape->shapeId())) {
                canvas2->viewManager()->nodeManager()->slotNonUiActivatedNode(shapeLayer);
                canvas2->shapeManager()->selection()->deselectAll();
                canvas2->shapeManager()->selection()->select(shape);
            } else {
                return false;
            }
        } else {
            return false;
        }

        return true;
    }


    MoveShortcutsHelper::MoveShortcutsHelper(KoToolBase *tool)
        : m_tool(tool)
    {
    }

    QList<QAction*> MoveShortcutsHelper::createActions()
    {
        KisActionRegistry *actionRegistry = KisActionRegistry::instance();
        QList<QAction *> actions;

        actions << actionRegistry->makeQAction("movetool-move-up", actionRegistry);
        actions << actionRegistry->makeQAction("movetool-move-down", actionRegistry);
        actions << actionRegistry->makeQAction("movetool-move-left", actionRegistry);
        actions << actionRegistry->makeQAction("movetool-move-right", actionRegistry);

        actions << actionRegistry->makeQAction("movetool-move-up-more", actionRegistry);
        actions << actionRegistry->makeQAction("movetool-move-down-more", actionRegistry);
        actions << actionRegistry->makeQAction("movetool-move-left-more", actionRegistry);
        actions << actionRegistry->makeQAction("movetool-move-right-more", actionRegistry);

        return actions;
    }

    void MoveShortcutsHelper::setInternalMoveShortcutsEnabled(bool value)
    {
        m_tool->action("movetool-move-up")->setEnabled(value);
        m_tool->action("movetool-move-down")->setEnabled(value);
        m_tool->action("movetool-move-left")->setEnabled(value);
        m_tool->action("movetool-move-right")->setEnabled(value);
        m_tool->action("movetool-move-up-more")->setEnabled(value);
        m_tool->action("movetool-move-down-more")->setEnabled(value);
        m_tool->action("movetool-move-left-more")->setEnabled(value);
        m_tool->action("movetool-move-right-more")->setEnabled(value);
    }
}
