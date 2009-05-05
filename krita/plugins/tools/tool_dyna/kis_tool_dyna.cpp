/*
 *  kis_tool_dyna.cpp - part of Krita
 *
 *  Copyright (c) 2009 Lukáš Tvrdý <LukasT.dev@gmail.com>
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

#include "kis_tool_dyna.h"
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
#include <QComboBox>

#include <kis_debug.h>
#include <klocale.h>

#include "KoPointerEvent.h"
#include "KoCanvasBase.h"

#include "kis_config.h"
#include "kis_paintop_preset.h"
#include "kis_paintop_registry.h"
#include "kis_paint_information.h"

#include "kis_cursor.h"
#include "kis_painter.h"

#include "kis_paint_device.h"
#include "kis_layer.h"

#define MAXIMUM_SMOOTHNESS 1000
#define MAXIMUM_MAGNETISM 1000

KisToolDyna::KisToolDyna(KoCanvasBase * canvas)
        : KisToolFreehand(canvas, KisCursor::load("tool_freehand_cursor.png", 5, 5), i18n("Dyna"))
{
    setObjectName("tool_dyna");

    m_rate = 200; // Conveniently hardcoded for now
    m_timer = new QTimer(this);
    Q_CHECK_PTR(m_timer);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timeoutPaint()));

    initDyna();
}


void KisToolDyna::initDyna() {
    // dyna init
    first = false;

    /* dynadraw init */
    m_curmass = 0.5;
    m_curdrag = 0.15;
    m_mouse.fixedangle = true;
    m_width = 1.5;
    m_xangle = 0.60;
    m_yangle = 0.20;
    m_widthRange = 0.05;

    m_previousPressure = 0.5;

}


KisToolDyna::~KisToolDyna()
{
    delete m_timer;
    m_timer = 0;
}

void KisToolDyna::timeoutPaint()
{
    Q_ASSERT(m_painter->paintOp()->incremental());

    if (currentImage() && m_painter) {
        paintAt(m_previousPaintInformation);
        QRegion r = m_painter->dirtyRegion();
        dbgPlugins << "Timeout paint dirty region:" << r;
        currentNode()->setDirty(r);
    }

}


void KisToolDyna::initPaint(KoPointerEvent *e)
{
    initDyna();
    initMouse( convertToPixelCoord(e->point) );

    KisToolFreehand::initPaint(e);

    if (!m_painter) {
        warnKrita << "Didn't create a painter! Something is wrong!";
        return;
    }

    m_painter->setPaintOpPreset(currentPaintOpPreset(), currentImage());
    if (m_painter->paintOp()->incremental()) {
        m_timer->start(m_rate);
    }
}


void KisToolDyna::endPaint()
{
    dbgPlugins << "DYNA endPAINT!!";
    m_timer->stop();
    KisToolFreehand::endPaint();
}


void KisToolDyna::mouseMoveEvent(KoPointerEvent *e)
{
    if (m_mode != PAINT) return;

    dbgPlugins << "mouse move!!";
    initMouse( convertToPixelCoord(e->point)  );

    qreal mx, my;   
    mx = m_mousePos.x();
    my = m_mousePos.y();

    if (!first){
        m_mouse.init(mx,my);
        m_odelx = 0.0;
        m_odely = 0.0;

        first = true;
        m_previousPaintInformation = KisPaintInformation(e->point);

        return;
    }

    if (applyFilter(mx, my)){
        drawSegment(e);
    }

    //KisToolFreehand::mouseMoveEvent(e);
    if ( m_painter && m_painter->paintOp() && m_painter->paintOp()->incremental() ) {
        m_timer->start(m_rate);
    }
}



void KisToolDyna::slotSetSmoothness(int smoothness)
{
    m_smoothness = smoothness / (double)MAXIMUM_SMOOTHNESS;
}

void KisToolDyna::slotSetMagnetism(int magnetism)
{
    m_magnetism = expf(magnetism / (double)MAXIMUM_MAGNETISM) / expf(1.0);
}

