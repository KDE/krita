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

#include <kis_view2.h>
#include <canvas/kis_canvas2.h>
#include <widgets/kis_cmb_composite.h>
#include <kis_cursor.h>

KisToolFill::KisToolFill(KoCanvasBase * canvas)
        : KisToolPaint(canvas, KisCursor::load("tool_fill_cursor.png", 6, 6))
{
    setObjectName("tool_fill");
    m_painter = 0;
    m_oldColor = 0;
    m_threshold = 15;
    m_usePattern = false;
    m_unmerged = false;
    m_fillOnlySelection = false;

}

KisToolFill::~KisToolFill()
{
}

bool KisToolFill::flood(int startX, int startY)
{

    KisPaintDeviceSP device = currentNode()->paintDevice();
    if (!device) return false;
    KisSelectionSP selection = currentSelection();

    KisCanvas2* canvas = dynamic_cast<KisCanvas2 *>(this->canvas());
    KoProgressUpdater * updater = canvas->view()->createProgressUpdater(KoProgressUpdater::Unthreaded);
    updater->start(100, i18n("Flood Fill"));

    QRegion dirty;

    if (m_fillOnlySelection && selection) {
#ifdef __GNUC__
#warning Port the fixes for filling the selection from 1.6!
#endif

        QRect rc = selection->selectedRect();
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

        dirty = fillPainter.dirtyRegion();

        m_painter = new KisPainter(device, currentSelection());
        Q_CHECK_PTR(m_painter);

        m_painter->beginTransaction(i18n("Fill"));

        QVector<QRect> rects = dirty.rects();

        QVector<QRect>::iterator it = rects.begin();
        QVector<QRect>::iterator end = rects.end();

        m_painter->setCompositeOp(m_compositeOp);
        m_painter->setOpacity(m_opacity);

        while (it != end) {
            QRect rc = *it;
            m_painter->bitBlt(rc.topLeft(), filled, rc);
            ++it;
        }

        canvas->addCommand(m_painter->endTransaction());

    } else {

        KisFillPainter fillPainter(device, currentSelection());
        setupPainter(&fillPainter);
        fillPainter.beginTransaction(i18n("Flood Fill"));

        fillPainter.setProgress(updater->startSubtask());
        fillPainter.setOpacity(m_opacity);
        fillPainter.setFillThreshold(m_threshold);
        fillPainter.setCompositeOp(m_compositeOp);
        fillPainter.setSampleMerged(!m_unmerged);
        fillPainter.setCareForSelection(true);
        fillPainter.setWidth(currentImage()->width());
        fillPainter.setHeight(currentImage()->height());

        if (m_usePattern)
            fillPainter.fillPattern(startX, startY, currentImage()->mergedImage());
        else
            fillPainter.fillColor(startX, startY, currentImage()->mergedImage());

        dirty = fillPainter.dirtyRegion();
        canvas->addCommand(fillPainter.endTransaction());
    }
    device->setDirty(dirty);
    delete updater;


    return true;
}

void KisToolFill::mousePressEvent(KoPointerEvent *e)
{
    QPointF pos = convertToPixelCoord(e);
    m_startPos = pos;
}

void KisToolFill::mouseReleaseEvent(KoPointerEvent *e)
{

    if (!canvas()) return;
    if (!currentNode()) return;
    if (!currentImage() || !currentNode()->paintDevice()) return;
    if (e->button() == Qt::LeftButton) {
        int x, y;

        x = static_cast<int>(m_startPos.x());
        y = static_cast<int>(m_startPos.y());

        if (!currentImage()->bounds().contains(x, y)) {
            return;
        }

        flood(x, y);
        notifyModified();
    } else {
        KisToolPaint::mouseReleaseEvent(e);
    }
}

QWidget* KisToolFill::createOptionWidget()
{
    //QWidget *widget = KisToolPaint::createOptionWidget(parent);
    QWidget *widget = KisToolPaint::createOptionWidget();
    widget->setObjectName(toolId() + " option widget");
    m_lbThreshold = new QLabel(i18n("Threshold: "), widget);
    m_slThreshold = new KIntNumInput(widget);
    m_slThreshold->setObjectName("int_widget");
    m_slThreshold->setRange(1, 100);
    m_slThreshold->setSteps(3, 3);
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
