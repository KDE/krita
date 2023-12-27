/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_qmic_interface.h"

#include <KisImageSignals.h>
#include <KisViewManager.h>
#include <kis_algebra_2d.h>
#include <kis_debug.h>
#include <kis_image.h>
#include <KisImageBarrierLock.h>
#include <kis_processing_applicator.h>
#include <kis_selection.h>
#include <kundo2magicstring.h>

#include "gmic.h"
#include "kis_qmic_import_tools.h"
#include "kis_qmic_simple_convertor.h"
#include "kis_qmic_synchronize_layers_command.h"

struct KisImageInterface::Private {
    Private() = default;

    KisViewManager *m_viewManager{nullptr};
    InputLayerMode m_inputMode{InputLayerMode::Active};
    OutputMode m_outputMode{OutputMode::InPlace};
    QVector<KisQMicImageSP> m_sharedMemorySegments{};
    KisQmicApplicator *m_gmicApplicator{nullptr};
};

KisImageInterface::KisImageInterface(KisViewManager *parent)
    : p(new Private)
{
    p->m_viewManager = parent;
    KIS_ASSERT(p->m_viewManager);
}

KisImageInterface::~KisImageInterface() = default;

QSize KisImageInterface::gmic_qt_get_image_size(int mode)
{
    if (!p->m_viewManager)
        return {};

    KisSelectionSP selection = p->m_viewManager->image()->globalSelection();

    if (selection) {
        QRect selectionRect = selection->selectedExactRect();
        return selectionRect.size();
    } else {
        p->m_inputMode = static_cast<InputLayerMode>(mode);

        QSize size(0, 0);

        dbgPlugins << "getImageSize()" << mode;

        KisNodeListSP nodes =
            KisQmicImportTools::inputNodes(p->m_viewManager->image(),
                                           p->m_inputMode,
                                           p->m_viewManager->activeNode());
        if (nodes->isEmpty()) {
            return size;
        }

        switch (p->m_inputMode) {
        case InputLayerMode::NoInput:
        case InputLayerMode::AllInvisible:
            break;
        case InputLayerMode::Active:
        case InputLayerMode::ActiveAndBelow: {
            // The last layer in the list is always the layer the user
            // has selected in Paint.NET, so it will be treated as the
            // active layer. The clipboard layer (if present) will be
            // placed above the active layer.
            const auto activeLayer = nodes->last();

            // G'MIC takes as the image size the size of the active layer.
            // This is a lie since a query to All will imply a query to Active
            // through CroppedActiveLayerProxy, and the latter will return a
            // bogus size if we reply straight with the paint device's bounds.
            if (activeLayer && activeLayer->paintDevice()) {
                const QSize layerSize = activeLayer->exactBounds().size();
                const QSize imageSize = activeLayer->image()->bounds().size();
                size = size.expandedTo(layerSize).expandedTo(imageSize);
            }
        } break;
        case InputLayerMode::All:
        case InputLayerMode::ActiveAndAbove:
        case InputLayerMode::AllVisible:
            for (auto &node : *nodes) {
                if (node && node->paintDevice()) {
                    // XXX: when using All, G'MIC will instead do another query
                    // through CroppedActiveLayerProxy to determine the image's
                    // "size". So we need to be both consistent with the image's
                    // bounds, but also extend them in case the layer's
                    // partially offscreen.
                    const QSize layerSize = node->exactBounds().size();
                    const QSize imageSize = node->image()->bounds().size();
                    size = size.expandedTo(layerSize).expandedTo(imageSize);
                }
            }
            break;
        case InputLayerMode::AllVisiblesDesc_DEPRECATED:
        case InputLayerMode::AllInvisiblesDesc_DEPRECATED:
        case InputLayerMode::AllDesc_DEPRECATED: {
            warnPlugins << "Inputmode" << static_cast<int>(p->m_inputMode)
                        << "is not supported by GMic anymore";
            break;
        }
        default: {
            warnPlugins
                << "Inputmode" << static_cast<int>(p->m_inputMode)
                << "must be specified by GMic or is not implemented in Krita";
            break;
        }
        }

        return size;
    }
}

