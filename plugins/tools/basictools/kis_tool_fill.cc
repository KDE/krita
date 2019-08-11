/*
*  kis_tool_fill.cc - part of Krayon
*
*  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
*  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
*  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
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

#include "kis_tool_fill.h"

#include <kis_debug.h>
#include <klocalizedstring.h>

#include <QLabel>
#include <QLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QVector>
#include <QRect>
#include <QColor>

#include <ksharedconfig.h>

#include <KoCanvasBase.h>
#include <KoPointerEvent.h>

#include <kis_layer.h>
#include <kis_painter.h>
#include <resources/KoPattern.h>
#include <kis_fill_painter.h>
#include <kis_selection.h>

#include <KisViewManager.h>
#include <canvas/kis_canvas2.h>
#include <widgets/kis_cmb_composite.h>
#include <widgets/kis_slider_spin_box.h>
#include <kis_cursor.h>
#include "kis_resources_snapshot.h"

#include <processing/fill_processing_visitor.h>
#include <kis_processing_applicator.h>


KisToolFill::KisToolFill(KoCanvasBase * canvas)
        : KisToolPaint(canvas, KisCursor::load("tool_fill_cursor.png", 6, 6))
{
    setObjectName("tool_fill");
    m_feather = 0;
    m_sizemod = 0;
    m_threshold = 80;
    m_usePattern = false;
    m_unmerged = false;
    m_fillOnlySelection = false;
}

KisToolFill::~KisToolFill()
{
}

void KisToolFill::resetCursorStyle()
{
    KisToolPaint::resetCursorStyle();

    overrideCursorIfNotEditable();
}

void KisToolFill::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    KisToolPaint::activate(toolActivation, shapes);
    m_configGroup =  KSharedConfig::openConfig()->group(toolId());
}


void KisToolFill::beginPrimaryAction(KoPointerEvent *event)
{
    // cannot use fill tool on non-painting layers.
    // this logic triggers with multiple layer types like vector layer, clone layer, file layer, group layer
    if ( currentNode().isNull() || currentNode()->inherits("KisShapeLayer") || nodePaintAbility()!=NodePaintAbility::PAINT ) {
        KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
        kiscanvas->viewManager()->
                showFloatingMessage(
                    i18n("You cannot use this tool with the selected layer type"),
                    QIcon(), 2000, KisFloatingMessage::Medium, Qt::AlignCenter);
        event->ignore();
        return;
    }


    if (!nodeEditable()) {
        event->ignore();
        return;
    }

    setMode(KisTool::PAINT_MODE);

    m_startPos = convertToImagePixelCoordFloored(event);
}

void KisToolFill::endPrimaryAction(KoPointerEvent *event)
{
    Q_UNUSED(event);
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    setMode(KisTool::HOVER_MODE);

    if (!currentNode() ||
        (!image()->wrapAroundModePermitted() &&
         !image()->bounds().contains(m_startPos))) {

        return;
    }

    bool useFastMode = m_useFastMode->isChecked();

    KisProcessingApplicator applicator(currentImage(), currentNode(),
                                       KisProcessingApplicator::SUPPORTS_WRAPAROUND_MODE,
                                       KisImageSignalVector() << ModifiedSignal,
                                       kundo2_i18n("Flood Fill"));

    KisResourcesSnapshotSP resources =
        new KisResourcesSnapshot(image(), currentNode(), this->canvas()->resourceManager());

    KisProcessingVisitorSP visitor =
        new FillProcessingVisitor(m_startPos,
                                  resources->activeSelection(),
                                  resources,
                                  useFastMode,
                                  m_usePattern,
                                  m_fillOnlySelection,
                                  m_feather,
                                  m_sizemod,
                                  m_threshold,
                                  m_unmerged,
                                  false);

    applicator.applyVisitor(visitor,
                            KisStrokeJobData::SEQUENTIAL,
                            KisStrokeJobData::EXCLUSIVE);

    applicator.end();
}

QWidget* KisToolFill::createOptionWidget()
{
    QWidget *widget = KisToolPaint::createOptionWidget();
    widget->setObjectName(toolId() + " option widget");

    QLabel *lbl_fastMode = new QLabel(i18n("Fast mode: "), widget);
    m_useFastMode = new QCheckBox(QString(), widget);
    m_useFastMode->setToolTip(
        i18n("Fills area faster, but does not take composition "
             "mode into account. Selections and other extended "
             "features will also be disabled."));


    QLabel *lbl_threshold = new QLabel(i18n("Threshold: "), widget);
    m_slThreshold = new KisSliderSpinBox(widget);
    m_slThreshold->setObjectName("int_widget");
    m_slThreshold->setRange(1, 100);
    m_slThreshold->setPageStep(3);


    QLabel *lbl_sizemod = new QLabel(i18n("Grow selection: "), widget);
    m_sizemodWidget = new KisSliderSpinBox(widget);
    m_sizemodWidget->setObjectName("sizemod");
    m_sizemodWidget->setRange(-40, 40);
    m_sizemodWidget->setSingleStep(1);
    m_sizemodWidget->setSuffix(i18n(" px"));

    QLabel *lbl_feather = new QLabel(i18n("Feathering radius: "), widget);
    m_featherWidget = new KisSliderSpinBox(widget);
    m_featherWidget->setObjectName("feather");
    m_featherWidget->setRange(0, 40);
    m_featherWidget->setSingleStep(1);
    m_featherWidget->setSuffix(i18n(" px"));

    QLabel *lbl_usePattern = new QLabel(i18n("Use pattern:"), widget);
    m_checkUsePattern = new QCheckBox(QString(), widget);
    m_checkUsePattern->setToolTip(i18n("When checked do not use the foreground color, but the pattern selected to fill with"));

    QLabel *lbl_sampleMerged = new QLabel(i18n("Limit to current layer:"), widget);
    m_checkSampleMerged = new QCheckBox(QString(), widget);


    QLabel *lbl_fillSelection = new QLabel(i18n("Fill entire selection:"), widget);
    m_checkFillSelection = new QCheckBox(QString(), widget);
    m_checkFillSelection->setToolTip(i18n("When checked do not look at the current layer colors, but just fill all of the selected area"));

    connect (m_useFastMode       , SIGNAL(toggled(bool))    , this, SLOT(slotSetUseFastMode(bool)));
    connect (m_slThreshold       , SIGNAL(valueChanged(int)), this, SLOT(slotSetThreshold(int)));
    connect (m_sizemodWidget     , SIGNAL(valueChanged(int)), this, SLOT(slotSetSizemod(int)));
    connect (m_featherWidget     , SIGNAL(valueChanged(int)), this, SLOT(slotSetFeather(int)));
    connect (m_checkUsePattern   , SIGNAL(toggled(bool))    , this, SLOT(slotSetUsePattern(bool)));
    connect (m_checkSampleMerged , SIGNAL(toggled(bool))    , this, SLOT(slotSetSampleMerged(bool)));
    connect (m_checkFillSelection, SIGNAL(toggled(bool))    , this, SLOT(slotSetFillSelection(bool)));

    addOptionWidgetOption(m_useFastMode, lbl_fastMode);
    addOptionWidgetOption(m_slThreshold, lbl_threshold);
    addOptionWidgetOption(m_sizemodWidget      , lbl_sizemod);
    addOptionWidgetOption(m_featherWidget      , lbl_feather);

    addOptionWidgetOption(m_checkFillSelection, lbl_fillSelection);
    addOptionWidgetOption(m_checkSampleMerged, lbl_sampleMerged);
    addOptionWidgetOption(m_checkUsePattern, lbl_usePattern);

    updateGUI();

    widget->setFixedHeight(widget->sizeHint().height());



    // load configuration options
    m_useFastMode->setChecked(m_configGroup.readEntry("useFastMode", false));
    m_slThreshold->setValue(m_configGroup.readEntry("thresholdAmount", 80));
    m_sizemodWidget->setValue(m_configGroup.readEntry("growSelection", 0));

    m_featherWidget->setValue(m_configGroup.readEntry("featherAmount", 0));
    m_checkUsePattern->setChecked(m_configGroup.readEntry("usePattern", false));
    m_checkSampleMerged->setChecked(m_configGroup.readEntry("sampleMerged", false));
    m_checkFillSelection->setChecked(m_configGroup.readEntry("fillSelection", false));

    return widget;
}

void KisToolFill::updateGUI()
{
    bool useAdvancedMode = !m_useFastMode->isChecked();
    bool selectionOnly = m_checkFillSelection->isChecked();

    m_useFastMode->setEnabled(!selectionOnly);
    m_slThreshold->setEnabled(!selectionOnly);

    m_sizemodWidget->setEnabled(!selectionOnly && useAdvancedMode);
    m_featherWidget->setEnabled(!selectionOnly && useAdvancedMode);
    m_checkSampleMerged->setEnabled(!selectionOnly && useAdvancedMode);
    m_checkUsePattern->setEnabled(useAdvancedMode);
}

void KisToolFill::slotSetUseFastMode(bool value)
{
    updateGUI();
    m_configGroup.writeEntry("useFastMode", value);
}

void KisToolFill::slotSetThreshold(int threshold)
{
    m_threshold = threshold;
    m_configGroup.writeEntry("thresholdAmount", threshold);
}

void KisToolFill::slotSetUsePattern(bool state)
{
    m_usePattern = state;
    m_configGroup.writeEntry("usePattern", state);
}

void KisToolFill::slotSetSampleMerged(bool state)
{
    m_unmerged = state;
    m_configGroup.writeEntry("sampleMerged", state);
}

void KisToolFill::slotSetFillSelection(bool state)
{
    m_fillOnlySelection = state;
    m_configGroup.writeEntry("fillSelection", state);
    updateGUI();
}

void KisToolFill::slotSetSizemod(int sizemod)
{
    m_sizemod = sizemod;
    m_configGroup.writeEntry("growSelection", sizemod);
}

void KisToolFill::slotSetFeather(int feather)
{
    m_feather = feather;
    m_configGroup.writeEntry("featherAmount", feather);
}