QWidget * KisToolDyna::createOptionWidget()
{

    QWidget * optionWidget = KisToolFreehand::createOptionWidget();
    optionWidget->setObjectName(toolId() + "option widget");
    m_chkSmooth = new QCheckBox(i18nc("smooth out the curves while drawing", "Smoothness"), optionWidget);
    m_chkSmooth->setObjectName("chkSmooth");
    m_chkSmooth->setChecked(m_smooth);
    connect(m_chkSmooth, SIGNAL(toggled(bool)), this, SLOT(setSmooth(bool)));

    m_sliderSmoothness = new QSlider(Qt::Horizontal, optionWidget);
    m_sliderSmoothness->setMinimum(0);
    m_sliderSmoothness->setMaximum(MAXIMUM_SMOOTHNESS);
    m_sliderSmoothness->setEnabled(false);
    connect(m_chkSmooth, SIGNAL(toggled(bool)), m_sliderSmoothness, SLOT(setEnabled(bool)));
    connect(m_sliderSmoothness, SIGNAL(valueChanged(int)), SLOT(slotSetSmoothness(int)));
    m_sliderSmoothness->setValue(m_smoothness * MAXIMUM_SMOOTHNESS);
    // Queueing checkbox
    m_chkQueueing = new QCheckBox(i18n("Queue paint tasks"), optionWidget);
    m_chkQueueing->setChecked(true);
    connect(m_chkQueueing, SIGNAL(toggled(bool)), this, SLOT(setQueueing(bool)));
    // Drawing assistant configuration
    m_chkAssistant = new QCheckBox(i18n("Assistant:"), optionWidget);
    connect(m_chkAssistant, SIGNAL(toggled(bool)), this, SLOT(setAssistant(bool)));
    QLabel* labelMagnetism = new QLabel(i18n("Magnetism:"), optionWidget);
    connect(m_chkAssistant, SIGNAL(toggled(bool)), labelMagnetism, SLOT(setEnabled(bool)));
    m_sliderMagnetism = new QSlider(Qt::Horizontal, optionWidget);
    m_sliderMagnetism->setMinimum(0);
    m_sliderMagnetism->setMaximum(MAXIMUM_SMOOTHNESS);
    m_sliderMagnetism->setEnabled(false);
    connect(m_chkAssistant, SIGNAL(toggled(bool)), m_sliderMagnetism, SLOT(setEnabled(bool)));
    m_sliderMagnetism->setValue(m_magnetism * MAXIMUM_MAGNETISM);
    connect(m_sliderMagnetism, SIGNAL(valueChanged(int)), SLOT(slotSetMagnetism(int)));

    m_optionLayout = new QGridLayout(optionWidget);
    Q_CHECK_PTR(m_optionLayout);

    m_optionLayout->setMargin(0);
    m_optionLayout->setSpacing(2);

    KisToolFreehand::addOptionWidgetLayout(m_optionLayout);
    m_optionLayout->addWidget(m_chkSmooth, 1, 0);
    m_optionLayout->addWidget(m_sliderSmoothness, 1, 2);
    m_optionLayout->addWidget(m_chkAssistant, 3, 0);
    m_optionLayout->addWidget(labelMagnetism, 4, 0);
    m_optionLayout->addWidget(m_sliderMagnetism, 4, 1, 1, 2);
    m_optionLayout->addWidget(m_chkQueueing, 5, 0);

    return optionWidget;
}

// dyna algorithm
int KisToolDyna::applyFilter(qreal mx, qreal my) {
    qreal mass, drag;
    qreal fx, fy;

    /* calculate mass and drag */
    mass = flerp(1.0, 160.0, m_curmass);
    drag = flerp(0.00, 0.5, m_curdrag * m_curdrag);

    /* calculate force and acceleration */
    fx = mx - m_mouse.curx;
    fy = my - m_mouse.cury;

    m_mouse.acc = sqrt(fx * fx + fy * fy);

    if(m_mouse.acc < 0.000001){
        return 0;
    }

    m_mouse.accx = fx / mass;
    m_mouse.accy = fy / mass;

    /* calculate new velocity */
    m_mouse.velx += m_mouse.accx;
    m_mouse.vely += m_mouse.accy;
    m_mouse.vel = sqrt(m_mouse.velx * m_mouse.velx + m_mouse.vely * m_mouse.vely);
    m_mouse.angx = -m_mouse.vely;
    m_mouse.angy = m_mouse.velx;
    if(m_mouse.vel < 0.000001){
        return 0;
    }

    /* calculate angle of drawing tool */
    m_mouse.angx /= m_mouse.vel;
    m_mouse.angy /= m_mouse.vel;
    if(m_mouse.fixedangle) {
        m_mouse.angx = m_xangle;
        m_mouse.angy = m_yangle;
    }

    m_mouse.velx = m_mouse.velx * (1.0 - drag);
    m_mouse.vely = m_mouse.vely * (1.0 - drag);

    m_mouse.lastx = m_mouse.curx;
    m_mouse.lasty = m_mouse.cury;
    m_mouse.curx = m_mouse.curx + m_mouse.velx;
    m_mouse.cury = m_mouse.cury + m_mouse.vely;

    return 1;
}


