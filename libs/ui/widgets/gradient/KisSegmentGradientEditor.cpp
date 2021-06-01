/*
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2004 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QPainter>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QAction>
#include <QDialog>

#include <KoColorSpace.h>
#include <resources/KoSegmentGradient.h>

#include "kis_debug.h"

#include "KisSegmentGradientSlider.h"

#include <KoCanvasResourcesIds.h>
#include <KoCanvasResourcesInterface.h>

#include <kis_icon_utils.h>
#include <kis_signals_blocker.h>

#include "KisSegmentGradientEditor.h"

KisSegmentGradientEditor::KisSegmentGradientEditor(QWidget *parent)
    : QWidget(parent)
    , m_gradient(nullptr)
    , m_canvasResourcesInterface(nullptr)
{
    setupUi(this);

    QAction *selectPreviousHandleAction = new QAction(KisIconUtils::loadIcon("arrow-left"),
        i18nc("Action to select previous handle in the segment gradient editor", "Select previous handle"), this);
    selectPreviousHandleAction->setToolTip(selectPreviousHandleAction->text());
    connect(selectPreviousHandleAction, SIGNAL(triggered()), gradientSlider, SLOT(selectPreviousHandle()));

    QAction *selectNextHandleAction = new QAction(KisIconUtils::loadIcon("arrow-right"),
        i18nc("Action to select next handle in the segment gradient editor", "Select next handle"), this);
    selectNextHandleAction->setToolTip(selectNextHandleAction->text());
    connect(selectNextHandleAction, SIGNAL(triggered()), gradientSlider, SLOT(selectNextHandle()));

    m_editHandleAction = new QAction(KisIconUtils::loadIcon("document-edit"), i18nc("Button to edit the selected handle in the segment gradient editor", "Edit handle"), this);
    m_editHandleAction->setToolTip(m_editHandleAction->text());
    connect(m_editHandleAction, SIGNAL(triggered()), this, SLOT(editSelectedHandle()));

    m_deleteSegmentAction = new QAction(KisIconUtils::loadIcon("edit-delete"),
        i18nc("Action to delete the selected segment in the segment gradient editor", "Delete segment"), this);
    m_deleteSegmentAction->setToolTip(m_deleteSegmentAction->text());
    connect(m_deleteSegmentAction, SIGNAL(triggered()), gradientSlider, SLOT(collapseSelectedSegment()));

    m_flipSegmentAction = new QAction(KisIconUtils::loadIcon("transform_icons_mirror_x"),
        i18nc("Action to flip the selected segment in the segment gradient editor", "Flip segment"), this);
    m_flipSegmentAction->setToolTip(m_flipSegmentAction->text());
    connect(m_flipSegmentAction, SIGNAL(triggered()), gradientSlider, SLOT(mirrorSelectedSegment()));

    m_splitSegmentAction = new QAction(KisIconUtils::loadIcon("cut-item"),
        i18nc("Action to split the selected segment in the segment gradient editor", "Split segment"), this);
    m_splitSegmentAction->setToolTip(m_splitSegmentAction->text());
    connect(m_splitSegmentAction, SIGNAL(triggered()), gradientSlider, SLOT(splitSelectedSegment()));

    m_duplicateSegmentAction = new QAction(KisIconUtils::loadIcon("duplicateitem"),
        i18nc("Action to duplicate the selected segment in the segment gradient editor", "Duplicate segment"), this);
    m_duplicateSegmentAction->setToolTip(m_duplicateSegmentAction->text());
    connect(m_duplicateSegmentAction, SIGNAL(triggered()), gradientSlider, SLOT(duplicateSelectedSegment()));

    m_deleteStopAction = new QAction(KisIconUtils::loadIcon("edit-delete"),
        i18nc("Action to delete the selected stop in the segment gradient editor", "Delete stop"), this);
    m_deleteStopAction->setToolTip(m_deleteStopAction->text());
    connect(m_deleteStopAction, SIGNAL(triggered()), gradientSlider, SLOT(deleteSelectedHandle()));

    m_centerStopAction = new QAction(KisIconUtils::loadIcon("object-align-horizontal-center-calligra"),
        i18nc("Action to center the selected stop in the segment gradient editor", "Center stop"), this);
    m_centerStopAction->setToolTip(m_centerStopAction->text());
    connect(m_centerStopAction, SIGNAL(triggered()), gradientSlider, SLOT(centerSelectedHandle()));

    m_centerMidPointAction = new QAction(KisIconUtils::loadIcon("object-align-horizontal-center-calligra"),
        i18nc("Action to center the selected mid point in the segment gradient editor", "Center middle point"), this);
    m_centerMidPointAction->setToolTip(m_centerMidPointAction->text());
    connect(m_centerMidPointAction, SIGNAL(triggered()), gradientSlider, SLOT(centerSelectedHandle()));

    QAction *flipGradientAction = new QAction(KisIconUtils::loadIcon("transform_icons_mirror_x"),
        i18nc("Button to flip the gradient in the segment gradient editor", "Flip gradient"), this);
    flipGradientAction->setToolTip(flipGradientAction->text());
    connect(flipGradientAction, SIGNAL(triggered()), gradientSlider, SLOT(flipGradient()));

    QAction *distributeSegmentsEvenlyAction = new QAction(KisIconUtils::loadIcon("distribute-horizontal"),
        i18nc("Button to evenly distribute the segments in the segment gradient editor", "Distribute segments evenly"), this);
    distributeSegmentsEvenlyAction->setToolTip(distributeSegmentsEvenlyAction->text());
    connect(distributeSegmentsEvenlyAction, SIGNAL(triggered()), gradientSlider, SLOT(distributeStopsEvenly()));

    selectPreviousHandleButton->setAutoRaise(true);
    selectPreviousHandleButton->setDefaultAction(selectPreviousHandleAction);
    
    selectNextHandleButton->setAutoRaise(true);
    selectNextHandleButton->setDefaultAction(selectNextHandleAction);

    deleteSegmentButton->setAutoRaise(true);
    deleteSegmentButton->setDefaultAction(m_deleteSegmentAction);

    flipSegmentButton->setAutoRaise(true);
    flipSegmentButton->setDefaultAction(m_flipSegmentAction);

    splitSegmentButton->setAutoRaise(true);
    splitSegmentButton->setDefaultAction(m_splitSegmentAction);

    duplicateSegmentButton->setAutoRaise(true);
    duplicateSegmentButton->setDefaultAction(m_duplicateSegmentAction);

    deleteStopButton->setAutoRaise(true);
    deleteStopButton->setDefaultAction(m_deleteStopAction);

    centerStopButton->setAutoRaise(true);
    centerStopButton->setDefaultAction(m_centerStopAction);

    centerMidPointButton->setAutoRaise(true);
    centerMidPointButton->setDefaultAction(m_centerMidPointAction);

    flipGradientButton->setAutoRaise(true);
    flipGradientButton->setDefaultAction(flipGradientAction);

    distributeSegmentsEvenlyButton->setAutoRaise(true);
    distributeSegmentsEvenlyButton->setDefaultAction(distributeSegmentsEvenlyAction);

    compactModeSelectPreviousHandleButton->setAutoRaise(true);
    compactModeSelectPreviousHandleButton->setDefaultAction(selectPreviousHandleAction);
    
    compactModeSelectNextHandleButton->setAutoRaise(true);
    compactModeSelectNextHandleButton->setDefaultAction(selectNextHandleAction);

    compactModeMiscOptionsButton->setPopupMode(QToolButton::InstantPopup);
    compactModeMiscOptionsButton->setAutoRaise(true);
    compactModeMiscOptionsButton->setIcon(KisIconUtils::loadIcon("view-choose"));
    compactModeMiscOptionsButton->setStyleSheet("QToolButton::menu-indicator { image: none; }");
    QAction *separator = new QAction;
    separator->setSeparator(true);
    compactModeMiscOptionsButton->addAction(m_editHandleAction);
    compactModeMiscOptionsButton->addAction(m_deleteSegmentAction);
    compactModeMiscOptionsButton->addAction(m_flipSegmentAction);
    compactModeMiscOptionsButton->addAction(m_splitSegmentAction);
    compactModeMiscOptionsButton->addAction(m_duplicateSegmentAction);
    compactModeMiscOptionsButton->addAction(m_deleteStopAction);
    compactModeMiscOptionsButton->addAction(m_centerStopAction);
    compactModeMiscOptionsButton->addAction(m_centerMidPointAction);
    compactModeMiscOptionsButton->addAction(separator);
    compactModeMiscOptionsButton->addAction(flipGradientAction);
    compactModeMiscOptionsButton->addAction(distributeSegmentsEvenlyAction);

    stopLeftEditor->setUsePositionSlider(false);
    stopRightEditor->setUsePositionSlider(false);
    constrainStopButton->setKeepAspectRatio(false);
    constrainStopButton->setToolTip(i18nc("Button to link both end colors of a stop handle in the segment gradient editor", "Link colors"));
    stopPositionSlider->setRange(0, 100, 2);
    stopPositionSlider->setPrefix(i18n("Position: "));
    stopPositionSlider->setSuffix(i18n("%"));
    midPointPositionSlider->setRange(0, 100, 2);
    midPointPositionSlider->setPrefix(i18n("Position: "));
    midPointPositionSlider->setSuffix(i18n("%"));

    setCompactMode(false);
    setGradient(0);
}

KisSegmentGradientEditor::KisSegmentGradientEditor(KoSegmentGradientSP gradient, QWidget *parent, const char* name, const QString& caption, KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    : KisSegmentGradientEditor(parent)
{
    m_canvasResourcesInterface = canvasResourcesInterface;
    setObjectName(name);
    setWindowTitle(caption);
    setGradient(gradient);
}

void KisSegmentGradientEditor::setCompactMode(bool value)
{
    lblName->setVisible(!value);
    nameedit->setVisible(!value);
    buttonsContainer->setVisible(!value);
    handleEditorContainer->setVisible(!value);
    compactModeButtonsContainer->setVisible(value);
}

void KisSegmentGradientEditor::setGradient(KoSegmentGradientSP gradient)
{
    m_gradient = gradient;
    setEnabled(m_gradient);

    if (m_gradient) {
        nameedit->setText(m_gradient->name());
        gradientSlider->setGradientResource(m_gradient);
    }

    emit sigGradientChanged();
}

void KisSegmentGradientEditor::setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    m_canvasResourcesInterface = canvasResourcesInterface;
}

KoCanvasResourcesInterfaceSP KisSegmentGradientEditor::canvasResourcesInterface() const
{
    return m_canvasResourcesInterface;
}

void KisSegmentGradientEditor::on_gradientSlider_selectedHandleChanged()
{
    KisSegmentGradientSlider::Handle handle = gradientSlider->selectedHandle();

    if (handle.type == KisSegmentGradientSlider::HandleType_Segment) {
        KoGradientSegment *segment = m_gradient->segments()[handle.index];

        KisSignalsBlocker blocker(
            segmentLeftEditor, segmentRightEditor,
            segmentInterpolationTypeComboBox, segmentColorInterpolationTypeComboBox
        );

        selectedHandleLabel->setText(i18nc("Text that indicates the selected segment in the segment gradient editor", "Segment #%1", handle.index + 1));
        
        segmentLeftEditor->setColorType(KisGradientWidgetsUtils::segmentEndPointTypeToColorType(segment->startType()));
        segmentLeftEditor->setTransparent(segment->startType() == FOREGROUND_TRANSPARENT_ENDPOINT ||
                                          segment->startType() == BACKGROUND_TRANSPARENT_ENDPOINT);
        segmentLeftEditor->setColor(segment->startColor());
        segmentLeftEditor->setOpacity(segment->startColor().opacityF() * 100.0);
        segmentLeftEditor->setPosition(segment->startOffset() * 100.0);
        segmentLeftEditor->setPositionSliderEnabled(handle.index > 0);

        segmentRightEditor->setColorType(KisGradientWidgetsUtils::segmentEndPointTypeToColorType(segment->endType()));
        segmentRightEditor->setTransparent(segment->endType() == FOREGROUND_TRANSPARENT_ENDPOINT ||
                                           segment->endType() == BACKGROUND_TRANSPARENT_ENDPOINT);
        segmentRightEditor->setColor(segment->endColor());
        segmentRightEditor->setOpacity(segment->endColor().opacityF() * 100.0);
        segmentRightEditor->setPosition(segment->endOffset() * 100.0);
        segmentRightEditor->setPositionSliderEnabled(handle.index < m_gradient->segments().size() - 1);

        segmentInterpolationTypeComboBox->setCurrentIndex(segment->interpolation());
        segmentColorInterpolationTypeComboBox->setCurrentIndex(segment->colorInterpolation());

        handleEditorContainer->setCurrentIndex(0);

        m_deleteSegmentAction->setEnabled(m_gradient->segments().size() > 1);

    } else if (handle.type == KisSegmentGradientSlider::HandleType_Stop) {
        KoGradientSegment *previousSegment = handle.index == 0 ? nullptr : m_gradient->segments()[handle.index - 1];
        KoGradientSegment *nextSegment = handle.index == m_gradient->segments().size() ? nullptr : m_gradient->segments()[handle.index];

        KisSignalsBlocker blocker(stopLeftEditor, stopRightEditor, constrainStopButton, stopPositionSlider);

        selectedHandleLabel->setText(i18nc("Text that indicates the selected stop in the segment gradient editor", "Stop #%1", handle.index + 1));
        
        if (previousSegment) {
            stopLeftEditor->setColorType(KisGradientWidgetsUtils::segmentEndPointTypeToColorType(previousSegment->endType()));
            stopLeftEditor->setTransparent(previousSegment->endType() == FOREGROUND_TRANSPARENT_ENDPOINT ||
                                           previousSegment->endType() == BACKGROUND_TRANSPARENT_ENDPOINT);
            stopLeftEditor->setColor(previousSegment->endColor());
            stopLeftEditor->setOpacity(previousSegment->endColor().opacityF() * 100.0);
        }
        stopLeftEditor->setEnabled(previousSegment);

        if (nextSegment) {
            stopRightEditor->setColorType(KisGradientWidgetsUtils::segmentEndPointTypeToColorType(nextSegment->startType()));
            stopRightEditor->setTransparent(nextSegment->startType() == FOREGROUND_TRANSPARENT_ENDPOINT ||
                                            nextSegment->startType() == BACKGROUND_TRANSPARENT_ENDPOINT);
            stopRightEditor->setColor(nextSegment->startColor());
            stopRightEditor->setOpacity(nextSegment->startColor().opacityF() * 100.0);
        }        
        stopRightEditor->setEnabled(nextSegment);

        if (previousSegment && nextSegment) {
            constrainStopButton->setKeepAspectRatio(
                previousSegment->endType() == nextSegment->startType() &&
                previousSegment->endColor() == nextSegment->startColor()
            );
        }
        constrainStopButton->setEnabled(previousSegment && nextSegment);
        if (previousSegment) {
            stopPositionSlider->setValue(previousSegment->endOffset() * 100.0);
        } else if (nextSegment) {
            stopPositionSlider->setValue(nextSegment->startOffset() * 100.0);
        }
        stopPositionSlider->setEnabled(previousSegment && nextSegment);

        handleEditorContainer->setCurrentIndex(1);

        m_deleteStopAction->setEnabled(previousSegment && nextSegment);
        m_centerStopAction->setEnabled(previousSegment && nextSegment);

    } else if (handle.type == KisSegmentGradientSlider::HandleType_MidPoint) {
        KoGradientSegment *segment = m_gradient->segments()[handle.index];

        KisSignalsBlocker blocker(midPointPositionSlider);

        selectedHandleLabel->setText(i18nc("Text that indicates the selected mid point in the segment gradient editor", "Mid-Point #%1", handle.index + 1));

        midPointPositionSlider->setValue(
            (segment->middleOffset() - segment->startOffset()) / (segment->endOffset() - segment->startOffset()) * 100.0
        );

        handleEditorContainer->setCurrentIndex(2);

    } else {
        selectedHandleLabel->setText(i18nc("Text that indicates no handle is selected in the stop gradient editor", "No handle selected"));
        handleEditorContainer->setCurrentIndex(3);
    }

    m_editHandleAction->setEnabled(handle.type != KisSegmentGradientSlider::HandleType_None);

    m_deleteSegmentAction->setVisible(handle.type == KisSegmentGradientSlider::HandleType_Segment);
    m_flipSegmentAction->setVisible(handle.type == KisSegmentGradientSlider::HandleType_Segment);
    m_splitSegmentAction->setVisible(handle.type == KisSegmentGradientSlider::HandleType_Segment);
    m_duplicateSegmentAction->setVisible(handle.type == KisSegmentGradientSlider::HandleType_Segment);
    segmentButtonsContainer->setVisible(handle.type == KisSegmentGradientSlider::HandleType_Segment);

    m_deleteStopAction->setVisible(handle.type == KisSegmentGradientSlider::HandleType_Stop);
    m_centerStopAction->setVisible(handle.type == KisSegmentGradientSlider::HandleType_Stop);
    stopButtonsContainer->setVisible(handle.type == KisSegmentGradientSlider::HandleType_Stop);

    m_centerMidPointAction->setVisible(handle.type == KisSegmentGradientSlider::HandleType_MidPoint);
    midPointButtonsContainer->setVisible(handle.type == KisSegmentGradientSlider::HandleType_MidPoint);

    emit sigGradientChanged();
}

void KisSegmentGradientEditor::on_segmentLeftEditor_positionChanged(double position)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_Segment) {
        return;
    } 
    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index];
    KisSegmentGradientSlider::Handle stopHandle{KisSegmentGradientSlider::HandleType_Stop, gradientSlider->selectedHandle().index};
    {
        KisSignalsBlocker blocker(gradientSlider, segmentLeftEditor);
        gradientSlider->moveHandle(stopHandle, position / 100.0 - segment->startOffset());
        // Set the clamped value
        segmentLeftEditor->setPosition(segment->startOffset() * 100.0);
    }
    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::on_segmentLeftEditor_colorTypeChanged(KisGradientWidgetsUtils::ColorType type)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_Segment) {
        return;
    } 

    KoGradientSegmentEndpointType endPointType = KisGradientWidgetsUtils::colorTypeToSegmentEndPointType(type, segmentLeftEditor->transparent());
    KoColor color;
    const qreal opacity = segmentLeftEditor->transparent() ? 0.0 : 1.0;
    const KoColorSpace* colorSpace = m_gradient->colorSpace();

    if (endPointType == FOREGROUND_ENDPOINT || endPointType == FOREGROUND_TRANSPARENT_ENDPOINT) {
        if (m_canvasResourcesInterface) {
            color = m_canvasResourcesInterface->resource(KoCanvasResource::ForegroundColor).value<KoColor>().convertedTo(colorSpace);
        } else {
            color = KoColor(segmentLeftEditor->color(), colorSpace);
        }
    } else if (endPointType == BACKGROUND_ENDPOINT || endPointType == BACKGROUND_TRANSPARENT_ENDPOINT) {
        if (m_canvasResourcesInterface) {
            color = m_canvasResourcesInterface->resource(KoCanvasResource::BackgroundColor).value<KoColor>().convertedTo(colorSpace);
        } else {
            color = KoColor(segmentLeftEditor->color(), colorSpace);
        }
    } else {
        color = KoColor(segmentLeftEditor->color(), colorSpace);
    }

    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index];
    segment->setStartType(endPointType);
    color.setOpacity(opacity);
    segment->setStartColor(color);

    segmentLeftEditor->setColor(color);
    segmentLeftEditor->setOpacity(opacity * 100.0);

    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::on_segmentLeftEditor_transparentToggled(bool checked)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_Segment) {
        return;
    } 
    const qreal opacity = checked ? 0.0 : 1.0;
    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index];
    segment->setStartType(KisGradientWidgetsUtils::colorTypeToSegmentEndPointType(segmentLeftEditor->colorType(), checked));
    KoColor color = segment->startColor();
    color.setOpacity(opacity);
    segment->setStartColor(color);
    segmentLeftEditor->setOpacity(opacity * 100.0);
    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::on_segmentLeftEditor_colorChanged(KoColor color)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_Segment) {
        return;
    }
    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index];
    KoColor c(color, segment->startColor().colorSpace());
    c.setOpacity(segmentLeftEditor->opacity() / 100.0);
    segment->setStartColor(c);
    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::on_segmentLeftEditor_opacityChanged(double opacity)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_Segment) {
        return;
    }
    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index];
    KoColor color = segment->startColor();
    color.setOpacity(opacity / 100.0);
    segment->setStartColor(color);
    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::on_segmentRightEditor_positionChanged(double position)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_Segment) {
        return;
    } 
    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index];
    KisSegmentGradientSlider::Handle stopHandle{KisSegmentGradientSlider::HandleType_Stop, gradientSlider->selectedHandle().index + 1};
    {
        KisSignalsBlocker blocker(gradientSlider, segmentLeftEditor);
        gradientSlider->moveHandle(stopHandle, position / 100.0 - segment->endOffset());
        // Set the clamped value
        segmentLeftEditor->setPosition(segment->startOffset() * 100.0);
    }
    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::on_segmentRightEditor_colorTypeChanged(KisGradientWidgetsUtils::ColorType type)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_Segment) {
        return;
    } 

    KoGradientSegmentEndpointType endPointType = KisGradientWidgetsUtils::colorTypeToSegmentEndPointType(type, segmentRightEditor->transparent());
    KoColor color;
    const qreal opacity = segmentRightEditor->transparent() ? 0.0 : 1.0;
    const KoColorSpace* colorSpace = m_gradient->colorSpace();

    if (endPointType == FOREGROUND_ENDPOINT || endPointType == FOREGROUND_TRANSPARENT_ENDPOINT) {
        if (m_canvasResourcesInterface) {
            color = m_canvasResourcesInterface->resource(KoCanvasResource::ForegroundColor).value<KoColor>().convertedTo(colorSpace);
        } else {
            color = KoColor(segmentRightEditor->color(), colorSpace);
        }
    } else if (endPointType == BACKGROUND_ENDPOINT || endPointType == BACKGROUND_TRANSPARENT_ENDPOINT) {
        if (m_canvasResourcesInterface) {
            color = m_canvasResourcesInterface->resource(KoCanvasResource::BackgroundColor).value<KoColor>().convertedTo(colorSpace);
        } else {
            color = KoColor(segmentRightEditor->color(), colorSpace);
        }
    } else {
        color = KoColor(segmentRightEditor->color(), colorSpace);
    }

    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index];
    segment->setEndType(endPointType);
    color.setOpacity(opacity);
    segment->setEndColor(color);

    segmentRightEditor->setColor(color);
    segmentRightEditor->setOpacity(opacity * 100.0);

    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::on_segmentRightEditor_transparentToggled(bool checked)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_Segment) {
        return;
    } 
    const qreal opacity = checked ? 0.0 : 1.0;
    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index];
    segment->setEndType(KisGradientWidgetsUtils::colorTypeToSegmentEndPointType(segmentRightEditor->colorType(), checked));
    KoColor color = segment->endColor();
    color.setOpacity(opacity);
    segment->setEndColor(color);
    segmentRightEditor->setOpacity(opacity * 100.0);
    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::on_segmentRightEditor_colorChanged(KoColor color)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_Segment) {
        return;
    }
    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index];
    KoColor c(color, segment->endColor().colorSpace());
    c.setOpacity(segmentRightEditor->opacity() / 100.0);
    segment->setEndColor(c);
    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::on_segmentRightEditor_opacityChanged(double opacity)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_Segment) {
        return;
    }
    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index];
    KoColor color = segment->endColor();
    color.setOpacity(opacity / 100.0);
    segment->setEndColor(color);
    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::on_segmentInterpolationTypeComboBox_activated(int value)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_Segment) {
        return;
    }
    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index];
    segment->setInterpolation(value);
    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::on_segmentColorInterpolationTypeComboBox_activated(int value)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_Segment) {
        return;
    }
    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index];
    segment->setColorInterpolation(value);
    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::on_stopPositionSlider_valueChanged(double position)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_Stop) {
        return;
    } 
    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index];
    {
        KisSignalsBlocker blocker(gradientSlider, stopPositionSlider);
        gradientSlider->moveSelectedHandle(position / 100.0 - segment->startOffset());
        // Set the clamped value
        stopPositionSlider->setValue(segment->startOffset() * 100.0);
    }
    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::on_stopLeftEditor_colorTypeChanged(KisGradientWidgetsUtils::ColorType type)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_Stop) {
        return;
    } 

    KoGradientSegmentEndpointType endPointType = KisGradientWidgetsUtils::colorTypeToSegmentEndPointType(type, stopLeftEditor->transparent());
    KoColor color;
    const qreal opacity = stopLeftEditor->transparent() ? 0.0 : 1.0;
    const KoColorSpace* colorSpace = m_gradient->colorSpace();

    if (endPointType == FOREGROUND_ENDPOINT || endPointType == FOREGROUND_TRANSPARENT_ENDPOINT) {
        if (m_canvasResourcesInterface) {
            color = m_canvasResourcesInterface->resource(KoCanvasResource::ForegroundColor).value<KoColor>().convertedTo(colorSpace);
        } else {
            color = KoColor(stopLeftEditor->color(), colorSpace);
        }
    } else if (endPointType == BACKGROUND_ENDPOINT || endPointType == BACKGROUND_TRANSPARENT_ENDPOINT) {
        if (m_canvasResourcesInterface) {
            color = m_canvasResourcesInterface->resource(KoCanvasResource::BackgroundColor).value<KoColor>().convertedTo(colorSpace);
        } else {
            color = KoColor(stopLeftEditor->color(), colorSpace);
        }
    } else {
        color = KoColor(stopLeftEditor->color(), colorSpace);
    }

    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index - 1];
    segment->setEndType(endPointType);
    color.setOpacity(opacity);
    segment->setEndColor(color);

    stopLeftEditor->setColor(color);
    stopLeftEditor->setOpacity(opacity * 100.0);

    emit gradientSlider->updateRequested();
    emit sigGradientChanged();

    if (constrainStopButton->keepAspectRatio() &&
        gradientSlider->selectedHandle().index < m_gradient->segments().size()) {
        stopRightEditor->setColorType(stopLeftEditor->colorType());
    }
}

void KisSegmentGradientEditor::on_stopLeftEditor_transparentToggled(bool checked)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_Stop) {
        return;
    } 
    const qreal opacity = checked ? 0.0 : 1.0;
    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index - 1];
    segment->setEndType(KisGradientWidgetsUtils::colorTypeToSegmentEndPointType(stopLeftEditor->colorType(), checked));
    KoColor color = segment->endColor();
    color.setOpacity(opacity);
    segment->setEndColor(color);
    stopLeftEditor->setOpacity(opacity * 100.0);
    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
    if (constrainStopButton->keepAspectRatio() &&
        gradientSlider->selectedHandle().index < m_gradient->segments().size()) {
        stopRightEditor->setTransparent(stopLeftEditor->transparent());
    }
}

void KisSegmentGradientEditor::on_stopLeftEditor_colorChanged(KoColor color)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_Stop) {
        return;
    }
    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index - 1];
    KoColor c(color, segment->endColor().colorSpace());
    c.setOpacity(stopLeftEditor->opacity() / 100.0);
    segment->setEndColor(c);
    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
    if (constrainStopButton->keepAspectRatio() &&
        gradientSlider->selectedHandle().index < m_gradient->segments().size()) {
        stopRightEditor->setColor(stopLeftEditor->color());
    }
}

void KisSegmentGradientEditor::on_stopLeftEditor_opacityChanged(double opacity)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_Stop) {
        return;
    }
    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index - 1];
    KoColor color = segment->endColor();
    color.setOpacity(opacity / 100.0);
    segment->setEndColor(color);
    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
    if (constrainStopButton->keepAspectRatio() &&
        gradientSlider->selectedHandle().index < m_gradient->segments().size()) {
        stopRightEditor->setOpacity(stopLeftEditor->opacity());
    }
}

void KisSegmentGradientEditor::on_stopRightEditor_colorTypeChanged(KisGradientWidgetsUtils::ColorType type)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_Stop) {
        return;
    } 

    KoGradientSegmentEndpointType endPointType = KisGradientWidgetsUtils::colorTypeToSegmentEndPointType(type, stopRightEditor->transparent());
    KoColor color;
    const qreal opacity = stopRightEditor->transparent() ? 0.0 : 1.0;
    const KoColorSpace* colorSpace = m_gradient->colorSpace();

    if (endPointType == FOREGROUND_ENDPOINT || endPointType == FOREGROUND_TRANSPARENT_ENDPOINT) {
        if (m_canvasResourcesInterface) {
            color = m_canvasResourcesInterface->resource(KoCanvasResource::ForegroundColor).value<KoColor>().convertedTo(colorSpace);
        } else {
            color = KoColor(stopRightEditor->color(), colorSpace);
        }
    } else if (endPointType == BACKGROUND_ENDPOINT || endPointType == BACKGROUND_TRANSPARENT_ENDPOINT) {
        if (m_canvasResourcesInterface) {
            color = m_canvasResourcesInterface->resource(KoCanvasResource::BackgroundColor).value<KoColor>().convertedTo(colorSpace);
        } else {
            color = KoColor(stopRightEditor->color(), colorSpace);
        }
    } else {
        color = KoColor(stopRightEditor->color(), colorSpace);
    }

    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index];
    segment->setStartType(endPointType);
    color.setOpacity(opacity);
    segment->setStartColor(color);

    stopRightEditor->setColor(color);
    stopRightEditor->setOpacity(opacity * 100.0);

    emit gradientSlider->updateRequested();
    emit sigGradientChanged();

    if (constrainStopButton->keepAspectRatio() && gradientSlider->selectedHandle().index > 0) {
        stopLeftEditor->setColorType(stopRightEditor->colorType());
    }
}

void KisSegmentGradientEditor::on_stopRightEditor_transparentToggled(bool checked)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_Stop) {
        return;
    } 
    const qreal opacity = checked ? 0.0 : 1.0;
    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index];
    segment->setStartType(KisGradientWidgetsUtils::colorTypeToSegmentEndPointType(stopRightEditor->colorType(), checked));
    KoColor color = segment->startColor();
    color.setOpacity(opacity);
    segment->setStartColor(color);
    stopRightEditor->setOpacity(opacity * 100.0);
    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
    if (constrainStopButton->keepAspectRatio() && gradientSlider->selectedHandle().index > 0) {
        stopLeftEditor->setTransparent(stopRightEditor->transparent());
    }
}

void KisSegmentGradientEditor::on_stopRightEditor_colorChanged(KoColor color)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_Stop) {
        return;
    }
    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index];
    KoColor c(color, segment->startColor().colorSpace());
    c.setOpacity(stopRightEditor->opacity() / 100.0);
    segment->setStartColor(c);
    stopRightEditor->update();
    emit sigGradientChanged();
    if (constrainStopButton->keepAspectRatio() && gradientSlider->selectedHandle().index > 0) {
        stopLeftEditor->setColor(stopRightEditor->color());
    }
}

void KisSegmentGradientEditor::on_stopRightEditor_opacityChanged(double opacity)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_Stop) {
        return;
    }
    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index];
    KoColor color = segment->startColor();
    color.setOpacity(opacity / 100.0);
    segment->setStartColor(color);
    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
    if (constrainStopButton->keepAspectRatio() && gradientSlider->selectedHandle().index > 0) {
        stopLeftEditor->setOpacity(stopRightEditor->opacity());
    }
}

void KisSegmentGradientEditor::on_constrainStopButton_keepAspectRatioChanged(bool keep)
{
    if (!keep || gradientSlider->selectedHandle().index == m_gradient->segments().size()) {
        return;
    }
    stopRightEditor->setColorType(stopLeftEditor->colorType());
    stopRightEditor->setTransparent(stopLeftEditor->transparent());
    stopRightEditor->setColor(stopLeftEditor->color());
    stopRightEditor->setOpacity(stopLeftEditor->opacity());
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::on_midPointPositionSlider_valueChanged(double position)
{
    if (gradientSlider->selectedHandle().type != KisSegmentGradientSlider::HandleType_MidPoint) {
        return;
    } 
    KoGradientSegment *segment = m_gradient->segments()[gradientSlider->selectedHandle().index];
    segment->setMiddleOffset(segment->startOffset() + (position / 100.0) * (segment->endOffset() - segment->startOffset()));    
    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::on_nameedit_editingFinished()
{
    m_gradient->setName(nameedit->text());
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::editSelectedHandle()
{
    if (gradientSlider->selectedHandle().type == KisSegmentGradientSlider::HandleType_None) {
        return;
    }

    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle(i18nc("Title for the segment gradient handle editor", "Edit Handle"));
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    QWidget *editor = handleEditorContainer->currentWidget();
    int index = handleEditorContainer->indexOf(editor);
    handleEditorContainer->removeWidget(editor);

    QVBoxLayout *dialogLayout = new QVBoxLayout;
    dialogLayout->setMargin(10);
    dialogLayout->addWidget(editor);

    dialog->setLayout(dialogLayout);
    editor->show();
    dialog->resize(0, 0);

    connect(dialog, &QDialog::finished, [this, editor, index](int)
                                        {
                                            handleEditorContainer->insertWidget(index, editor);
                                            handleEditorContainer->setCurrentIndex(index);
                                        });

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}
