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

KisToolFill::KisToolFill()
    : super(i18n("Fill"))
{
    setName("tool_fill");
    m_subject = 0;
    m_oldColor = 0;
    m_threshold = 15;
    m_usePattern = false;
    m_sampleMerged = false;

    // set custom cursor.
    setCursor(KisCursor::fillerCursor());
}

void KisToolFill::update(KisCanvasSubject *subject)
{
    m_subject = subject;
    m_currentImage = subject -> currentImg();

    super::update(m_subject);
}

KisToolFill::~KisToolFill()
{
}

bool KisToolFill::flood(int startX, int startY)
{
    KisPaintDeviceImplSP device = m_currentImage->activeDevice();

    KisFillPainter painter(device);
    painter.beginTransaction(i18n("Floodfill"));
    painter.setPaintColor(m_subject -> fgColor());
    painter.setOpacity(m_opacity);
    painter.setFillThreshold(m_threshold);
    painter.setCompositeOp(m_compositeOp);
    painter.setPattern(m_subject -> currentPattern());
    painter.setSampleMerged(m_sampleMerged);

    KisProgressDisplayInterface *progress = m_subject -> progressDisplay();
    if (progress) {
        progress -> setSubject(&painter, true, true);
    }

    if (m_usePattern)
        painter.fillPattern(startX, startY);
    else
        painter.fillColor(startX, startY);

    m_currentImage -> notify();
    notifyModified();

    KisUndoAdapter *adapter = m_currentImage -> undoAdapter();
    if (adapter) {
        adapter -> addCommand(painter.endTransaction());
    }

    return true;
}

void KisToolFill::buttonPress(KisButtonPressEvent *e)
{
    if (!m_subject) return;
    if (!m_currentImage || !m_currentImage -> activeDevice()) return;
    if (e->button() != QMouseEvent::LeftButton) return;
    int x, y;
    x = e -> pos().floorX();
    y = e -> pos().floorY();
    if (!m_currentImage -> bounds().contains(x, y)) {
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
    m_slThreshold -> setRange( 0, 255);
    m_slThreshold -> setValue(m_threshold);
    connect(m_slThreshold, SIGNAL(valueChanged(int)), this, SLOT(slotSetThreshold(int)));

    m_checkUsePattern = new QCheckBox(i18n("Use pattern"), widget);
    m_checkUsePattern->setChecked(m_usePattern);
    connect(m_checkUsePattern, SIGNAL(stateChanged(int)), this, SLOT(slotSetUsePattern(int)));

    m_checkSampleMerged = new QCheckBox(i18n("Sample merged"), widget);
    m_checkSampleMerged->setChecked(m_sampleMerged);
    connect(m_checkSampleMerged, SIGNAL(stateChanged(int)), this, SLOT(slotSetSampleMerged(int)));

    QGridLayout *optionLayout = new QGridLayout(widget, 4, 3);
    super::addOptionWidgetLayout(optionLayout);

    optionLayout -> addWidget(m_lbThreshold, 1, 0);
    optionLayout -> addWidget(m_slThreshold, 1, 1);

    optionLayout -> addMultiCellWidget(m_checkUsePattern, 2, 2, 0, 2);
    optionLayout -> addMultiCellWidget(m_checkSampleMerged, 3, 3, 0, 2);

    return widget;
}

void KisToolFill::slotSetThreshold(int threshold)
{
    m_threshold = threshold;
}

void KisToolFill::slotSetUsePattern(int state)
{
    if (state == QButton::NoChange)
        return;
    m_usePattern = (state == QButton::On);
}

void KisToolFill::slotSetSampleMerged(int state)
{
    if (state == QButton::NoChange)
        return;
    m_sampleMerged = (state == QButton::On);
}

void KisToolFill::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection -> action(name()));

    if (m_action == 0) {
        m_action = new KRadioAction(i18n("&Fill"),
                        "color_fill",
                        Qt::Key_F,
                        this,
                        SLOT(activate()),
                        collection,
                        name());
        m_action -> setToolTip(i18n("Contiguous fill"));
        m_action -> setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

#include "kis_tool_fill.moc"
