/*
 *  SPDX-FileCopyrightText: 1999 Matthias Elter <me@kde.org>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_select_similar.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include <ksharedconfig.h>

#include <KoColorSpace.h>

#include "kis_canvas2.h"
#include "kis_command_utils.h"
#include "kis_image.h"
#include "kis_iterator_ng.h"
#include "kis_selection_tool_helper.h"
#include "kis_slider_spin_box.h"
#include "krita_utils.h"
#include <KoPointerEvent.h>
#include <kis_cursor.h>
#include <kis_paint_device.h>
#include <kis_pixel_selection.h>
#include <kis_selection_filters.h>
#include <kis_selection_options.h>

void selectByColor(KisPaintDeviceSP dev,
                   KisPixelSelectionSP selection,
                   const quint8 *c,
                   int threshold,
                   const QRect &rc)
{
    if (rc.isEmpty()) {
        return;
    }
    // XXX: Multithread this!
    qint32 x, y, w, h;
    x = rc.x();
    y = rc.y();
    w = rc.width();
    h = rc.height();

    const KoColorSpace * cs = dev->colorSpace();

    KisHLineConstIteratorSP hiter = dev->createHLineConstIteratorNG(x, y, w);
    KisHLineIteratorSP selIter = selection->createHLineIteratorNG(x, y, w);

    quint8 wantedOpacity = cs->opacityU8(c);

    for (int row = y; row < y + h; ++row) {
        do {
            if (threshold == 1) {
                if (wantedOpacity == 0 && cs->opacityU8(hiter->rawDataConst()) == 0) {
                    *(selIter->rawData()) = MAX_SELECTED;
                }
                else if (memcmp(c, hiter->rawDataConst(), cs->pixelSize()) == 0) {
                    *(selIter->rawData()) = MAX_SELECTED;
                }
            } else {
                quint8 match = cs->difference(c, hiter->rawDataConst());
                if (match <= threshold) {
                    *(selIter->rawData()) = MAX_SELECTED;
                }
            }
        }
        while (hiter->nextPixel() && selIter->nextPixel());
        hiter->nextRow();
        selIter->nextRow();
    }

}

KisToolSelectSimilar::KisToolSelectSimilar(KoCanvasBase *canvas)
    : KisToolSelect(canvas,
                    KisCursor::load("tool_similar_selection_cursor.png", 6, 6),
                    i18n("Similar Color Selection"))
    , m_threshold(20)
{
}

void KisToolSelectSimilar::activate(const QSet<KoShape*> &shapes)
{
    KisToolSelect::activate(shapes);
    m_configGroup =  KSharedConfig::openConfig()->group(toolId());
}

void KisToolSelectSimilar::deactivate()
{
    m_referencePaintDevice = nullptr;
    m_referenceNodeList = nullptr;
    KisToolSelect::deactivate();
}

void KisToolSelectSimilar::beginPrimaryAction(KoPointerEvent *event)
{
    KisToolSelectBase::beginPrimaryAction(event);
    if (isMovingSelection()) {
        return;
    }

    KisPaintDeviceSP dev;

    if (!currentNode() ||
        !(dev = currentNode()->projection()) ||
        !selectionEditable()) {

        event->ignore();
        return;
    }

    QPointF pos = convertToPixelCoord(event);

    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_SAFE_ASSERT_RECOVER(kisCanvas) {
        QApplication::restoreOverrideCursor();
        return;
    };

    
    beginSelectInteraction();

    QApplication::setOverrideCursor(KisCursor::waitCursor());

    KisProcessingApplicator applicator(currentImage(),
                                       currentNode(),
                                       KisProcessingApplicator::NONE,
                                       KisImageSignalVector(),
                                       kundo2_i18n("Select Similar Color"));

    KisImageSP imageSP = currentImage();
    KisPaintDeviceSP sourceDevice;
    QRect areaToCheck;

    if (sampleLayersMode() == SampleCurrentLayer) {
        sourceDevice = m_referencePaintDevice = dev;
    } else if (sampleLayersMode() == SampleAllLayers) {
        sourceDevice = m_referencePaintDevice = currentImage()->projection();
    } else if (sampleLayersMode() == SampleColorLabeledLayers) {
        KisImageSP refImage = KisMergeLabeledLayersCommand::createRefImage(image(), "Similar Colors Selection Tool Reference Image");
        if (!m_referenceNodeList) {
            m_referencePaintDevice = KisMergeLabeledLayersCommand::createRefPaintDevice(image(), "Similar Colors Selection Tool Reference Result Paint Device");
            m_referenceNodeList.reset(new KisMergeLabeledLayersCommand::ReferenceNodeInfoList);
        }
        KisPaintDeviceSP newReferencePaintDevice = KisMergeLabeledLayersCommand::createRefPaintDevice(image(), "Similar Colors Selection Tool Reference Result Paint Device");
        KisMergeLabeledLayersCommand::ReferenceNodeInfoListSP newReferenceNodeList(new KisMergeLabeledLayersCommand::ReferenceNodeInfoList);
        applicator.applyCommand(
            new KisMergeLabeledLayersCommand(
                refImage,
                m_referenceNodeList,
                newReferenceNodeList,
                m_referencePaintDevice,
                newReferencePaintDevice,
                image()->root(),
                colorLabelsSelected(),
                KisMergeLabeledLayersCommand::GroupSelectionPolicy_SelectIfColorLabeled
            ),
            KisStrokeJobData::SEQUENTIAL,
            KisStrokeJobData::EXCLUSIVE
        );
        sourceDevice = m_referencePaintDevice = newReferencePaintDevice;
        m_referenceNodeList = newReferenceNodeList;
    }

    if (sampleLayersMode() == SampleColorLabeledLayers) {
        // source device is not ready to get a pixel out of it, let's assume image bounds
        areaToCheck = imageSP->bounds();
    } else {
        KoColor pixelColor;
        sourceDevice->pixel(pos.x(), pos.y(), &pixelColor);
        if (sourceDevice->colorSpace()->difference(
                pixelColor.data(),
                sourceDevice->defaultPixel().data())
            <= m_threshold) {
            areaToCheck = imageSP->bounds() | sourceDevice->exactBounds();
        } else {
            areaToCheck = sourceDevice->exactBounds();
        }
    }


    // XXX we should make this configurable: "allow to select transparent"
    // if (opacity > OPACITY_TRANSPARENT)
    KisPixelSelectionSP tmpSel =
        new KisPixelSelection(new KisSelectionDefaultBounds(dev));

    const int threshold = m_threshold;
    const bool antiAlias = antiAliasSelection();
    const int grow = growSelection();
    const int feather = featherSelection();
    // new stroke

    QSharedPointer<KoColor> color = QSharedPointer<KoColor>(new KoColor(sourceDevice->colorSpace()));
    QSharedPointer<bool> isDefaultPixel = QSharedPointer<bool>(new bool(true));

    KUndo2Command *cmdPickColor = new KisCommandUtils::LambdaCommand(
        [pos, sourceDevice, color, isDefaultPixel, threshold]() mutable
        -> KUndo2Command * {
            sourceDevice->pixel(pos.x(), pos.y(), color.data());
            *isDefaultPixel.data() = sourceDevice->colorSpace()->difference(
                                         color.data()->data(),
                                         sourceDevice->defaultPixel().data())
                < threshold;

            return 0;
        });

    applicator.applyCommand(cmdPickColor, KisStrokeJobData::SEQUENTIAL);

    QVector<QRect> patches = KritaUtils::splitRectIntoPatches(areaToCheck, KritaUtils::optimalPatchSize());

    for (int i = 0; i < patches.count(); i++) {
        QSharedPointer<QRect> patch = QSharedPointer<QRect>(new QRect(patches[i]));
        KUndo2Command *patchCmd = new KisCommandUtils::LambdaCommand(
            [threshold,
             tmpSel,
             sourceDevice,
             patch,
             color,
             isDefaultPixel]() mutable -> KUndo2Command * {
                QRect patchRect = *patch.data();
                QRect finalRect = patchRect;
                if (!isDefaultPixel) {
                    finalRect =
                        patchRect.intersected(sourceDevice->exactBounds());
                }
                if (!finalRect.isEmpty()) {
                    selectByColor(sourceDevice,
                                  tmpSel,
                                  color->data(),
                                  threshold,
                                  patchRect);
                }
                return 0;
            });

        applicator.applyCommand(patchCmd, KisStrokeJobData::CONCURRENT);
    }



    /*
     * Division of out-of-the-image-bounds areas
     * into different commands
     *
     * i---i------------------i
     * |   |       top        |
     * | l |--------------i---|
     * | e |              |   |
     * | f |              | r |
     * | t |    image     | i |
     * |   |              | g |
     * |   |              | h |
     * |___|______________| t |
     * |      bottom      |   |
     * |__________________|___|
     */

    if (sampleLayersMode() == SampleColorLabeledLayers) {
        QRect imageRect = image()->bounds();

        KUndo2Command *topCmd = new KisCommandUtils::LambdaCommand(
            [threshold,
             tmpSel,
             sourceDevice,
             color,
             imageRect,
             isDefaultPixel]() mutable -> KUndo2Command * {
                QRect contentRect = sourceDevice->exactBounds();
                QRect patchRect = QRect(
                    QPoint(0, contentRect.top()),
                    QPoint(qMax(contentRect.right(), imageRect.right()), 0));
                QRect finalRect = patchRect;
                if (!*isDefaultPixel) {
                    finalRect = patchRect.intersected(contentRect);
                }
                if (!finalRect.isEmpty()) {
                    selectByColor(sourceDevice,
                                  tmpSel,
                                  color->data(),
                                  threshold,
                                  finalRect);
                }
                return 0;
            });

        KUndo2Command *rightCmd = new KisCommandUtils::LambdaCommand(
            [threshold,
             tmpSel,
             sourceDevice,
             color,
             imageRect,
             isDefaultPixel]() mutable -> KUndo2Command * {
                QRect contentRect = sourceDevice->exactBounds();
                QRect patchRect = QRect(
                    QPoint(imageRect.width(), 0),
                    QPoint(contentRect.right(),
                           qMax(contentRect.bottom(), imageRect.bottom())));
                QRect finalRect = patchRect;
                if (!*isDefaultPixel) {
                    finalRect = patchRect.intersected(contentRect);
                }
                if (!finalRect.isEmpty()) {
                    selectByColor(sourceDevice,
                                  tmpSel,
                                  color->data(),
                                  threshold,
                                  finalRect);
                }
                return 0;
            });

        KUndo2Command *bottomCmd = new KisCommandUtils::LambdaCommand(
            [threshold,
             tmpSel,
             sourceDevice,
             color,
             imageRect,
             isDefaultPixel]() mutable -> KUndo2Command * {
                QRect contentRect = sourceDevice->exactBounds();
                QRect patchRect =
                    QRect(QPoint(qMin(contentRect.left(), imageRect.left()),
                                 imageRect.bottom()),
                          QPoint(imageRect.right(), contentRect.bottom()));
                QRect finalRect = patchRect;
                if (!*isDefaultPixel) {
                    finalRect = patchRect.intersected(contentRect);
                }
                if (!finalRect.isEmpty()) {
                    selectByColor(sourceDevice,
                                  tmpSel,
                                  color->data(),
                                  threshold,
                                  finalRect);
                }
                return 0;
            });

        KUndo2Command *leftCmd = new KisCommandUtils::LambdaCommand(
            [threshold,
             tmpSel,
             sourceDevice,
             color,
             imageRect,
             isDefaultPixel]() mutable -> KUndo2Command * {
                QRect contentRect = sourceDevice->exactBounds();
                QRect patchRect =
                    QRect(QPoint(contentRect.left(),
                                 qMin(contentRect.top(), imageRect.top())),
                          QPoint(0, imageRect.bottom()));
                QRect finalRect = patchRect;
                if (!*isDefaultPixel) {
                    finalRect = patchRect.intersected(contentRect);
                }
                if (!finalRect.isEmpty()) {
                    selectByColor(sourceDevice,
                                  tmpSel,
                                  color->data(),
                                  threshold,
                                  finalRect);
                }
                return 0;
            });

        applicator.applyCommand(topCmd, KisStrokeJobData::CONCURRENT);
        applicator.applyCommand(rightCmd, KisStrokeJobData::CONCURRENT);
        applicator.applyCommand(bottomCmd, KisStrokeJobData::CONCURRENT);
        applicator.applyCommand(leftCmd, KisStrokeJobData::CONCURRENT);

    }

    KUndo2Command *cmdAdjustSelection = new KisCommandUtils::LambdaCommand(
        [tmpSel, antiAlias, grow, feather]() mutable -> KUndo2Command * {
            if (grow > 0) {
                KisGrowSelectionFilter biggy(grow, grow);
                biggy.process(
                    tmpSel,
                    tmpSel->selectedRect().adjusted(-grow, -grow, grow, grow));
            } else if (grow < 0) {
                KisShrinkSelectionFilter tiny(-grow, -grow, false);
                tiny.process(tmpSel, tmpSel->selectedRect());
            }
            // Since the feathering already smooths the selection, the antiAlias
            // is not applied if we must feather
            if (feather > 0) {
                KisFeatherSelectionFilter feathery(feather);
                feathery.process(tmpSel,
                                 tmpSel->selectedRect().adjusted(-feather,
                                                                 -feather,
                                                                 feather,
                                                                 feather));
            } else if (antiAlias) {
                KisAntiAliasSelectionFilter antiAliasFilter;
                antiAliasFilter.process(tmpSel, tmpSel->selectedRect());
            }

            return 0;
        });
    applicator.applyCommand(cmdAdjustSelection, KisStrokeJobData::SEQUENTIAL);

    KUndo2Command *cmdInvalidateCache = new KisCommandUtils::LambdaCommand(
        [tmpSel]() mutable -> KUndo2Command * {
            tmpSel->invalidateOutlineCache();
            return 0;
        });
    applicator.applyCommand(cmdInvalidateCache, KisStrokeJobData::SEQUENTIAL);

    KisSelectionToolHelper helper(kisCanvas, kundo2_i18n("Select Similar Color"));
    helper.selectPixelSelection(applicator, tmpSel, selectionAction());

    applicator.end();
    QApplication::restoreOverrideCursor();

}

