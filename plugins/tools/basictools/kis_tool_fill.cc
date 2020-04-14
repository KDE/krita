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
#include <QSharedPointer>

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
#include <kis_command_utils.h>
#include <functional>
#include <kis_group_layer.h>
#include <kis_layer_utils.h>

#include "KoCompositeOpRegistry.h"


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
    keysAtStart = event->modifiers();
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

    Qt::KeyboardModifiers fillOnlySelectionModifier = Qt::AltModifier; // Not sure where to keep it
    if (keysAtStart == fillOnlySelectionModifier) {
      m_fillOnlySelection = true;
    }
    keysAtStart = Qt::NoModifier; // libs/ui/tool/kis_tool_select_base.h cleans it up in endPrimaryAction so i do it too

    KisProcessingApplicator applicator(currentImage(), currentNode(),
                                       KisProcessingApplicator::SUPPORTS_WRAPAROUND_MODE,
                                       KisImageSignalVector() << ModifiedSignal,
                                       kundo2_i18n("Flood Fill"));

    KisResourcesSnapshotSP resources =
        new KisResourcesSnapshot(image(), currentNode(), this->canvas()->resourceManager());



    KisImageSP refImage = KisImageSP(new KisImage(new KisSurrogateUndoStore(), image()->width(), image()->height(), image()->colorSpace(),
                                      "Fill Tool Reference Image"));




    KisPaintDeviceSP refPaintDevice = KisPaintDeviceSP(new KisPaintDevice(image()->colorSpace(), "Fill Tool Reference Result Paint Device"));
    KisImageWSP currentImageWSP = currentImage();
    KisNodeSP currentRoot = currentImageWSP->root();

    KUndo2Command *cmd = new KisCommandUtils::LambdaCommand(
                [refImage, refPaintDevice, currentRoot] () mutable {

        fprintf(stderr, "refImage is %p\n", refImage.data());
        fprintf(stderr, "current root is %p\n", currentRoot.data());

        fprintf(stderr, "COMMAND IS BEING EXECUTED!!!\n");
        KUndo2Command* ptr = 0; // a dummy to ensure the compiler that the lambda has a proper type

        KisNodeSP root = currentRoot;
        KisNodeSP child = root->firstChild();

        fprintf(stderr, "Now: %s\n", (refImage->root().isNull() ? "(null)" : refImage->root()->name().toStdString().c_str()));


        QList<KisNodeSP> nodesList;
        KisLayerUtils::recursiveApplyNodes(currentRoot, [&nodesList, refImage] (KisNodeSP node) mutable {
            fprintf(stderr, "node\n");
            if (node->colorLabelIndex() == 2)
            {
                KisNodeSP copy = node->clone();

                if (copy.isNull()) {
                    fprintf(stderr, "(1)\n");
                    return;
                }

                if (node->inherits("KisLayer")) {
                    KisLayer* layerCopy = dynamic_cast<KisLayer*>(copy.data());
                    layerCopy->setChannelFlags(QBitArray());
                }

                copy->setCompositeOpId(COMPOSITE_OVER);

                bool success = refImage->addNode(copy, refImage->root());

                if (!success) {
                    fprintf(stderr, "(2)\n");
                    return;
                }

                fprintf(stderr, "adding to nodesList\n");

                nodesList << copy;
            }



        });

        /*
        while(child)
        {
            fprintf(stderr, "child: %s, %i\n", child->name().toStdString().c_str(), child->colorLabelIndex());
            if (child->colorLabelIndex() == 2)
            {
                fprintf(stderr, "child adding to the image: %s, %i\n", child->name().toStdString().c_str(), child->colorLabelIndex());

                fprintf(stderr, "child->paintDevice()->exactBounds(): %i %i\n",
                        child->paintDevice()->exactBounds().width(), child->paintDevice()->exactBounds().height());


                KisNodeSP copy = child->clone();
                nodesList << copy;

                fprintf(stderr, "copy->paintDevice()->exactBounds(): %i %i\n",
                        copy->paintDevice()->exactBounds().width(), copy->paintDevice()->exactBounds().height());



                if (copy.isNull()) {
                    fprintf(stderr, "COPY IS NULL! FINISHING...\n");
                    return ptr;
                }

                if (copy->inherits("KisLayer")) {
                    // KisLayerSP layerCopy = copy.dynamicCast<KisLayer>();
                    // KisLayerSP layerCopy = qSharedPointerDynamicCast(copy);
                    KisLayer* layerCopy = dynamic_cast<KisLayer*>(copy.data());
                    layerCopy->setChannelFlags(QBitArray());
                }

                copy->setCompositeOpId(COMPOSITE_OVER);

                bool success = refImage->addNode(copy, refImage->root());

                if (!success) {
                    fprintf(stderr, "NO SUCCESS! FINISHING...\n");
                    return ptr;
                }
            }
            child = child->nextSibling();
        }
        */

        nodesList = KisLayerUtils::sortAndFilterAnyMergableNodesSafe(nodesList, refImage);
        refImage->initialRefreshGraph();


        fprintf(stderr, "reference has %i children\n", refImage->root()->childCount());
        if (refImage->root()->childCount() == 0)
        {
            fprintf(stderr, "finishing because of no kids...\n");
            return ptr;
        }

        fprintf(stderr, "root->paintDevice()->exactBounds() in refImage: %i %i\n",
                refImage->root()->exactBounds().width(), refImage->root()->exactBounds().height());
        fprintf(stderr, "root->original()->exactBounds() in refImage: %i %i\n",
                refImage->root()->original()->exactBounds().width(), refImage->root()->original()->exactBounds().height());


        KisNodeSP childIt = refImage->root()->firstChild();
        while(!childIt.isNull()) {
            fprintf(stderr, "childIt->paintDevice()->exactBounds() in refImage: %i %i\n",
                    childIt->paintDevice()->exactBounds().width(), childIt->paintDevice()->exactBounds().height());
            childIt = childIt->nextSibling();
        }


        refImage->waitForDone();
        fprintf(stderr, "refImage->bounds() before: %i %i\n", refImage->bounds().width(), refImage->bounds().height());
        fprintf(stderr, "refImage->root()->exactBounds() before: %i %i\n", refImage->root()->exactBounds().width(), refImage->root()->exactBounds().height());
        fprintf(stderr, "refImage->projection()->exactBounds() before: %i %i\n", refImage->projection()->exactBounds().width(), refImage->projection()->exactBounds().height());



        refImage->mergeMultipleLayers(nodesList, 0);
        //refImage->flatten(refImage->root());


        refImage->waitForDone();
        fprintf(stderr, "refImage->bounds() after: %i %i\n", refImage->bounds().width(), refImage->bounds().height());
        fprintf(stderr, "refImage->root()->exactBounds() after: %i %i\n", refImage->root()->exactBounds().width(), refImage->root()->exactBounds().height());
        fprintf(stderr, "refImage->projection()->exactBounds() after: %i %i\n", refImage->projection()->exactBounds().width(), refImage->projection()->exactBounds().height());

        fprintf(stderr, "root->original()->exactBounds() in refImage: %i %i\n",
                refImage->root()->firstChild()->exactBounds().width(), refImage->root()->firstChild()->exactBounds().height());

        KisPainter::copyAreaOptimized(QPoint(), refImage->projection(), refPaintDevice, refImage->bounds());
        fprintf(stderr, "bounds of refPaintDevice after: %i %i\n", refPaintDevice->exactBounds().width(), refPaintDevice->exactBounds().height());
        //fprintf(stderr, "bounds of refImage projection after: %i %i\n", refImage->projection()->exactBounds().width(), refImage->projection()->exactBounds().height());



        return ptr;


        });

    applicator.applyCommand(cmd,
                                KisStrokeJobData::SEQUENTIAL);


    KisProcessingVisitorSP visitor =
        new FillProcessingVisitor(refPaintDevice,
                                  m_startPos,
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
    m_fillOnlySelection = m_checkFillSelection->isChecked();
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
