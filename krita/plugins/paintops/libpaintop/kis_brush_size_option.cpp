/*
 *  Copyright (c) 2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_brush_size_option.h"
#include <klocale.h>

#include <QWidget>
#include <QRadioButton>

#include "ui_wdgBrushSizeOptions.h"

class KisBrushSizeOptionsWidget: public QWidget, public Ui::WdgBrushSizeOptions
{
public:
    KisBrushSizeOptionsWidget(QWidget *parent = 0)
            : QWidget(parent) {
        setupUi(this);
    }

};


KisBrushSizeOption::KisBrushSizeOption()
        : KisPaintOpOption(i18n("Brush tip"), KisPaintOpOption::brushCategory(), false)
{
    m_checkable = false;
    m_options = new KisBrushSizeOptionsWidget();

    connect(m_options->diameter,SIGNAL(valueChanged(double)),SIGNAL( sigSettingChanged()));
    connect(m_options->scale,SIGNAL(valueChanged(double)),SIGNAL( sigSettingChanged()));
    connect(m_options->aspectBox, SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    connect(m_options->spacing,SIGNAL(valueChanged(double)),SIGNAL( sigSettingChanged()));
    connect(m_options->rotationBox, SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    connect(m_options->densityBox, SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    connect(m_options->jitterMove, SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    connect(m_options->jitterMove, SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    connect(m_options->jitterMoveBox, SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));

    connect(m_options->jitterMoveBox, SIGNAL(toggled(bool)), m_options->jitterMove, SLOT(setEnabled(bool)));
    setConfigurationPage(m_options);
}

KisBrushSizeOption::~KisBrushSizeOption()
{
    delete m_options;
}


int KisBrushSizeOption::diameter() const
{
        return qRound(m_options->diameter->value());
}


void KisBrushSizeOption::setDiameter(int diameter)
{
        m_options->diameter->setValue(diameter);
}

qreal KisBrushSizeOption::brushAspect() const
{
    return m_options->aspectBox->value();
}


void KisBrushSizeOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty( BRUSH_DIAMETER, m_options->diameter->value() );
    setting->setProperty( BRUSH_ASPECT, m_options->aspectBox->value() );
    setting->setProperty( BRUSH_ROTATION, m_options->rotationBox->value() );
    setting->setProperty( BRUSH_SCALE, m_options->scale->value() );
    setting->setProperty( BRUSH_SPACING, m_options->spacing->value() );
    setting->setProperty( BRUSH_DENSITY, m_options->densityBox->value() );
    setting->setProperty( BRUSH_JITTER_MOVEMENT, m_options->jitterMove->value());
    setting->setProperty( BRUSH_JITTER_MOVEMENT_ENABLED, m_options->jitterMoveBox->isChecked());
}

void KisBrushSizeOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    m_options->diameter->setValue( setting->getDouble(BRUSH_DIAMETER) );
    m_options->aspectBox->setValue( setting->getDouble(BRUSH_ASPECT) );
    m_options->rotationBox->setValue( setting->getDouble(BRUSH_ROTATION) );
    m_options->scale->setValue( setting->getDouble(BRUSH_SCALE) );
    m_options->spacing->setValue( setting->getDouble(BRUSH_SPACING) );
    m_options->densityBox->setValue( setting->getDouble(BRUSH_DENSITY) );
    m_options->jitterMove->setValue( setting->getDouble(BRUSH_JITTER_MOVEMENT) );
    m_options->jitterMoveBox->setChecked( setting->getBool(BRUSH_JITTER_MOVEMENT_ENABLED) );

}

void KisBrushSizeOption::setSpacing(qreal spacing)
{
    m_options->spacing->setValue(spacing);
}

qreal KisBrushSizeOption::spacing() const
{
    return m_options->spacing->value();
}