void KisToolSelectSimilar::endPrimaryAction(KoPointerEvent *event)
{
    if (isMovingSelection()) {
        KisToolSelectBase::endPrimaryAction(event);
        return;
    }

    endSelectInteraction();
}

void KisToolSelectSimilar::slotSetThreshold(int threshold)
{
    m_threshold = threshold;
    m_configGroup.writeEntry("threshold", threshold);
}

QWidget* KisToolSelectSimilar::createOptionWidget()
{
    KisToolSelectBase::createOptionWidget();
    KisSelectionOptions *selectionWidget = selectionOptionWidget();

    KisSliderSpinBox *sliderThreshold = new KisSliderSpinBox;
    sliderThreshold->setPrefix(i18nc(
        "The 'threshold' spinbox prefix in similar selection tool options",
        "Threshold: "));
    sliderThreshold->setRange(1, 200);
    sliderThreshold->setSingleStep(20);
    sliderThreshold->setToolTip(
        i18n("Set how far the selection should extend in terms of color "
             "similarity"));

    KisOptionCollectionWidgetWithHeader *sectionSelectionExtent =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'selection extent' section label in similar selection "
                  "tool options",
                  "Selection extent"));
    sectionSelectionExtent->appendWidget("sliderThreshold", sliderThreshold);
    selectionWidget->insertWidget(2,
                                  "sectionSelectionExtent",
                                  sectionSelectionExtent);

    // load setting from config
    if (m_configGroup.hasKey("threshold")) {
        m_threshold = m_configGroup.readEntry("threshold", 20);
    } else {
        m_threshold = m_configGroup.readEntry("fuzziness", 20);
    }
    sliderThreshold->setValue(m_threshold);

    connect(sliderThreshold,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(slotSetThreshold(int)));

    return selectionWidget;
}

void KisToolSelectSimilar::resetCursorStyle()
{
    if (selectionAction() == SELECTION_ADD) {
        useCursor(KisCursor::load("tool_similar_selection_cursor_add.png", 6, 6));
    } else if (selectionAction() == SELECTION_SUBTRACT) {
        useCursor(KisCursor::load("tool_similar_selection_cursor_sub.png", 6, 6));
    } else if (selectionAction() == SELECTION_INTERSECT) {
        useCursor(KisCursor::load("tool_similar_selection_cursor_inter.png", 6, 6));
    } else if (selectionAction() == SELECTION_SYMMETRICDIFFERENCE) {
        useCursor(KisCursor::load("tool_similar_selection_cursor_symdiff.png", 6, 6));
    } else {
        KisToolSelect::resetCursorStyle();
    }
}
