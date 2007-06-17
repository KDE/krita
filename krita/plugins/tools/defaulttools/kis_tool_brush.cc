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
#include <QSlider>

#include <kdebug.h>
#include <klocale.h>

#include "KoPointerEvent.h"
#include "KoCanvasBase.h"

#include "kis_config.h"
#include "kis_brush.h"
#include "kis_paintop.h"
#include "kis_paintop_registry.h"
#include "kis_cursor.h"
#include "kis_painter.h"
#include "kis_tool_brush.h"
#include "kis_boundary.h"
#include "kis_layer.h"

#define MAXIMUM_SMOOTHNESS 1000

KisToolBrush::KisToolBrush(KoCanvasBase * canvas)
        : KisToolFreehand(canvas, KisCursor::load("tool_freehand_cursor.png", 5, 5), i18n("Brush"))
{
    setObjectName("tool_brush");

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
    if (m_currentImage && m_painter) {
        m_painter->paintAt( m_previousPaintInformation );
        QRegion r = m_painter->dirtyRegion();
        kDebug() << "Timeout paint dirty region: " << r << endl;
        m_currentLayer->setDirty(r);
    }
}


void KisToolBrush::initPaint(KoPointerEvent *e)
{
    KisToolFreehand::initPaint(e);

    if (!m_painter) {
        kWarning() << "Didn't create a painter! Something is wrong!\n";
        return;
    }

    KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp(m_currentPaintOp,
                                                              m_currentPaintOpSettings,
                                                              m_painter,
                                                              m_currentImage);
    if (!op) return;

#if 0
    // XXX: TOOL_REFACTOR: how to update all of the canvas? Or how to
    // find out the cursor area around the cursor so we can remove the
    // outline?
    m_canvas->updateCanvas(); // remove the outline
#endif

    m_painter->setPaintOp(op); // And now the painter owns the op and will destroy it.

    if (op->incremental()) {
        m_timer->start( m_rate );
    }
}


void KisToolBrush::endPaint()
{
    m_timer->stop();
    KisToolFreehand::endPaint();
}


void KisToolBrush::mouseMoveEvent(KoPointerEvent *e) {
    KisToolFreehand::mouseMoveEvent(e);
    KisConfig cfg;
    if (m_mode != PAINT && cfg.cursorStyle() == CURSOR_STYLE_OUTLINE)
        paintOutline(e->pos());
}

#if 0
// XXX: TOOL_REFACTOR
void KisToolBrush::leave(QEvent */*e*/) {
    m_canvs->updateCanvas(); // remove the outline
}
#endif

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

void KisToolBrush::slotSetSmoothness( int smoothness )
{
    m_smoothness = smoothness / (double)MAXIMUM_SMOOTHNESS;
}

QWidget * KisToolBrush::createOptionWidget()
{

    QWidget * optionWidget = KisToolFreehand::createOptionWidget();

    m_chkDirect = new QCheckBox(i18n("Paint incrementally"), optionWidget);
    m_chkDirect->setObjectName("chkDirect");
    m_chkDirect->setChecked(true);
    connect(m_chkDirect, SIGNAL(stateChanged(int)), this, SLOT(slotSetPaintingMode(int)));

    m_chkSmooth = new QCheckBox(i18n("Smooth"), optionWidget);
    m_chkSmooth->setObjectName("chkSmooth");
    m_chkSmooth->setChecked(false);
    connect(m_chkSmooth, SIGNAL(toggled(bool)), this, SLOT(setSmooth(bool)));

    QLabel* labelSmoothness = new QLabel(i18n("Smoothness:"), optionWidget);
    labelSmoothness->setEnabled(false);
    connect(m_chkSmooth, SIGNAL(toggled(bool)), labelSmoothness, SLOT(setEnabled(bool)));
    
    m_sliderSmoothness = new QSlider(Qt::Horizontal, optionWidget);
    m_sliderSmoothness->setMinimum(0);
    m_sliderSmoothness->setMaximum(MAXIMUM_SMOOTHNESS);
    m_sliderSmoothness->setEnabled(false);
    connect(m_chkSmooth, SIGNAL(toggled(bool)), m_sliderSmoothness, SLOT(setEnabled(bool)));
    m_sliderSmoothness->setValue( m_smoothness * MAXIMUM_SMOOTHNESS );
    
    m_optionLayout = new QGridLayout(optionWidget);
    Q_CHECK_PTR(m_optionLayout);

    m_optionLayout->setMargin(0);
    m_optionLayout->setSpacing(6);

    KisToolFreehand::addOptionWidgetLayout(m_optionLayout);
    m_optionLayout->addWidget(m_chkDirect, 0, 0);
    m_optionLayout->addWidget(m_chkSmooth, 1, 0);
    m_optionLayout->addWidget(labelSmoothness, 2, 0);
    m_optionLayout->addWidget(m_sliderSmoothness, 2, 1);

    return optionWidget;
}

#include "kis_tool_brush.moc"

