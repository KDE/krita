/*
 *  kis_tool_select_contiguous - part of Krayon^WKrita
 *
 *  SPDX-FileCopyrightText: 1999 Michael Koch <koch@kde.org>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2012 José Luis Vergara <pentalis@gmail.com>
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_select_contiguous.h"
#include <QPainter>
#include <QLayout>
#include <QLabel>
#include <QApplication>
#include <QCheckBox>
#include <QVBoxLayout>

#include <kis_debug.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

#include "KoPointerEvent.h"
#include "KoViewConverter.h"

#include "kis_cursor.h"
#include "kis_selection_manager.h"
#include "kis_image.h"
#include "canvas/kis_canvas2.h"
#include "kis_layer.h"
#include "kis_selection_options.h"
#include "kis_paint_device.h"
#include "kis_fill_painter.h"
#include "kis_pixel_selection.h"
#include "kis_selection_tool_helper.h"
#include "kis_slider_spin_box.h"
#include "tiles3/kis_hline_iterator.h"
#include "commands_new/KisMergeLabeledLayersCommand.h"
#include "kis_image.h"
#include "kis_undo_stores.h"
#include "kis_resources_snapshot.h"
#include "kis_processing_applicator.h"
#include <processing/fill_processing_visitor.h>

#include "kis_command_utils.h"


KisToolSelectContiguous::KisToolSelectContiguous(KoCanvasBase *canvas)
    : KisToolSelect(canvas,
                    KisCursor::load("tool_contiguous_selection_cursor.png", 6, 6),
                    i18n("Contiguous Area Selection")),
    m_threshold(8),
    m_opacitySpread(100),
    m_sizemod(0),
    m_feather(0)
{
    setObjectName("tool_select_contiguous");
}

KisToolSelectContiguous::~KisToolSelectContiguous()
{
}

void KisToolSelectContiguous::activate(const QSet<KoShape*> &shapes)
{
    KisToolSelect::activate(shapes);
    m_configGroup =  KSharedConfig::openConfig()->group(toolId());
}

void KisToolSelectContiguous::beginPrimaryAction(KoPointerEvent *event)
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

    beginSelectInteraction();

    QApplication::setOverrideCursor(KisCursor::waitCursor());

    // -------------------------------

    KisProcessingApplicator applicator(currentImage(), currentNode(),
                                       KisProcessingApplicator::NONE,
                                       KisImageSignalVector(),
                                       kundo2_i18n("Select Contiguous Area"));

    QPoint pos = convertToImagePixelCoordFloored(event);
    QRect rc = currentImage()->bounds();


    KisImageSP image = currentImage();
    KisPaintDeviceSP sourceDevice;
    if (sampleLayersMode() == SampleAllLayers) {
        sourceDevice = image->projection();
    } else if (sampleLayersMode() == SampleColorLabeledLayers) {
        KisImageSP refImage = KisMergeLabeledLayersCommand::createRefImage(image, "Contiguous Selection Tool Reference Image");
        sourceDevice = KisMergeLabeledLayersCommand::createRefPaintDevice(
                    image, "Contiguous Selection Tool Reference Result Paint Device");

        KisMergeLabeledLayersCommand* command = new KisMergeLabeledLayersCommand(refImage, sourceDevice,
                                                                                 image->root(), colorLabelsSelected(),
                                                                                 KisMergeLabeledLayersCommand::GroupSelectionPolicy_SelectIfColorLabeled);
        applicator.applyCommand(command,
                                KisStrokeJobData::SEQUENTIAL,
                                KisStrokeJobData::EXCLUSIVE);

    } else { // Sample Current Layer
        sourceDevice = dev;
    }

    KisPixelSelectionSP selection = KisPixelSelectionSP(new KisPixelSelection(new KisSelectionDefaultBounds(dev)));

    int threshold = m_threshold;
    int opacitySpread = m_opacitySpread;
    int feather = m_feather;
    int sizemod = m_sizemod;
    bool useSelectionAsBoundary = m_useSelectionAsBoundary;
    bool antiAlias = antiAliasSelection();

    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_SAFE_ASSERT_RECOVER(kisCanvas) {
        applicator.cancel();
        QApplication::restoreOverrideCursor();
        return;
    };

    KisPixelSelectionSP existingSelection;
    if (kisCanvas->imageView() && kisCanvas->imageView()->selection())
    {
        existingSelection = kisCanvas->imageView()->selection()->pixelSelection();
    }

    KUndo2Command* cmd = new KisCommandUtils::LambdaCommand(
                [dev, rc, threshold, opacitySpread, antiAlias, feather, sizemod, useSelectionAsBoundary,
                selection, pos, sourceDevice, existingSelection] () mutable -> KUndo2Command* {

                    KisFillPainter fillpainter(dev);
                    fillpainter.setHeight(rc.height());
                    fillpainter.setWidth(rc.width());
                    fillpainter.setFillThreshold(threshold);
                    fillpainter.setOpacitySpread(opacitySpread);
                    fillpainter.setAntiAlias(antiAlias);
                    fillpainter.setFeather(feather);
                    fillpainter.setSizemod(sizemod);
                    fillpainter.setUseCompositioning(true);

                    useSelectionAsBoundary &=
                        existingSelection &&
                        !existingSelection->isEmpty() &&
                        existingSelection->pixel(pos).opacityU8() != OPACITY_TRANSPARENT_U8;

                    fillpainter.setUseSelectionAsBoundary(useSelectionAsBoundary);
                    fillpainter.createFloodSelection(selection, pos.x(), pos.y(), sourceDevice, existingSelection);

                    selection->invalidateOutlineCache();

                    return 0;
    });
    applicator.applyCommand(cmd, KisStrokeJobData::SEQUENTIAL);



    KisSelectionToolHelper helper(kisCanvas, kundo2_i18n("Select Contiguous Area"));

    helper.selectPixelSelection(applicator, selection, selectionAction());

    applicator.end();
    QApplication::restoreOverrideCursor();

}

void KisToolSelectContiguous::endPrimaryAction(KoPointerEvent *event)
{
    if (isMovingSelection()) {
        KisToolSelectBase::endPrimaryAction(event);
        return;
    }

    endSelectInteraction();
}

void KisToolSelectContiguous::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

void KisToolSelectContiguous::slotSetThreshold(int threshold)
{
    m_threshold = threshold;
    m_configGroup.writeEntry("threshold", threshold);
}

void KisToolSelectContiguous::slotSetOpacitySpread(int opacitySpread)
{
    m_opacitySpread = opacitySpread;
    m_configGroup.writeEntry("opacitySpread", opacitySpread);
}

void KisToolSelectContiguous::slotSetSizemod(int sizemod)
{
    m_sizemod = sizemod;
    m_configGroup.writeEntry("sizemod", sizemod);
}

void KisToolSelectContiguous::slotSetFeather(int feather)
{
    m_feather = feather;
    m_configGroup.writeEntry("feather", feather);
}

void KisToolSelectContiguous::slotSetUseSelectionAsBoundary(bool useSelectionAsBoundary)
{
    m_useSelectionAsBoundary = useSelectionAsBoundary;
    m_configGroup.writeEntry("useSelectionAsBoundary", useSelectionAsBoundary);
}

QWidget* KisToolSelectContiguous::createOptionWidget()
{
    KisToolSelectBase::createOptionWidget();
    KisSelectionOptions *selectionWidget = selectionOptionWidget();

    m_widgetHelper.setConfigGroupForExactTool(toolId());

    // Create widgets
    KisSliderSpinBox *sliderThreshold = new KisSliderSpinBox;
    sliderThreshold->setPrefix(i18nc("The 'threshold' spinbox prefix in contiguous selection tool options", "Threshold: "));
    sliderThreshold->setRange(1, 100);
    KisSliderSpinBox *sliderSpread = new KisSliderSpinBox;
    sliderSpread->setPrefix(i18nc("The 'spread' spinbox prefix in contiguous selection tool options", "Spread: "));
    sliderSpread->setSuffix(i18n("%"));
    sliderSpread->setRange(0, 100);
    QCheckBox *checkBoxSelectionAsBoundary =
        new QCheckBox(
            i18nc("The 'use selection as boundary' checkbox in contiguous selection tool to use selection borders as boundary when filling",
                  "Use selection as boundary")
        );
    checkBoxSelectionAsBoundary->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    KisSliderSpinBox *sliderGrow = new KisSliderSpinBox;
    sliderGrow->setPrefix(i18nc("The 'grow/shrink' spinbox prefix in contiguous selection tool options", "Grow: "));
    sliderGrow->setRange(-40, 40);
    sliderGrow->setSuffix(i18n(" px"));
    KisSliderSpinBox *sliderFeather = new KisSliderSpinBox;
    sliderFeather->setPrefix(i18nc("The 'feather' spinbox prefix in contiguous selection tool options", "Feather: "));
    sliderFeather->setRange(0, 40);
    sliderFeather->setSuffix(i18n(" px"));

    // Set the tooltips
    sliderThreshold->setToolTip(i18n("Set how far the selection should extend from the selected pixel in terms of color similarity"));
    sliderSpread->setToolTip(i18n("Set how far the fully opaque portion of the selection should extend."
                                  "\n0% will make the selection opaque only where the pixels are exactly equal to the selected pixel."
                                  "\n100% will make all the selection opaque up to its boundary."));
    checkBoxSelectionAsBoundary->setToolTip(i18n("Set if the contour of the current selection should be treated as a boundary when obtaining the new one"));

    sliderGrow->setToolTip(i18n("Grow (positive values) or shrink (negative values) the selection by the set amount"));
    sliderFeather->setToolTip(i18n("Blur the selection by the set amount"));
    
    // Construct the option widget
    KisOptionCollectionWidgetWithHeader *sectionSelectionExtent =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'selection extent' section label in contiguous selection tool options", "Selection extent")
        );
    sectionSelectionExtent->appendWidget("sliderThreshold", sliderThreshold);
    sectionSelectionExtent->appendWidget("sliderSpread", sliderSpread);
    sectionSelectionExtent->appendWidget("checkBoxSelectionAsBoundary", checkBoxSelectionAsBoundary);
    selectionWidget->insertWidget(2, "sectionSelectionExtent", sectionSelectionExtent);

    KisOptionCollectionWidgetWithHeader *sectionAdjustments =
        selectionWidget->widgetAs<KisOptionCollectionWidgetWithHeader*>("sectionAdjustments");
    sectionAdjustments->appendWidget("sliderGrow", sliderGrow);
    sectionAdjustments->appendWidget("sliderFeather", sliderFeather);
    
    // Load configuration settings into tool options
    if (m_configGroup.hasKey("threshold")) {
        m_threshold = m_configGroup.readEntry("threshold", 8);
    } else {
        m_threshold = m_configGroup.readEntry("fuzziness", 8);
    }
    m_opacitySpread = m_configGroup.readEntry("opacitySpread", 100);
    m_sizemod = m_configGroup.readEntry("sizemod", 0);
    m_feather = m_configGroup.readEntry("feather", 0);
    m_useSelectionAsBoundary = m_configGroup.readEntry("useSelectionAsBoundary", false);

    sliderThreshold->setValue(m_threshold);
    sliderSpread->setValue(m_opacitySpread);
    checkBoxSelectionAsBoundary->setChecked(m_useSelectionAsBoundary);
    sliderGrow->setValue(m_sizemod);
    sliderFeather->setValue(m_feather);

    // Make connections
    connect(sliderThreshold, SIGNAL(valueChanged(int)), this, SLOT(slotSetThreshold(int)));
    connect(sliderSpread, SIGNAL(valueChanged(int)), this, SLOT(slotSetOpacitySpread(int)));
    connect(sliderGrow, SIGNAL(valueChanged(int)), this, SLOT(slotSetSizemod(int)));
    connect(sliderFeather, SIGNAL(valueChanged(int)), this, SLOT(slotSetFeather(int)));
    connect(checkBoxSelectionAsBoundary, SIGNAL(toggled(bool)), this, SLOT(slotSetUseSelectionAsBoundary(bool)));

    return selectionWidget;
}

void KisToolSelectContiguous::resetCursorStyle()
{
    if (selectionAction() == SELECTION_ADD) {
        useCursor(KisCursor::load("tool_contiguous_selection_cursor_add.png", 6, 6));
    } else if (selectionAction() == SELECTION_SUBTRACT) {
        useCursor(KisCursor::load("tool_contiguous_selection_cursor_sub.png", 6, 6));
    } else if (selectionAction() == SELECTION_INTERSECT) {
        useCursor(KisCursor::load("tool_contiguous_selection_cursor_inter.png", 6, 6));
    } else if (selectionAction() == SELECTION_SYMMETRICDIFFERENCE) {
        useCursor(KisCursor::load("tool_contiguous_selection_cursor_symdiff.png", 6, 6));
    } else {
        KisToolSelect::resetCursorStyle();
    }
}