QVector<KisQMicImageSP> KisImageInterface::gmic_qt_get_cropped_images(int inputMode, QRectF &rc)
{
    // Create the shared memory segments, and create a new "message" to send back

    QVector<KisQMicImageSP> message;

    if (!p->m_viewManager)
        return {};

    if (!p->m_viewManager->image()->tryBarrierLock(true)) return {};

    KisImageBarrierLock lock(p->m_viewManager->image(), std::adopt_lock);

    p->m_inputMode = static_cast<InputLayerMode>(inputMode);

    dbgPlugins << "prepareCroppedImages()" << message << rc << inputMode;

    KisNodeListSP nodes =
        KisQmicImportTools::inputNodes(p->m_viewManager->image(),
                                       p->m_inputMode,
                                       p->m_viewManager->activeNode());
    if (nodes->isEmpty()) {
        return {};
    }

    for (auto &node : *nodes) {
         if (node && node->paintDevice()) {
            KisSelectionSP selection = p->m_viewManager->image()->globalSelection();

            const QRect cropRect = [&]() {
                if (selection) {
                    return selection->selectedExactRect();
                } else {
                    // XXX: This is a lie, see gmic_qt_get_image_size as to why
                    const QRect nodeBounds = node->exactBounds();
                    const QRect imageBounds = node->image()->bounds();

                    if (imageBounds.contains(nodeBounds)) {
                        return imageBounds;
                    } else {
                        return nodeBounds.united(imageBounds);
                    }
                }
            }();

            dbgPlugins << "Converting node" << node->name() << cropRect;

            const QRectF mappedRect = KisAlgebra2D::mapToRect(cropRect).mapRect(rc);
            const QRect resultRect = mappedRect.toAlignedRect();

            QString noParenthesisName(node->name());
            noParenthesisName.replace(QChar('('), QChar(21)).replace(QChar(')'), QChar(22));

            const auto translatedMode = KisQmicSimpleConvertor::blendingModeToString(node->compositeOpId());

            const QString name = QString("mode(%1),opacity(%2),pos(%3,%4),name(%5)")
                               .arg(translatedMode)
                               .arg(node->percentOpacity())
                               .arg(cropRect.x())
                               .arg(cropRect.y())
                               .arg(noParenthesisName);

            auto m = KisQMicImageSP::create(name, resultRect.width(), resultRect.height(), 4);
            p->m_sharedMemorySegments << m;

            {
                QMutexLocker lock(&m->m_mutex);

                KisQmicSimpleConvertor::convertToGmicImageFast(
                    node->paintDevice(),
                    *m.data(),
                    resultRect);
            }

            message << m;
        }
    }

    dbgPlugins << message;

    return message;
}

void KisImageInterface::gmic_qt_detach()
{
    p->m_sharedMemorySegments.clear();
}

void KisImageInterface::gmic_qt_output_images(int mode, QVector<KisQMicImageSP> layers)
{
    // Parse the message. read the shared memory segments, fix up the current image and send an ack
    dbgPlugins << "gmic_qt_output_images";
    p->m_outputMode = (OutputMode)mode;
    if (p->m_outputMode != OutputMode::InPlace) {
        errPlugins << "Requested mode" << static_cast<int>(p->m_outputMode) << "which is not implemented yet";
        p->m_outputMode = OutputMode::InPlace;
    }

    dbgPlugins << "slotStartApplicator();" << layers;
    if (!p->m_viewManager)
        return;

    dbgPlugins << "Got" << layers.size() << "gmic images";

    {
        // Start the applicator
        KUndo2MagicString actionName = kundo2_i18n("G'MIC filter");
        KisNodeSP rootNode = p->m_viewManager->image()->root();
        KisNodeListSP mappedLayers =
            KisQmicImportTools::inputNodes(p->m_viewManager->image(),
                                           p->m_inputMode,
                                           p->m_viewManager->activeNode());
        // p->m_gmicApplicator->setProperties(p->m_viewManager->image(), rootNode, images, actionName, layers);
        // p->m_gmicApplicator->apply();

        KisImageSignalVector emitSignals;
        emitSignals << ComplexSizeChangedSignal();

        KisProcessingApplicator applicator(p->m_viewManager->image(),
                                           rootNode,
                                           KisProcessingApplicator::RECURSIVE | KisProcessingApplicator::NO_UI_UPDATES,
                                           emitSignals,
                                           actionName);
        dbgPlugins << "Created applicator " << &applicator;

        QRect layerSize;
        KisSelectionSP selection = p->m_viewManager->image()->globalSelection();
        if (selection) {
            layerSize = selection->selectedExactRect();
        } else {
            layerSize = QRect(0, 0, p->m_viewManager->image()->width(), p->m_viewManager->image()->height());
        }

        // This is a three-stage process.

        // 1. Layer sizes must be adjusted individually
        // 2. synchronize layer count and convert excess GMic nodes to paint
        // layers
        // 3. visit the existing nodes and reuse them to apply the remaining
        // changes from GMic
        applicator.applyCommand(
            new KisQmicSynchronizeLayersCommand(mappedLayers,
                                                layers,
                                                p->m_viewManager->image(),
                                                layerSize,
                                                selection),
            KisStrokeJobData::SEQUENTIAL,
            KisStrokeJobData::EXCLUSIVE);

        applicator.end();
    }
}

QDebug operator<<(QDebug d, const KisQMicImage &model)
{
    d << QString("0x%1,%2,%3,%4").arg((quintptr)&model).arg(QString(model.m_layerName.toUtf8().toHex())).arg(model.m_width).arg(model.m_height);
    return d;
}
