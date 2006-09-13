/*
 *  kis_tool_brush.cc - part of Krita
 *
 *  Copyright (c) 2003-2004 Boudewijn Rempt <boud@valdyas.org>
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
#include <QEvent>
#include <QLabel>
#include <QLayout>
#include <QWidget>
#include <QTimer>
#include <QPushButton>
#include <QPainter>
#include <QRect>
#include <QCheckBox>
#include <QGridLayout>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>

#include "kis_config.h"
#include "kis_brush.h"
#include "kis_paintop.h"
#include "kis_paintop_registry.h"
#include "kis_cmb_composite.h"
#include "kis_cursor.h"
#include "kis_painter.h"
#include "kis_tool_brush.h"
#include "kis_canvas_subject.h"
#include "kis_boundary.h"
#include "kis_move_event.h"
#include "kis_canvas.h"
#include "kis_layer.h"

KisToolBrush::KisToolBrush()
        : super(i18n("Brush"))
{
    setObjectName("tool_brush");
    setCursor(KisCursor::load("tool_freehand_cursor.png", 5, 5));
    m_rate = 100; // Conveniently hardcoded for now
    m_timer = new QTimer(this);
    Q_CHECK_PTR(m_timer);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(timeoutPaint()));

}

KisToolBrush::~KisToolBrush()
{
    delete m_timer;
    m_timer = 0;
}

void KisToolBrush::timeoutPaint()
{
    if (currentImage() && painter()) {
        painter()->paintAt(m_prevPos, m_prevPressure, m_prevXTilt, m_prevYTilt);
        currentImage()->activeLayer()->setDirty(painter()->dirtyRect());
    }
}


void KisToolBrush::update(KisCanvasSubject *subject)
{
    super::update(subject);
}

void KisToolBrush::initPaint(KisEvent *e)
{
    super::initPaint(e);

    if (!m_painter) {
        kWarning() << "Didn't create a painter! Something is wrong!\n";
        return;
    }
    KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp(m_subject->currentPaintop(), m_subject->currentPaintopSettings(), m_painter);
    if (!op) return;

    m_subject->canvasController()->kiscanvas()->update(); // remove the outline

    painter()->setPaintOp(op); // And now the painter owns the op and will destroy it.

    if (op->incremental()) {
        m_timer->start( m_rate );
    }
}


void KisToolBrush::endPaint()
{
    m_timer->stop();
    super::endPaint();
}


void KisToolBrush::setup(KActionCollection *collection)
{

    m_action = collection->action(objectName());

    if (m_action == 0) {
        m_action = new KAction(KIcon("tool_freehand"),
                               i18n("&Brush"),
                               collection,
                               objectName());
        m_action->setShortcut(Qt::Key_B);
        connect(m_action, SIGNAL(triggered()), this, SLOT(activate()));
        m_action->setToolTip(i18n("Draw freehand"));
        m_action->setActionGroup(actionGroup());
        m_ownAction = true;
    }
}

void KisToolBrush::move(KisMoveEvent *e) {
    KisToolFreehand::move(e);
    KisConfig cfg;
    if (m_mode != PAINT && cfg.cursorStyle() == CURSOR_STYLE_OUTLINE)
        paintOutline(e->pos());
}

void KisToolBrush::leave(QEvent */*e*/) {
    m_subject->canvasController()->kiscanvas()->update(); // remove the outline
}


void KisToolBrush::slotSetPaintingMode( int mode )
{
    if (mode == QCheckBox::On) {
        // Direct painting
        m_paintIncremental = true;
    }
    else {
        m_paintIncremental = false;
    }
}


QWidget* KisToolBrush::createOptionWidget(QWidget* parent)
{
    QWidget *widget = super::createOptionWidget(parent);
    m_chkDirect = new QCheckBox(i18n("Paint incrementally"), widget);
    m_chkDirect->setObjectName("chkDirect");
    m_chkDirect->setChecked(true);
    connect(m_chkDirect, SIGNAL(stateChanged(int)), this, SLOT(slotSetPaintingMode(int)));

    m_optionLayout = new QGridLayout(widget);
    Q_CHECK_PTR(m_optionLayout);

    m_optionLayout->setMargin(0);
    m_optionLayout->setSpacing(6);

    super::addOptionWidgetLayout(m_optionLayout);
    m_optionLayout->addWidget(m_chkDirect, 0, 0);

    return widget;
}

#include "kis_tool_brush.moc"

