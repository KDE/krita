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
#include <klocale.h>
#include <QLabel>
#include <QLayout>
#include <QCheckBox>

#include <QVector>
#include <QRect>
#include <QColor>

#include <knuminput.h>

#include <KoUpdater.h>
#include <KoColorSpace.h>
#include <KoCanvasBase.h>
#include <KoPointerEvent.h>
#include <KoProgressUpdater.h>

#include <kis_layer.h>
#include <kis_painter.h>
#include <kis_pattern.h>
#include <kis_fill_painter.h>
#include <kis_selection.h>
#include <kis_system_locker.h>

#include <kis_view2.h>
#include <canvas/kis_canvas2.h>
#include <widgets/kis_cmb_composite.h>
#include <widgets/kis_slider_spin_box.h>
#include <kis_cursor.h>
#include <recorder/kis_recorded_fill_paint_action.h>
#include <recorder/kis_node_query_path.h>
#include <recorder/kis_action_recorder.h>
#include "kis_resources_snapshot.h"


KisToolFill::KisToolFill(KoCanvasBase * canvas)
        : KisToolPaint(canvas, KisCursor::load("tool_fill_cursor.png", 6, 6))
{
    setObjectName("tool_fill");
    m_painter = 0;
    m_oldColor = 0;
    m_threshold = 80;
    m_usePattern = false;
    m_unmerged = false;
    m_fillOnlySelection = false;

}

KisToolFill::~KisToolFill()
{
}

bool KisToolFill::flood(int startX, int startY)
{
    if (image()) {
        KisNodeSP projectionNode;
        if(m_unmerged) {
            projectionNode = currentNode();
        } else {
            projectionNode = image()->root();
        }
        KisRecordedFillPaintAction paintAction(KisNodeQueryPath::absolutePath(currentNode()), QPoint(startX, startY), KisNodeQueryPath::absolutePath(projectionNode));
        setupPaintAction(&paintAction);
        paintAction.setPattern(currentPattern());
        if(m_usePattern)
        {
            paintAction.setFillStyle(KisPainter::FillStylePattern);
        }
        image()->actionRecorder()->addAction(paintAction);
    }

    KisPaintDeviceSP device = currentNode()->paintDevice();
    if (!device) return false;
    KisSelectionSP selection = currentSelection();

    KisCanvas2* canvas = dynamic_cast<KisCanvas2 *>(this->canvas());
    KoProgressUpdater * updater = canvas->view()->createProgressUpdater(KoProgressUpdater::Unthreaded);
    updater->start(100, i18n("Flood Fill"));

    QVector<QRect> dirty;

    KisUndoAdapter *undoAdapter = image()->undoAdapter();
    undoAdapter->beginMacro(i18n("Flood Fill"));

    if (m_fillOnlySelection && selection) {
        KisPaintDeviceSP filled = new KisPaintDevice(device->colorSpace());
        KisFillPainter fillPainter(filled);
        fillPainter.setProgress(updater->startSubtask());

        if (m_usePattern)
            fillPainter.fillRect(0, 0,
                                 currentImage()->width(),
                                 currentImage()->height(),
                                 currentPattern());
        else
            fillPainter.fillRect(0, 0,
                                 currentImage()->width(),
                                 currentImage()->height(),
                                 currentFgColor(),
                                 m_opacity);

        dirty = fillPainter.takeDirtyRegion();

        m_painter = new KisPainter(device, currentSelection());
        Q_CHECK_PTR(m_painter);

        m_painter->beginTransaction("");

        m_painter->setCompositeOp(compositeOp());
        m_painter->setOpacity(m_opacity);

        foreach(const QRect &rc, dirty) {
            m_painter->bitBlt(rc.topLeft(), filled, rc);
        }

        m_painter->endTransaction(image()->undoAdapter());

    } else {

        KisFillPainter fillPainter(device, currentSelection());

        KisResourcesSnapshotSP resources =
            new KisResourcesSnapshot(image(), 0,
                                     this->canvas()->resourceManager());
        resources->setupPainter(&fillPainter);

        fillPainter.beginTransaction("");

        fillPainter.setProgress(updater->startSubtask());
        fillPainter.setOpacity(m_opacity);
        fillPainter.setFillThreshold(m_threshold);
        fillPainter.setCompositeOp(compositeOp());
        fillPainter.setSampleMerged(!m_unmerged);
        fillPainter.setCareForSelection(true);
        fillPainter.setWidth(currentImage()->width());
        fillPainter.setHeight(currentImage()->height());

        if (m_usePattern)
            fillPainter.fillPattern(startX, startY, currentImage()->mergedImage());
        else
            fillPainter.fillColor(startX, startY, currentImage()->mergedImage());

        fillPainter.endTransaction(image()->undoAdapter());
        dirty = fillPainter.takeDirtyRegion();
    }

    undoAdapter->endMacro();

    device->setDirty(dirty);
    delete updater;

    return true;
}