void KisToolDyna::drawSegment(KoPointerEvent* event) {
    qreal delx, dely;
    qreal wid;
    qreal px, py, nx, ny;

    wid = m_widthRange - m_mouse.vel;

    wid = wid * m_width;

    if(wid < 0.00001){
        wid = 0.00001;
    }

    delx = m_mouse.angx * wid;
    dely = m_mouse.angy * wid;

    px = m_mouse.lastx;
    py = m_mouse.lasty;
    nx = m_mouse.curx;
    ny = m_mouse.cury;

    QPointF prev( px , py );       // previous position
    QPointF now( nx , ny );         // new position

    QPointF prevr( px + m_odelx , py + m_odely );  
    QPointF prevl( px - m_odelx , py - m_odely );  

    QPointF nowl( nx - delx , ny - dely);
    QPointF nowr( nx + delx , ny + dely);

    // transform coords from float points into image points
    prev.rx() *= currentImage()->width();
    prevr.rx()*= currentImage()->width();
    prevl.rx()*= currentImage()->width();
    now.rx()  *= currentImage()->width();
    nowl.rx() *= currentImage()->width();
    nowr.rx() *= currentImage()->width();

    prev.ry() *= currentImage()->height();
    prevr.ry()*= currentImage()->height();
    prevl.ry()*= currentImage()->height();
    now.ry()  *= currentImage()->height();
    nowl.ry() *= currentImage()->height();
    nowr.ry() *= currentImage()->height();



    qreal m_pressure;
    qreal xTilt, yTilt;
    qreal m_rotation;
    qreal m_tangentialPressure;

    dbgPlugins << "m_mouse.vel: " << m_mouse.vel;
    dbgPlugins << "m_mouse.velx: " << m_mouse.velx;
    dbgPlugins << "m_mouse.vely: " << m_mouse.vely;
    dbgPlugins << "m_mouse.accx: " << m_mouse.accx;
    dbgPlugins << "m_mouse.accy: " << m_mouse.accy;

    m_pressure =  m_mouse.vel * 100;
    if (m_pressure > 1.0) m_pressure = 1.0;
    if (m_pressure < 0.0) m_pressure = 0.0;

    KisPaintInformation pi1 = KisPaintInformation(prev, m_pressure);
    KisPaintInformation info = KisPaintInformation(now, m_pressure);
    paintLine(pi1 , info);

    m_previousPaintInformation = info;
    m_previousPressure = m_pressure;

    m_odelx = delx;
    m_odely = dely;
}

void KisToolDyna::mousePressEvent(KoPointerEvent *e){
   if (!currentNode())
        return;

    if (!currentNode()->paintDevice())
        return;

    if (currentPaintOpPreset() && currentPaintOpPreset()->settings()) {
        m_paintIncremental = currentPaintOpPreset()->settings()->paintIncremental();
        currentPaintOpPreset()->settings()->mousePressEvent(e);
        if (e->isAccepted()) {
            return;
        }
    }

    if (e->button() == Qt::LeftButton) {
        initPaint(e);
        m_previousPaintInformation = KisPaintInformation(convertToPixelCoord(e->point),
                                     e->pressure(), e->xTilt(), e->yTilt(),
                                     KisVector2D::Zero(),
                                     e->rotation(), e->tangentialPressure());
    }

}
#include "kis_tool_dyna.moc"

