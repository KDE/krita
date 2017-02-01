/*
 *  Copyright (c) 2009-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_dynaop_option.h"
#include <klocalizedstring.h>
#include <kis_config.h>
#include <brushengine/kis_paintop_lod_limitations.h>

#include "ui_wdgdynaoptions.h"

class KisDynaOpOptionsWidget: public QWidget, public Ui::WdgDynaOptions
{
public:
    KisDynaOpOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);
        angleSlider->setRange(0, 360, 0);
        angleSlider->setValue(0);
        angleSlider->setSingleStep(1);
        angleSlider->setSuffix(QChar(Qt::Key_degree));

        diameterDSSB->setRange(0, KisConfig().readEntry("maximumBrushSize", 1000), 0);
        diameterDSSB->setValue(20);
        diameterDSSB->setExponentRatio(3.0);

    }
};

KisDynaOpOption::KisDynaOpOption()
    : KisPaintOpOption(KisPaintOpOption::GENERAL, false)
{
    setObjectName("KisDynaOpOption");

    m_checkable = false;
    m_options = new KisDynaOpOptionsWidget();

    //ui
    connect(m_options->fixedAngleChBox, SIGNAL(toggled(bool)), m_options->angleSlider, SLOT(setEnabled(bool)));

    // preset
    connect(m_options->circleRBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->polygonRBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->wireRBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->linesRBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->initWidthSPBox, SIGNAL(valueChanged(double)), SLOT(emitSettingChanged()));
    connect(m_options->massSPBox, SIGNAL(valueChanged(double)), SLOT(emitSettingChanged()));
    connect(m_options->dragSPBox, SIGNAL(valueChanged(double)), SLOT(emitSettingChanged()));
    connect(m_options->angleSlider, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->widthRangeSPBox, SIGNAL(valueChanged(double)), SLOT(emitSettingChanged()));
    connect(m_options->diameterDSSB, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->lineCountSPBox, SIGNAL(valueChanged(int)), SLOT(emitSettingChanged()));
    connect(m_options->lineSpacingSPBox, SIGNAL(valueChanged(double)), SLOT(emitSettingChanged()));
    connect(m_options->LineCBox, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->twoCBox, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->fixedAngleChBox, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));

    setConfigurationPage(m_options);
}

KisDynaOpOption::~KisDynaOpOption()
{
    delete m_options;
}

qreal KisDynaOpOption::initWidth() const
{
    return m_options->initWidthSPBox->value();
}

qreal KisDynaOpOption::mass() const
{
    return m_options->massSPBox->value();
}

qreal KisDynaOpOption::drag() const
{
    return m_options->dragSPBox->value();
}

bool KisDynaOpOption::useFixedAngle() const
{
    return m_options->fixedAngleChBox->isChecked();
}


qreal KisDynaOpOption::widthRange() const
{
    return m_options->widthRangeSPBox->value();
}


int KisDynaOpOption::action() const
{
    if (m_options->circleRBox->isChecked())
        return 0;
    if (m_options->polygonRBox->isChecked())
        return 1;
    if (m_options->wireRBox->isChecked())
        return 2;
    if (m_options->linesRBox->isChecked())
        return 3;
    return 0;
}


bool KisDynaOpOption::enableLine() const
{
    return m_options->LineCBox->isChecked();
}

bool KisDynaOpOption::useTwoCircles() const
{
    return m_options->twoCBox->isChecked();
}

int KisDynaOpOption::lineCount() const
{
    return m_options->lineCountSPBox->value();
}

qreal KisDynaOpOption::lineSpacing() const
{
    return m_options->lineSpacingSPBox->value();
}

void KisDynaOpOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    DynaOption op;

    op.dyna_width = initWidth();
    op.dyna_mass  = mass();
    op.dyna_drag  = drag();
    op.dyna_use_fixed_angle  = useFixedAngle();
    op.dyna_angle  = m_options->angleSlider->value();
    op.dyna_width_range  = widthRange();
    op.dyna_action  = action();
    op.dyna_diameter  = m_options->diameterDSSB->value();
    op.dyna_enable_line  = enableLine();
    op.dyna_use_two_circles  = useTwoCircles();
    op.dyna_line_count  = lineCount();
    op.dyna_line_spacing  = lineSpacing();

    op.writeOptionSetting(setting);
}

void KisDynaOpOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    DynaOption op;
    op.readOptionSetting(setting);

    switch (op.dyna_action) {
    case 0: m_options->circleRBox->setChecked(true); break;
    case 1: m_options->polygonRBox->setChecked(true); break;
    case 2: m_options->wireRBox->setChecked(true); break;
    case 3: m_options->linesRBox->setChecked(true); break;
    default: break;
    }

    m_options->initWidthSPBox->setValue(op.dyna_width);
    m_options->massSPBox->setValue(op.dyna_mass);
    m_options->dragSPBox->setValue(op.dyna_drag);
    m_options->angleSlider->setValue(op.dyna_angle);
    m_options->widthRangeSPBox->setValue(op.dyna_width_range);
    m_options->diameterDSSB->setValue(op.dyna_diameter);
    m_options->lineCountSPBox->setValue(op.dyna_line_count);
    m_options->lineSpacingSPBox->setValue(op.dyna_line_spacing);
    m_options->LineCBox->setChecked(op.dyna_enable_line);
    m_options->twoCBox->setChecked(op.dyna_use_two_circles);
    m_options->fixedAngleChBox->setChecked(op.dyna_use_fixed_angle);
}

void KisDynaOpOption::lodLimitations(KisPaintopLodLimitations *l) const
{
    l->blockers << KoID("dyna-brush", i18nc("PaintOp instant preview limitation", "Dyna Brush (not supported)"));
}
