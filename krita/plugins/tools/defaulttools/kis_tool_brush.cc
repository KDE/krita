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
#include <QComboBox>

#include <klocale.h>

#include "kis_cursor.h"
#include "kis_slider_spin_box.h"

#define MAXIMUM_SMOOTHNESS_DISTANCE 1000.0 // 0..1000.0 == weight in gui
#define MAXIMUM_MAGNETISM 1000


KisToolBrush::KisToolBrush(KoCanvasBase * canvas)
    : KisToolFreehand(canvas,
                      KisCursor::load("tool_freehand_cursor.png", 5, 5),
                      i18nc("(qtundo-format)", "Brush"))
{
    setObjectName("tool_brush");
}

KisToolBrush::~KisToolBrush()
{
}

void KisToolBrush::slotSetSmoothingType(int index)
{
    switch (index) {
    case 0:
        m_smoothingOptions.smoothingType = KisSmoothingOptions::NO_SMOOTHING;
        m_sliderSmoothnessDistance->setEnabled(false);
        m_sliderTailAggressiveness->setEnabled(false);
        m_chkSmoothPressure->setEnabled(false);
        break;
    case 1:
        m_smoothingOptions.smoothingType = KisSmoothingOptions::SIMPLE_SMOOTHING;
        m_sliderSmoothnessDistance->setEnabled(false);
        m_sliderTailAggressiveness->setEnabled(false);
        m_chkSmoothPressure->setEnabled(false);
        break;
    case 2:
    default:
        m_smoothingOptions.smoothingType = KisSmoothingOptions::WEIGHTED_SMOOTHING;
        m_sliderSmoothnessDistance->setEnabled(true);
        m_sliderTailAggressiveness->setEnabled(true);
        m_chkSmoothPressure->setEnabled(true);
    }
}

void KisToolBrush::slotSetSmoothnessDistance(qreal distance)
{
    m_smoothingOptions.smoothnessDistance = distance;
}

void KisToolBrush::slotSetTailAgressiveness(qreal argh_rhhrr)
{
    m_smoothingOptions.tailAggressiveness = argh_rhhrr;
}

void KisToolBrush::setSmoothPressure(bool value)
{
    m_smoothingOptions.smoothPressure = value;
}

void KisToolBrush::slotSetMagnetism(int magnetism)
{
    m_magnetism = expf(magnetism / (double)MAXIMUM_MAGNETISM) / expf(1.0);
}

QWidget * KisToolBrush::createOptionWidget()
{
    QWidget * optionWidget = KisToolFreehand::createOptionWidget();
    optionWidget->setObjectName(toolId() + "option widget");

    // Line smoothing configuration
    m_cmbSmoothingType = new QComboBox(optionWidget);
    m_cmbSmoothingType->addItems(QStringList() << i18n("No Smoothing") << i18n("Basic Smoothing") << i18n("Weighted Smoothing"));
    m_cmbSmoothingType->setCurrentIndex(1);
    connect(m_cmbSmoothingType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetSmoothingType(int)));
    addOptionWidgetOption(m_cmbSmoothingType);

    m_sliderSmoothnessDistance = new KisDoubleSliderSpinBox(optionWidget);
    m_sliderSmoothnessDistance->setRange(3.0, MAXIMUM_SMOOTHNESS_DISTANCE, 1);
    m_sliderSmoothnessDistance->setEnabled(true);
    connect(m_sliderSmoothnessDistance, SIGNAL(valueChanged(qreal)), SLOT(slotSetSmoothnessDistance(qreal)));
    m_sliderSmoothnessDistance->setValue(m_smoothingOptions.smoothnessDistance);
    addOptionWidgetOption(m_sliderSmoothnessDistance, new QLabel(i18n("Smooth Distance:")));

    m_sliderTailAggressiveness = new KisDoubleSliderSpinBox(optionWidget);
    m_sliderTailAggressiveness->setRange(0.0, 1.0, 2);
    m_sliderTailAggressiveness->setEnabled(true);
    connect(m_sliderTailAggressiveness, SIGNAL(valueChanged(qreal)), SLOT(slotSetTailAgressiveness(qreal)));
    m_sliderTailAggressiveness->setValue(m_smoothingOptions.tailAggressiveness);
    addOptionWidgetOption(m_sliderTailAggressiveness, new QLabel(i18n("Tail Aggressiveness:")));

    m_chkSmoothPressure = new QCheckBox("", optionWidget);
    m_chkSmoothPressure->setChecked(m_smoothingOptions.smoothPressure);
    connect(m_chkSmoothPressure, SIGNAL(toggled(bool)), this, SLOT(setSmoothPressure(bool)));
    addOptionWidgetOption(m_chkSmoothPressure, new QLabel(i18n("Smooth Pressure")));

    slotSetSmoothingType(1);

    // Drawing assistant configuration
    m_chkAssistant = new QCheckBox(i18n("Assistant:"), optionWidget);
    m_chkAssistant->setToolTip(i18n("You need to add Ruler Assistants before this tool will work."));
    connect(m_chkAssistant, SIGNAL(toggled(bool)), this, SLOT(setAssistant(bool)));
    m_sliderMagnetism = new KisSliderSpinBox(optionWidget);
    m_sliderMagnetism->setToolTip(i18n("Assistant Magnetism"));
    m_sliderMagnetism->setRange(0, MAXIMUM_MAGNETISM);
    m_sliderMagnetism->setEnabled(false);
    connect(m_chkAssistant, SIGNAL(toggled(bool)), m_sliderMagnetism, SLOT(setEnabled(bool)));
    m_sliderMagnetism->setValue(m_magnetism * MAXIMUM_MAGNETISM);
    connect(m_sliderMagnetism, SIGNAL(valueChanged(int)), SLOT(slotSetMagnetism(int)));

    addOptionWidgetOption(m_sliderMagnetism, m_chkAssistant);

    return optionWidget;
}

#include "kis_tool_brush.moc"

