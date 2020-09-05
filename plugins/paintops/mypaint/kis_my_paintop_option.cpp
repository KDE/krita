/*
 * Copyright (c) 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <QWidget>
#include "ui_wdgmypaintoptions.h"
#include "kis_my_paintop_option.h"
#include <kis_paintop_lod_limitations.h>

class KisMyPaintOpOptionsWidget: public QWidget, public Ui::WdgMyPaintOptions
{
public:
    KisMyPaintOpOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {

        setupUi(this);
    }
};

KisMyPaintOpOption::KisMyPaintOpOption()
    : KisPaintOpOption(KisPaintOpOption::GENERAL, false)
{
    setObjectName("KisMyPaintOpOption");

    m_checkable = false;
    m_options = new KisMyPaintOpOptionsWidget();    

    m_options->radiusSPBox->setRange(0.01, 7.0, 2);
    m_options->radiusSPBox->setValue(radius());    

    m_options->hardnessSPBox->setRange(0.02, 1.0, 2);
    m_options->hardnessSPBox->setValue(hardness());

    m_options->opacitySPBox->setRange(0.0, 1.0, 2);
    m_options->opacitySPBox->setValue(opacity());

    m_options->eraserBox->setChecked(eraser());

    connect(m_options->eraserBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));

    setConfigurationPage(m_options);
}

KisMyPaintOpOption::~KisMyPaintOpOption()
{
    delete m_options;
}

void KisMyPaintOpOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{        
    KisMyPaintOptionProperties op;

    op.diameter = 2*exp(m_options->radiusSPBox->value());
    op.hardness = m_options->hardnessSPBox->value();
    op.opacity = m_options->opacitySPBox->value();
    op.eraserMode = m_options->eraserBox->isChecked();
    op.json = this->json;
    op.eraser = this->eraserVal;

    op.writeOptionSetting(setting);
}

void KisMyPaintOpOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{        
    KisMyPaintOptionProperties op;
    op.readOptionSetting(setting);

    if(!setting->getProperty(MYPAINT_JSON).isNull())
        this->json = setting->getProperty(MYPAINT_JSON).toByteArray();

    m_options->radiusSPBox->setValue(op.radius());
    m_options->hardnessSPBox->setValue(op.hardness);
    m_options->opacitySPBox->setValue(op.opacity);
    m_options->eraserBox->setChecked(op.eraserMode);
    this->eraserVal = op.eraser;
}

void KisMyPaintOpOption::lodLimitations(KisPaintopLodLimitations *l) const {

   Q_UNUSED(l);
}

void KisMyPaintOpOption::refresh() {

    emitSettingChanged();
}

void KisMyPaintOpOption::setEraser(bool isEraser) const
{
    m_options->eraserBox->setChecked(isEraser);
}

bool KisMyPaintOpOption::eraser() const
{
    return m_options->eraserBox->isChecked();
}

void KisMyPaintOpOption::setOpacity(int opacity) const
{
    m_options->opacitySPBox->setValue(opacity);
}

int KisMyPaintOpOption::opacity() const
{
    return m_options->opacitySPBox->value();
}


void KisMyPaintOpOption::setHardness(int hardness) const
{
    m_options->hardnessSPBox->setValue(hardness);
}

int KisMyPaintOpOption::hardness() const
{
    return m_options->hardnessSPBox->value();
}

void KisMyPaintOpOption::setRadius(int radius) const
{
    m_options->radiusSPBox->setValue(radius);
}

int KisMyPaintOpOption::radius() const
{
    return m_options->radiusSPBox->value();
}

KisDoubleSliderSpinBox* KisMyPaintOpOption::radiusSlider() {

    return m_options->radiusSPBox;
}

KisDoubleSliderSpinBox* KisMyPaintOpOption::hardnessSlider() {

    return m_options->hardnessSPBox;
}

KisDoubleSliderSpinBox* KisMyPaintOpOption::opacitySlider() {

    return m_options->opacitySPBox;
}
