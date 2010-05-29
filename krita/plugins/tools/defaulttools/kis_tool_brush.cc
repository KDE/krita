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

#include "kis_tool_brush.h"
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
#include <QComboBox>
#include <QSizePolicy>

#include <kis_debug.h>
#include <klocale.h>

#include "KoPointerEvent.h"
#include "KoCanvasBase.h"
#include "kis_slider_spin_box.h"

#include "kis_config.h"
#include "kis_paintop_preset.h"
#include "kis_paintop_registry.h"
#include "kis_cursor.h"
#include "kis_painter.h"
#include "kis_slider_spin_box.h"
#include "kis_paint_device.h"
#include "kis_layer.h"

#define MAXIMUM_SMOOTHNESS 1000
#define MAXIMUM_MAGNETISM 1000

const int MAXIMUM_RATE = 1000;  // 1 second for rate? Just quick test

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
    Q_ASSERT(m_painter->paintOp()->incremental());
    if (currentImage() && m_painter) {
        paintAt(m_previousPaintInformation);
        QRegion r = m_painter->dirtyRegion();
        dbgPlugins << "Timeout paint dirty region:" << r;
        currentNode()->setDirty(r);
    }
}


void KisToolBrush::initPaint(KoPointerEvent *e)
{
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


void KisToolBrush::endPaint()
{
    m_timer->stop();
    KisToolFreehand::endPaint();
}


void KisToolBrush::mouseMoveEvent(KoPointerEvent *e)
{
    KisToolFreehand::mouseMoveEvent(e);
    if (m_painter && m_painter->paintOp() && m_painter->paintOp()->incremental()) {
        m_timer->start(m_rate);
    }
}


void KisToolBrush::slotSetRate(int rate)
{
    m_rate = rate;
    m_sliderRate->setToolTip(QString::number(m_rate) + ' ' + i18n("ms"));
}

void KisToolBrush::slotSetSmoothness(int smoothness)
{
    m_smoothness = smoothness / (double)MAXIMUM_SMOOTHNESS;
}

void KisToolBrush::slotSetMagnetism(int magnetism)
{
    m_magnetism = expf(magnetism / (double)MAXIMUM_MAGNETISM) / expf(1.0);
}

QWidget * KisToolBrush::createOptionWidget()
{

    QWidget * optionWidget = KisToolFreehand::createOptionWidget();
    optionWidget->setObjectName(toolId() + "option widget");

    QLabel* labelRate = new QLabel(i18n("Rate:"), optionWidget);
    m_sliderRate = new KisSliderSpinBox (optionWidget);
    m_sliderRate->setRange(0, MAXIMUM_RATE);
    m_sliderRate->setExponentRatio(3.0);
    connect(m_sliderRate, SIGNAL(valueChanged(int)), SLOT(slotSetRate(int)));
    m_sliderRate->setValue(m_rate);
    m_sliderRate->setToolTip(QString::number(m_rate) + ' ' + i18n("ms"));

    addOptionWidgetOption(m_sliderRate, labelRate);

    m_chkSmooth = new QCheckBox(i18nc("smooth out the curves while drawing", "Smoothness:"), optionWidget);
    m_chkSmooth->setObjectName("chkSmooth");
    m_chkSmooth->setChecked(m_smooth);
    connect(m_chkSmooth, SIGNAL(toggled(bool)), this, SLOT(setSmooth(bool)));

    m_sliderSmoothness = new KisSliderSpinBox(optionWidget);
    m_sliderSmoothness->setRange(0, MAXIMUM_SMOOTHNESS);
    m_sliderSmoothness->setEnabled(true);
    connect(m_chkSmooth, SIGNAL(toggled(bool)), m_sliderSmoothness, SLOT(setEnabled(bool)));
    connect(m_sliderSmoothness, SIGNAL(valueChanged(int)), SLOT(slotSetSmoothness(int)));
    m_sliderSmoothness->setValue(m_smoothness * MAXIMUM_SMOOTHNESS);

    addOptionWidgetOption(m_sliderSmoothness, m_chkSmooth);

    // Drawing assistant configuration
    m_chkAssistant = new QCheckBox(i18n("Assistant:"), optionWidget);
    connect(m_chkAssistant, SIGNAL(toggled(bool)), this, SLOT(setAssistant(bool)));
    m_sliderMagnetism = new KisSliderSpinBox(optionWidget);
    m_sliderMagnetism->setToolTip(i18n("Assistant Magnetism"));
    m_sliderMagnetism->setRange(0, MAXIMUM_SMOOTHNESS);
    m_sliderMagnetism->setEnabled(false);
    connect(m_chkAssistant, SIGNAL(toggled(bool)), m_sliderMagnetism, SLOT(setEnabled(bool)));
    m_sliderMagnetism->setValue(m_magnetism * MAXIMUM_MAGNETISM);
    connect(m_sliderMagnetism, SIGNAL(valueChanged(int)), SLOT(slotSetMagnetism(int)));

    addOptionWidgetOption(m_sliderMagnetism, m_chkAssistant);

    return optionWidget;
}

#include "kis_tool_brush.moc"

