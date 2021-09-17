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
    m_fuzziness(20),
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

        KisMergeLabeledLayersCommand* command = new KisMergeLabeledLayersCommand(refImage, sourceDevice, image->root(), colorLabelsSelected());
        applicator.applyCommand(command,
                                KisStrokeJobData::SEQUENTIAL,
                                KisStrokeJobData::EXCLUSIVE);

    } else { // Sample Current Layer
        sourceDevice = dev;
    }

    KisPixelSelectionSP selection = KisPixelSelectionSP(new KisPixelSelection(new KisSelectionDefaultBounds(dev)));

    bool antiAlias = antiAliasSelection();

    int fuzziness = m_fuzziness;
    int feather = m_feather;
    int sizemod = m_sizemod;
    bool useSelectionAsBoundary = m_useSelectionAsBoundary;

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
                [dev, rc, fuzziness, feather, sizemod, useSelectionAsBoundary,
                selection, pos, sourceDevice, antiAlias, existingSelection] () mutable -> KUndo2Command* {

                    KisFillPainter fillpainter(dev);
                    fillpainter.setHeight(rc.height());
                    fillpainter.setWidth(rc.width());
                    fillpainter.setFillThreshold(fuzziness);
                    fillpainter.setFeather(feather);
                    fillpainter.setSizemod(sizemod);

                    useSelectionAsBoundary &=
                        existingSelection &&
                        !existingSelection->isEmpty() &&
                        existingSelection->pixel(pos).opacityU8() != OPACITY_TRANSPARENT_U8;

                    fillpainter.setUseSelectionAsBoundary(useSelectionAsBoundary);
                    fillpainter.createFloodSelection(selection, pos.x(), pos.y(), sourceDevice, existingSelection);

                    // If we're not antialiasing, threshold the entire selection
                    if (!antiAlias) {
                        const QRect r = selection->selectedExactRect();
                        KisSequentialIterator it (selection, r);
                        while(it.nextPixel()) {
                            if (*it.rawData() > 0) {
                                *it.rawData() = OPACITY_OPAQUE_U8;
                            }
                        }
                    }

                    selection->invalidateOutlineCache();

                    return 0;
    });
    applicator.applyCommand(cmd, KisStrokeJobData::SEQUENTIAL);



    KisSelectionToolHelper helper(kisCanvas, kundo2_i18n("Select Contiguous Area"));

    helper.selectPixelSelection(applicator, selection, selectionAction());

    applicator.end();
    QApplication::restoreOverrideCursor();

}

void KisToolSelectContiguous::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

void KisToolSelectContiguous::slotSetFuzziness(int fuzziness)
{
    m_fuzziness = fuzziness;
    m_configGroup.writeEntry("fuzziness", fuzziness);
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

    QVBoxLayout * l = dynamic_cast<QVBoxLayout*>(selectionWidget->layout());
    Q_ASSERT(l);
    if (l) {

        QGridLayout * gridLayout = new QGridLayout();
        l->insertLayout(1, gridLayout);

        QLabel * lbl = new QLabel(i18n("Fuzziness: "), selectionWidget);
        gridLayout->addWidget(lbl, 0, 0, 1, 1);

        KisSliderSpinBox *input = new KisSliderSpinBox(selectionWidget);
        Q_CHECK_PTR(input);
        input->setObjectName("fuzziness");
        input->setRange(1, 100);
        input->setSingleStep(1);
        input->setExponentRatio(2);
        gridLayout->addWidget(input, 0, 1, 1, 1);

        lbl = new QLabel(i18n("Grow/shrink selection: "), selectionWidget);
        gridLayout->addWidget(lbl, 1, 0, 1, 1);

        KisSliderSpinBox *sizemod = new KisSliderSpinBox(selectionWidget);
        Q_CHECK_PTR(sizemod);
        sizemod->setObjectName("sizemod"); //grow/shrink selection
        sizemod->setRange(-40, 40);
        sizemod->setSingleStep(1);
        gridLayout->addWidget(sizemod, 1, 1, 1, 1);

        lbl = new QLabel(i18n("Feathering radius: "), selectionWidget);
        gridLayout->addWidget(lbl, 2, 0, 1, 1);

        KisSliderSpinBox *feather = new KisSliderSpinBox(selectionWidget);
        Q_CHECK_PTR(feather);
        feather->setObjectName("feathering");
        feather->setRange(0, 40);
        feather->setSingleStep(1);
        gridLayout->addWidget(feather, 2, 1, 1, 1);

        lbl = new QLabel(i18n("Use selection as boundary: "), selectionWidget);
        gridLayout->addWidget(lbl, 3, 0, 1, 1);

        QCheckBox *useSelectionAsBoundary = new QCheckBox(selectionWidget);
        Q_CHECK_PTR(useSelectionAsBoundary);
        gridLayout->addWidget(useSelectionAsBoundary, 3, 1, 1, 1);

        connect (input  , SIGNAL(valueChanged(int)), this, SLOT(slotSetFuzziness(int)));
        connect (sizemod, SIGNAL(valueChanged(int)), this, SLOT(slotSetSizemod(int)));
        connect (feather, SIGNAL(valueChanged(int)), this, SLOT(slotSetFeather(int)));
        connect (useSelectionAsBoundary, SIGNAL(toggled(bool)), this, SLOT(slotSetUseSelectionAsBoundary(bool)));


        selectionWidget->attachToImage(image(), dynamic_cast<KisCanvas2*>(canvas()));
        m_widgetHelper.setConfigGroupForExactTool(toolId());

        // load configuration settings into tool options
        input->setValue(m_configGroup.readEntry("fuzziness", 8)); // fuzziness
        sizemod->setValue( m_configGroup.readEntry("sizemod", 0)); //grow/shrink
        sizemod->setSuffix(i18n(" px"));

        feather->setValue(m_configGroup.readEntry("feather", 0));
        feather->setSuffix(i18n(" px"));

        useSelectionAsBoundary->setChecked(m_configGroup.readEntry("useSelectionAsBoundary", false));

    }
    return selectionWidget;
}

void KisToolSelectContiguous::resetCursorStyle()
{
    if (selectionAction() == SELECTION_ADD) {
        useCursor(KisCursor::load("tool_contiguous_selection_cursor_add.png", 6, 6));
    } else if (selectionAction() == SELECTION_SUBTRACT) {
        useCursor(KisCursor::load("tool_contiguous_selection_cursor_sub.png", 6, 6));
    } else {
        KisToolSelect::resetCursorStyle();
    }
}