void KisToolFill::mousePressEvent(KoPointerEvent *event)
{
    if(PRESS_CONDITION(event, KisTool::HOVER_MODE,
                       Qt::LeftButton, Qt::NoModifier)) {

        if (!nodeEditable()) {
            return;
        }

        setMode(KisTool::PAINT_MODE);

        m_startPos = convertToPixelCoord(event).toPoint();
    }
    else {
        KisToolPaint::mousePressEvent(event);
    }
}

void KisToolFill::mouseReleaseEvent(KoPointerEvent *event)
{
    if(RELEASE_CONDITION(event, KisTool::PAINT_MODE, Qt::LeftButton)) {
        setMode(KisTool::HOVER_MODE);

        if (!currentNode() || currentNode()->systemLocked() ||
            !currentImage()->bounds().contains(m_startPos)) {

            return;
        }

        KisSystemLocker locker(currentNode());
        flood(m_startPos.x(), m_startPos.y());
        notifyModified();
    }
    else {
        KisToolPaint::mouseReleaseEvent(event);
    }
}

QWidget* KisToolFill::createOptionWidget()
{
    //QWidget *widget = KisToolPaint::createOptionWidget(parent);
    QWidget *widget = KisToolPaint::createOptionWidget();
    widget->setObjectName(toolId() + " option widget");
    m_lbThreshold = new QLabel(i18n("Threshold: "), widget);
    m_slThreshold = new KisSliderSpinBox(widget);
    m_slThreshold->setObjectName("int_widget");
    m_slThreshold->setRange(1, 100);
    m_slThreshold->setPageStep(3);
    m_slThreshold->setValue(m_threshold);
    connect(m_slThreshold, SIGNAL(valueChanged(int)), this, SLOT(slotSetThreshold(int)));

    m_checkUsePattern = new QCheckBox(i18n("Use pattern"), widget);
    m_checkUsePattern->setToolTip(i18n("When checked do not use the foreground color, but the gradient selected to fill with"));
    m_checkUsePattern->setChecked(m_usePattern);
    connect(m_checkUsePattern, SIGNAL(toggled(bool)), this, SLOT(slotSetUsePattern(bool)));

    m_checkSampleMerged = new QCheckBox(i18n("Limit to current layer"), widget);
    m_checkSampleMerged->setChecked(m_unmerged);
    connect(m_checkSampleMerged, SIGNAL(toggled(bool)), this, SLOT(slotSetSampleMerged(bool)));

    m_checkFillSelection = new QCheckBox(i18n("Fill entire selection"), widget);
    m_checkFillSelection->setToolTip(i18n("When checked do not look at the current layer colors, but just fill all of the selected area"));
    m_checkFillSelection->setChecked(m_fillOnlySelection);
    connect(m_checkFillSelection, SIGNAL(toggled(bool)), this, SLOT(slotSetFillSelection(bool)));

    addOptionWidgetOption(m_slThreshold, m_lbThreshold);

    addOptionWidgetOption(m_checkFillSelection);
    addOptionWidgetOption(m_checkSampleMerged);
    addOptionWidgetOption(m_checkUsePattern);

    widget->setFixedHeight(widget->sizeHint().height());

    return widget;
}

void KisToolFill::slotSetThreshold(int threshold)
{
    m_threshold = threshold;
}

void KisToolFill::slotSetUsePattern(bool state)
{
    m_usePattern = state;
}

void KisToolFill::slotSetSampleMerged(bool state)
{
    m_unmerged = state;
}

void KisToolFill::slotSetFillSelection(bool state)
{
    m_fillOnlySelection = state;
    m_slThreshold->setEnabled(!state);
    m_checkSampleMerged->setEnabled(!state);
}

#include "kis_tool_fill.moc"
