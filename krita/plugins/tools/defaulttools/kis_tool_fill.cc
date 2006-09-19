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

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcommand.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include <qcolor.h>

#include "knuminput.h"

#include "kis_layer.h"
#include "kis_cursor.h"
#include "kis_painter.h"
#include "kis_tool_brush.h"
#include "kis_cmb_composite.h"
#include "kis_tool_fill.h"
#include "kis_colorspace.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"
#include "kis_pattern.h"
#include "kis_fill_painter.h"
#include "kis_progress_display_interface.h"
#include "kis_undo_adapter.h"
#include "kis_canvas_subject.h"
#include "kis_selection.h"

KisToolFill::KisToolFill()
    : super(i18n("Fill"))
{
    setName("tool_fill");
    m_subject = 0;
    m_oldColor = 0;
    m_threshold = 15;
    m_usePattern = false;
    m_unmerged = false;
    m_fillOnlySelection = false;

    setCursor(KisCursor::load("tool_fill_cursor.png", 6, 6));
}

void KisToolFill::update(KisCanvasSubject *subject)
{
    m_subject = subject;
    m_currentImage = subject->currentImg();

    super::update(m_subject);
}

KisToolFill::~KisToolFill()
{
}

bool KisToolFill::flood(int startX, int startY)
{
    KisPaintDeviceSP device = m_currentImage->activeDevice();
    if (!device) return false;

    if (m_fillOnlySelection) {
        QRect rc = device->selection()->exactBounds();
        KisPaintDeviceSP filled = new KisPaintDevice(device->colorSpace(),  "filled");
        KisFillPainter painter(filled);
        if (m_usePattern)
            painter.fillRect(rc.x(), rc.y(), rc.width(), rc.height(),
                             m_subject->currentPattern());
        else
            painter.fillRect(rc.x(), rc.y(), rc.width(), rc.height(),
                             m_subject->fgColor(), m_opacity);
        painter.end();
        KisPainter painter2(device);
        if (m_currentImage->undo()) painter2.beginTransaction(i18n("Fill"));
        painter2.bltSelection(rc.x(), rc.y() , m_compositeOp, filled, m_opacity,
                              rc.x(), rc.y(), rc.width(), rc.height());

        device->setDirty(filled->extent());
        notifyModified();

        if (m_currentImage->undo()) {
            m_currentImage->undoAdapter()->addCommand(painter2.endTransaction());
        }
        return true;
    }

    KisFillPainter painter(device);
    if (m_currentImage->undo()) painter.beginTransaction(i18n("Flood Fill"));
    painter.setPaintColor(m_subject->fgColor());
    painter.setOpacity(m_opacity);
    painter.setFillThreshold(m_threshold);
    painter.setCompositeOp(m_compositeOp);
    painter.setPattern(m_subject->currentPattern());
    painter.setSampleMerged(!m_unmerged);
    painter.setCareForSelection(true);

    KisProgressDisplayInterface *progress = m_subject->progressDisplay();
    if (progress) {
        progress->setSubject(&painter, true, true);
    }

    if (m_usePattern)
        painter.fillPattern(startX, startY);
    else
        painter.fillColor(startX, startY);

    device->setDirty(painter.dirtyRect());
    notifyModified();

    if (m_currentImage->undo()) {
        m_currentImage->undoAdapter()->addCommand(painter.endTransaction());
    }

    return true;
}

void KisToolFill::buttonPress(KisButtonPressEvent *e)
{
    m_startPos = e->pos();
}

void KisToolFill::buttonRelease(KisButtonReleaseEvent *e)
{
    if (!m_subject) return;
    if (!m_currentImage || !m_currentImage->activeDevice()) return;
    if (e->button() != QMouseEvent::LeftButton) return;
    int x, y;
    x = m_startPos.floorX();
    y = m_startPos.floorY();
    if (!m_currentImage->bounds().contains(x, y)) {
        return;
    }
    flood(x, y);
    notifyModified();
}

QWidget* KisToolFill::createOptionWidget(QWidget* parent)
{
    QWidget *widget = super::createOptionWidget(parent);

    m_lbThreshold = new QLabel(i18n("Threshold: "), widget);
    m_slThreshold = new KIntNumInput( widget, "int_widget");
    m_slThreshold->setRange( 1, 100);
    m_slThreshold->setSteps( 3, 3);
    m_slThreshold->setValue(m_threshold);
    connect(m_slThreshold, SIGNAL(valueChanged(int)), this, SLOT(slotSetThreshold(int)));

    m_checkUsePattern = new QCheckBox(i18n("Use pattern"), widget);
    m_checkUsePattern->setChecked(m_usePattern);
    connect(m_checkUsePattern, SIGNAL(toggled(bool)), this, SLOT(slotSetUsePattern(bool)));

    m_checkSampleMerged = new QCheckBox(i18n("Limit to current layer"), widget);
    m_checkSampleMerged->setChecked(m_unmerged);
    connect(m_checkSampleMerged, SIGNAL(toggled(bool)), this, SLOT(slotSetSampleMerged(bool)));

    m_checkFillSelection = new QCheckBox(i18n("Fill entire selection"), widget);
    m_checkFillSelection->setChecked(m_fillOnlySelection);
    connect(m_checkFillSelection, SIGNAL(toggled(bool)), this, SLOT(slotSetFillSelection(bool)));

    addOptionWidgetOption(m_slThreshold, m_lbThreshold);

    addOptionWidgetOption(m_checkFillSelection);
    addOptionWidgetOption(m_checkSampleMerged);
    addOptionWidgetOption(m_checkUsePattern);

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

void KisToolFill::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection->action(name()));

    if (m_action == 0) {
        m_action = new KRadioAction(i18n("&Fill"),
                        "tool_color_fill",
                        Qt::Key_F,
                        this,
                        SLOT(activate()),
                        collection,
                        name());
        m_action->setToolTip(i18n("Contiguous fill"));
        m_action->setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

#include "kis_tool_fill.moc"
