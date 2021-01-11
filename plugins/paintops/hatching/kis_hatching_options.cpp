/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
 *  Copyright (c) 2010 José Luis Vergara <pentalis@gmail.com>
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

#include "kis_hatching_options.h"
#include <brushengine/kis_paintop_lod_limitations.h>

#include "ui_wdghatchingoptions.h"

class KisHatchingOptionsWidget: public QWidget, public Ui::WdgHatchingOptions
{
public:
    KisHatchingOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);

        QString degree = QChar(Qt::Key_degree);
        QString px = i18n(" px");

        angleKisAngleSelector           ->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);

        //setRange(minimum, maximum, decimals)
        angleKisAngleSelector           -> setRange(-90.0, 90.0);
        angleKisAngleSelector           -> setDecimals(1);
        separationKisDoubleSliderSpinBox-> setRange(1.0, 30.0, 1);
        thicknessKisDoubleSliderSpinBox -> setRange(1.0, 30.0, 1);
        originXKisDoubleSliderSpinBox   -> setRange(-300, 300, 0);
        originYKisDoubleSliderSpinBox   -> setRange(-300, 300, 0);

        angleKisAngleSelector           -> setAngle(-60);
        separationKisDoubleSliderSpinBox-> setValue(6);
        thicknessKisDoubleSliderSpinBox -> setValue(1);
        originXKisDoubleSliderSpinBox   -> setValue(50);
        originYKisDoubleSliderSpinBox   -> setValue(50);

        separationKisDoubleSliderSpinBox-> setSuffix(px);
        thicknessKisDoubleSliderSpinBox -> setSuffix(px);
        originXKisDoubleSliderSpinBox   -> setSuffix(px);
        originYKisDoubleSliderSpinBox   -> setSuffix(px);
    }
};

KisHatchingOptions::KisHatchingOptions()
    : KisPaintOpOption(KisPaintOpOption::GENERAL, false)
{
    setObjectName("KisHatchingOptions");

    m_checkable = false;
    m_options = new KisHatchingOptionsWidget();

    connect(m_options->angleKisAngleSelector, SIGNAL(angleChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->separationKisDoubleSliderSpinBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->thicknessKisDoubleSliderSpinBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->originXKisDoubleSliderSpinBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->originYKisDoubleSliderSpinBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));

    connect(m_options->noCrosshatchingRadioButton, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->perpendicularRadioButton, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->minusThenPlusRadioButton, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->plusThenMinusRadioButton, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->moirePatternRadioButton, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));

    connect(m_options->separationIntervalSpinBox, SIGNAL(valueChanged(int)), SLOT(emitSettingChanged()));

    setConfigurationPage(m_options);
}

KisHatchingOptions::~KisHatchingOptions()
{
}

void KisHatchingOptions::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    HatchingOption op;

    op.angle = m_options->angleKisAngleSelector->angle();
    op.separation = m_options->separationKisDoubleSliderSpinBox->value();
    op.thickness = m_options->thicknessKisDoubleSliderSpinBox->value();
    op.origin_x = m_options->originXKisDoubleSliderSpinBox->value();
    op.origin_y = m_options->originYKisDoubleSliderSpinBox->value();

    op.bool_nocrosshatching = m_options->noCrosshatchingRadioButton->isChecked();
    op.bool_perpendicular = m_options->perpendicularRadioButton->isChecked();
    op.bool_minusthenplus = m_options->minusThenPlusRadioButton->isChecked();
    op.bool_plusthenminus = m_options->plusThenMinusRadioButton->isChecked();
    op.bool_moirepattern = m_options->moirePatternRadioButton->isChecked();

    op.separationintervals = m_options->separationIntervalSpinBox->value();

    op.writeOptionSetting(setting);
}

void KisHatchingOptions::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    HatchingOption op;
    op.readOptionSetting(setting);

    m_options->angleKisAngleSelector->setAngle(op.angle);
    m_options->separationKisDoubleSliderSpinBox->setValue(op.separation);
    m_options->thicknessKisDoubleSliderSpinBox->setValue(op.thickness);
    m_options->originXKisDoubleSliderSpinBox->setValue(op.origin_x);
    m_options->originYKisDoubleSliderSpinBox->setValue(op.origin_y);

    m_options->noCrosshatchingRadioButton->setChecked(op.bool_nocrosshatching);
    m_options->perpendicularRadioButton->setChecked(op.bool_perpendicular);
    m_options->minusThenPlusRadioButton->setChecked(op.bool_minusthenplus);
    m_options->plusThenMinusRadioButton->setChecked(op.bool_plusthenminus);
    m_options->moirePatternRadioButton->setChecked(op.bool_moirepattern);

    m_options->separationIntervalSpinBox->setValue(op.separationintervals);
}

void KisHatchingOptions::lodLimitations(KisPaintopLodLimitations *l) const
{
    l->limitations << KoID("hatching-brush", i18nc("PaintOp instant preview limitation", "Hatching Brush (heavy aliasing in preview mode)"));
}

