/*
 *  SPDX-FileCopyrightText: 1999 Matthias Elter <me@kde.org>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_select_similar.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <ksharedconfig.h>

#include <KoColorSpace.h>

#include <kis_cursor.h>
#include <KoPointerEvent.h>
#include <kis_selection_options.h>
#include <kis_paint_device.h>
#include "kis_canvas2.h"
#include <kis_pixel_selection.h>
#include "kis_selection_tool_helper.h"
#include "kis_slider_spin_box.h"
#include "kis_iterator_ng.h"
#include "kis_image.h"
#include "commands_new/KisMergeLabeledLayersCommand.h"
#include "kis_command_utils.h"
#include "krita_utils.h"

void selectByColor(KisPaintDeviceSP dev, KisPixelSelectionSP selection, const quint8 *c, int fuzziness, const QRect & rc)
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
            if (fuzziness == 1) {
                if (wantedOpacity == 0 && cs->opacityU8(hiter->rawDataConst()) == 0) {
                    *(selIter->rawData()) = MAX_SELECTED;
                }
                else if (memcmp(c, hiter->rawDataConst(), cs->pixelSize()) == 0) {
                    *(selIter->rawData()) = MAX_SELECTED;
                }
            }
            else {
                quint8 match = cs->difference(c, hiter->rawDataConst());
                if (match <= fuzziness) {
                    *(selIter->rawData()) = MAX_SELECTED;
                }
            }
        }
        while (hiter->nextPixel() && selIter->nextPixel());
        hiter->nextRow();
        selIter->nextRow();
    }

}


KisToolSelectSimilar::KisToolSelectSimilar(KoCanvasBase * canvas)
    : KisToolSelect(canvas,
                    KisCursor::load("tool_similar_selection_cursor.png", 6, 6),
                    i18n("Similar Color Selection")),
      m_fuzziness(20)
{
}

void KisToolSelectSimilar::activate(const QSet<KoShape*> &shapes)
{
    KisToolSelect::activate(shapes);
    if (selectionOptionWidget()) {
        // similar color selection tool doesn't use antialiasing option for now
        // hence explicit disabling it
        selectionOptionWidget()->disableAntiAliasSelectionOption();
    }
    m_configGroup =  KSharedConfig::openConfig()->group(toolId());
}

void KisToolSelectSimilar::beginPrimaryAction(KoPointerEvent *event)
{
    KisToolSelectBase::beginPrimaryAction(event);
    KisPaintDeviceSP dev;

    if (!currentNode() ||
        !(dev = currentNode()->projection()) ||
        !currentNode()->visible() ||
        !selectionEditable()) {

        event->ignore();
        return;
    }

    if (KisToolSelect::selectionDidMove()) {
        return;
    }

    QPointF pos = convertToPixelCoord(event);

    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_SAFE_ASSERT_RECOVER(kisCanvas) {
        QApplication::restoreOverrideCursor();
        return;
    };

    QApplication::setOverrideCursor(KisCursor::waitCursor());

    KisProcessingApplicator applicator(currentImage(), currentNode(),
                                       KisProcessingApplicator::NONE,
                                       KisImageSignalVector(),
                                       kundo2_i18n("Select Contiguous Area"));


    KisImageSP imageSP = currentImage();
    KisPaintDeviceSP sourceDevice;
    QRect areaToCheck;

    if (sampleLayersMode() == SampleAllLayers) {
        sourceDevice = imageSP->projection();
    } else if (sampleLayersMode() == SampleColorLabeledLayers) {
        KisImageSP refImage = KisMergeLabeledLayersCommand::createRefImage(imageSP, "Similar Colors Selection Tool Reference Image");
        sourceDevice = KisMergeLabeledLayersCommand::createRefPaintDevice(
                    imageSP, "Similar Colors Selection Tool Reference Result Paint Device");

        KisMergeLabeledLayersCommand* command = new KisMergeLabeledLayersCommand(refImage, sourceDevice, imageSP->root(), colorLabelsSelected());
        applicator.applyCommand(command,
                                KisStrokeJobData::SEQUENTIAL,
                                KisStrokeJobData::EXCLUSIVE);

    } else { // Sample Current Layer
        sourceDevice = dev;
    }

    if (sampleLayersMode() == SampleColorLabeledLayers) {
        // source device is not ready to get a pixel out of it, let's assume image bounds
        areaToCheck = imageSP->bounds();
    } else {
        KoColor pixelColor;
        sourceDevice->pixel(pos.x(), pos.y(), &pixelColor);
        if (sourceDevice->colorSpace()->difference(pixelColor.data(), sourceDevice->defaultPixel().data()) <= m_fuzziness) {
            areaToCheck = imageSP->bounds() | sourceDevice->exactBounds();
        } else {
            areaToCheck = sourceDevice->exactBounds();
        }
    }


    // XXX we should make this configurable: "allow to select transparent"
    // if (opacity > OPACITY_TRANSPARENT)
    KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());


    int fuzziness = m_fuzziness;
    // new stroke

    QSharedPointer<KoColor> color = QSharedPointer<KoColor>(new KoColor(sourceDevice->colorSpace()));
    QSharedPointer<bool> isDefaultPixel = QSharedPointer<bool>(new bool(true));

    KUndo2Command* cmdPickColor = new KisCommandUtils::LambdaCommand(
                [pos, sourceDevice, color, isDefaultPixel, fuzziness] () mutable -> KUndo2Command* {

                    sourceDevice->pixel(pos.x(), pos.y(), color.data());
                    *isDefaultPixel.data() = sourceDevice->colorSpace()->difference(color.data()->data(), sourceDevice->defaultPixel().data()) < fuzziness;

                    return 0;
    });

    applicator.applyCommand(cmdPickColor, KisStrokeJobData::SEQUENTIAL);

    QVector<QRect> patches = KritaUtils::splitRectIntoPatches(areaToCheck, KritaUtils::optimalPatchSize());

    for (int i = 0; i < patches.count(); i++) {
        QSharedPointer<QRect> patch = QSharedPointer<QRect>(new QRect(patches[i]));
        KUndo2Command* patchCmd = new KisCommandUtils::LambdaCommand(
                    [fuzziness, tmpSel, sourceDevice, patch, color, isDefaultPixel] () mutable -> KUndo2Command* {

                        QRect patchRect = *patch.data();
                        QRect finalRect = patchRect;
                        if (!isDefaultPixel) {
                            finalRect = patchRect.intersected(sourceDevice->exactBounds());
                        }
                        if (!finalRect.isEmpty()) {
                            selectByColor(sourceDevice, tmpSel, color->data(), fuzziness, patchRect);
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

        KUndo2Command* topCmd = new KisCommandUtils::LambdaCommand(
                    [fuzziness, tmpSel, sourceDevice, color, imageRect, isDefaultPixel] () mutable -> KUndo2Command* {

                        QRect contentRect = sourceDevice->exactBounds();
                        QRect patchRect = QRect(QPoint(0, contentRect.top()), QPoint(qMax(contentRect.right(), imageRect.right()), 0));
                        QRect finalRect = patchRect;
                        if (!*isDefaultPixel) {
                            finalRect = patchRect.intersected(contentRect);
                        }
                        if (!finalRect.isEmpty()) {
                            selectByColor(sourceDevice, tmpSel, color->data(), fuzziness, finalRect);
                        }
                        return 0;
        });

        KUndo2Command* rightCmd = new KisCommandUtils::LambdaCommand(
                    [fuzziness, tmpSel, sourceDevice, color, imageRect, isDefaultPixel] () mutable -> KUndo2Command* {

                        QRect contentRect = sourceDevice->exactBounds();
                        QRect patchRect = QRect(QPoint(imageRect.width(), 0), QPoint(contentRect.right(), qMax(contentRect.bottom(), imageRect.bottom())));
                        QRect finalRect = patchRect;
                        if (!*isDefaultPixel) {
                            finalRect = patchRect.intersected(contentRect);
                        }
                        if (!finalRect.isEmpty()) {
                            selectByColor(sourceDevice, tmpSel, color->data(), fuzziness, finalRect);
                        }
                        return 0;
        });

        KUndo2Command* bottomCmd = new KisCommandUtils::LambdaCommand(
                    [fuzziness, tmpSel, sourceDevice, color, imageRect, isDefaultPixel] () mutable -> KUndo2Command* {

                        QRect contentRect = sourceDevice->exactBounds();
                        QRect patchRect = QRect(QPoint(qMin(contentRect.left(), imageRect.left()), imageRect.bottom()), QPoint(imageRect.right(), contentRect.bottom()));
                        QRect finalRect = patchRect;
                        if (!*isDefaultPixel) {
                            finalRect = patchRect.intersected(contentRect);
                        }
                        if (!finalRect.isEmpty()) {
                            selectByColor(sourceDevice, tmpSel, color->data(), fuzziness, finalRect);
                        }
                        return 0;
        });

        KUndo2Command* leftCmd = new KisCommandUtils::LambdaCommand(
                    [fuzziness, tmpSel, sourceDevice, color, imageRect, isDefaultPixel] () mutable -> KUndo2Command* {

                        QRect contentRect = sourceDevice->exactBounds();
                        QRect patchRect = QRect(QPoint(contentRect.left(), qMin(contentRect.top(), imageRect.top())), QPoint(0, imageRect.bottom()));
                        QRect finalRect = patchRect;
                        if (!*isDefaultPixel) {
                            finalRect = patchRect.intersected(contentRect);
                        }
                        if (!finalRect.isEmpty()) {
                            selectByColor(sourceDevice, tmpSel, color->data(), fuzziness, finalRect);
                        }
                        return 0;
        });


        applicator.applyCommand(topCmd, KisStrokeJobData::CONCURRENT);
        applicator.applyCommand(rightCmd, KisStrokeJobData::CONCURRENT);
        applicator.applyCommand(bottomCmd, KisStrokeJobData::CONCURRENT);
        applicator.applyCommand(leftCmd, KisStrokeJobData::CONCURRENT);

    }


    KUndo2Command* cmdInvalidateCache = new KisCommandUtils::LambdaCommand(
                [tmpSel] () mutable -> KUndo2Command* {

                    tmpSel->invalidateOutlineCache();
                    return 0;
    });
    applicator.applyCommand(cmdInvalidateCache, KisStrokeJobData::SEQUENTIAL);

    KisSelectionToolHelper helper(kisCanvas, kundo2_i18n("Select Similar Color"));
    helper.selectPixelSelection(applicator, tmpSel, selectionAction());

    applicator.end();
    QApplication::restoreOverrideCursor();

}

void KisToolSelectSimilar::slotSetFuzziness(int fuzziness)
{
    m_fuzziness = fuzziness;
    m_configGroup.writeEntry("fuzziness", fuzziness);
}

QWidget* KisToolSelectSimilar::createOptionWidget()
{
    KisToolSelectBase::createOptionWidget();
    KisSelectionOptions *selectionWidget = selectionOptionWidget();
    // similar color selection tool doesn't use antialiasing option for now
    // hence explicit disabling it
    selectionWidget->disableAntiAliasSelectionOption();

    QHBoxLayout* fl = new QHBoxLayout();
    QLabel * lbl = new QLabel(i18n("Fuzziness: "), selectionWidget);
    fl->addWidget(lbl);

    KisSliderSpinBox* input = new KisSliderSpinBox(selectionWidget);
    input->setObjectName("fuzziness");
    input->setRange(1, 200);
    input->setSingleStep(10);
    fl->addWidget(input);
    connect(input, SIGNAL(valueChanged(int)), this, SLOT(slotSetFuzziness(int)));


    selectionWidget->attachToImage(image(), dynamic_cast<KisCanvas2*>(canvas()));
    m_widgetHelper.setConfigGroupForExactTool(toolId());

    QVBoxLayout* l = dynamic_cast<QVBoxLayout*>(selectionWidget->layout());
    Q_ASSERT(l);
    l->insertLayout(1, fl);

    // load setting from config
    input->setValue(m_configGroup.readEntry("fuzziness", 20));
    return selectionWidget;
}

void KisToolSelectSimilar::resetCursorStyle()
{
    if (selectionAction() == SELECTION_ADD) {
        useCursor(KisCursor::load("tool_similar_selection_cursor_add.png", 6, 6));
    } else if (selectionAction() == SELECTION_SUBTRACT) {
        useCursor(KisCursor::load("tool_similar_selection_cursor_sub.png", 6, 6));
    } else {
        KisToolSelect::resetCursorStyle();
    }
}
