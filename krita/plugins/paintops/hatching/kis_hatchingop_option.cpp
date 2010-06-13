/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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
#include "kis_hatchingop_option.h"

#include "ui_newhatchingoptions.h"

class KisHatchingOpOptionsWidget: public QWidget, public Ui::NewHatchingOptions
{
public:
    KisHatchingOpOptionsWidget(QWidget *parent = 0)
            : QWidget(parent) {
        setupUi(this);
    }
};

KisHatchingOpOption::KisHatchingOpOption()
        : KisPaintOpOption(i18n("Brush size"), KisPaintOpOption::brushCategory(), false)
{
    m_checkable = false;
    m_options = new KisHatchingOpOptionsWidget();
    connect(m_options->w_angle, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->w_w, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->w_h, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->w_s, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->w_thickness, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
   
    setConfigurationPage(m_options);
}

KisHatchingOpOption::~KisHatchingOpOption()
{
    // delete m_options;
}


void KisHatchingOpOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty(HATCHING_ANGLE, m_options->w_angle->value());
    setting->setProperty(HATCHING_WIDTH, m_options->w_w->value());
    setting->setProperty(HATCHING_HEIGHT, m_options->w_h->value());
    setting->setProperty(HATCHING_SEPARATION, m_options->w_s->value());
    setting->setProperty(HATCHING_THICKNESS, m_options->w_thickness->value());
}

void KisHatchingOpOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    m_options->w_angle->setValue( setting->getDouble(HATCHING_ANGLE) );
    m_options->w_w->setValue( setting->getInt(HATCHING_WIDTH) ); 
    m_options->w_h->setValue( setting->getInt(HATCHING_HEIGHT) );
    m_options->w_s->setValue( setting->getDouble(HATCHING_SEPARATION)); 
    m_options->w_thickness->setValue( setting->getDouble(HATCHING_THICKNESS) ); 
}


