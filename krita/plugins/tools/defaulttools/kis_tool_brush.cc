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

#include <QCheckBox>

#include <klocale.h>

#include "kis_cursor.h"
#include "kis_slider_spin_box.h"


#define MAXIMUM_SMOOTHNESS 1000
#define MAXIMUM_MAGNETISM 1000


KisToolBrush::KisToolBrush(KoCanvasBase * canvas)
        : KisToolFreehand(canvas, KisCursor::load("tool_freehand_cursor.png", 5, 5), i18nc("(qtundo-format)", "Brush"))
{
    setObjectName("tool_brush");
}

KisToolBrush::~KisToolBrush()
{
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
    m_chkAssistant->setToolTip(i18n("You need to add Ruler Assistants before this tool will work."));
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

