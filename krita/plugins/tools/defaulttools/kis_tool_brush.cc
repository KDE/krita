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
#include <QSlider>
#include <QComboBox>
#include <QSizePolicy>

#include <kis_debug.h>
#include <klocale.h>

#include "KoPointerEvent.h"
#include "KoCanvasBase.h"
#include "KoSliderCombo.h"

#include "kis_config.h"
#include "kis_paintop_preset.h"
#include "kis_paintop_registry.h"
#include "kis_cursor.h"
#include "kis_painter.h"

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


void KisToolBrush::slotSetRate(qreal rate)
{
    m_rate = qRound(rate);
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
//    optionWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_chkSmooth = new QCheckBox(i18nc("smooth out the curves while drawing", "Smoothness"), optionWidget);
    m_chkSmooth->setObjectName("chkSmooth");
    m_chkSmooth->setChecked(m_smooth);
    connect(m_chkSmooth, SIGNAL(toggled(bool)), this, SLOT(setSmooth(bool)));

    QLabel* labelRate = new QLabel(i18n("Rate:"), optionWidget);
    m_sliderRate = new KoSliderCombo(optionWidget);
    m_sliderRate->setMinimum(0);
    m_sliderRate->setMaximum(MAXIMUM_RATE);
    connect(m_sliderRate, SIGNAL(valueChanged(qreal, bool)), SLOT(slotSetRate(qreal)));
    m_sliderRate->setValue(m_rate);
    m_sliderRate->setToolTip(QString::number(m_rate) + ' ' + i18n("ms"));


    m_sliderSmoothness = new QSlider(Qt::Horizontal, optionWidget);
    m_sliderSmoothness->setMinimum(0);
    m_sliderSmoothness->setMaximum(MAXIMUM_SMOOTHNESS);
    m_sliderSmoothness->setEnabled(false);
    connect(m_chkSmooth, SIGNAL(toggled(bool)), m_sliderSmoothness, SLOT(setEnabled(bool)));
    connect(m_sliderSmoothness, SIGNAL(valueChanged(int)), SLOT(slotSetSmoothness(int)));
    m_sliderSmoothness->setValue(m_smoothness * MAXIMUM_SMOOTHNESS);

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
    m_optionLayout->addWidget(labelRate, 1, 0);
    m_optionLayout->addWidget(m_sliderRate, 1, 1, 1, 2);
    m_optionLayout->addWidget(m_chkSmooth, 2, 0);
    m_optionLayout->addWidget(m_sliderSmoothness, 2, 1, 1, 2);
    m_optionLayout->addWidget(m_chkAssistant, 4, 0);
    m_optionLayout->addWidget(labelMagnetism, 5, 0);
    m_optionLayout->addWidget(m_sliderMagnetism, 5, 1, 1, 2);

    optionWidget->setFixedHeight(optionWidget->sizeHint().height());

    return optionWidget;
}

#include "kis_tool_brush.moc"

