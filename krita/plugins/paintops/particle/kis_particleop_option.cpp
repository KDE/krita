/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_particleop_option.h"
#include <klocale.h>

#include <QWidget>
#include <QRadioButton>

#include "ui_wdgparticleoptions.h"

class KisParticleOpOptionsWidget: public QWidget, public Ui::WdgParticleOptions
{
public:
    KisParticleOpOptionsWidget(QWidget *parent = 0)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

KisParticleOpOption::KisParticleOpOption()
        : KisPaintOpOption(i18n("Brush size"), false)
{
    m_checkable = false;
    m_options = new KisParticleOpOptionsWidget();

    connect(m_options->particleSpinBox, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->itersSPBox,SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->gravSPBox, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->weightSPBox, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->dxSPBox, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->dySPBox, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));

    setConfigurationPage(m_options);
}

KisParticleOpOption::~KisParticleOpOption()
{
    delete m_options; 
}

int KisParticleOpOption::particleCount() const
{
    return m_options->particleSpinBox->value();
}

qreal KisParticleOpOption::weight() const
{
    return m_options->weightSPBox->value();
}


QPointF KisParticleOpOption::scale() const
{
    return QPointF(m_options->dxSPBox->value(),m_options->dySPBox->value());
}


int KisParticleOpOption::iterations() const
{
    return m_options->itersSPBox->value();
}



qreal KisParticleOpOption::gravity() const
{
    return m_options->gravSPBox->value();
}

void KisParticleOpOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty(PARTICLE_COUNT, particleCount());
    setting->setProperty(PARTICLE_ITERATIONS, iterations());
    setting->setProperty(PARTICLE_GRAVITY, gravity());
    setting->setProperty(PARTICLE_WEIGHT, weight());
    setting->setProperty(PARTICLE_SCALE_X, scale().x());
    setting->setProperty(PARTICLE_SCALE_Y, scale().y());
}

void KisParticleOpOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{

    m_options->particleSpinBox->setValue(setting->getInt(PARTICLE_COUNT));
    m_options->itersSPBox->setValue(setting->getInt(PARTICLE_ITERATIONS));
    m_options->gravSPBox->setValue(setting->getDouble(PARTICLE_GRAVITY));
    m_options->weightSPBox->setValue(setting->getDouble(PARTICLE_WEIGHT));
    m_options->dxSPBox->setValue(setting->getDouble(PARTICLE_SCALE_X));
    m_options->dySPBox->setValue(setting->getDouble(PARTICLE_SCALE_Y));
}
