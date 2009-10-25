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
#include "kis_grid_shape_option.h"
#include <klocale.h>

#include "ui_wdgshapeoptions.h"

class KisShapeOptionsWidget: public QWidget, public Ui::WdgShapeOptions
{
public:
    KisShapeOptionsWidget(QWidget *parent = 0)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

KisGridShapeOption::KisGridShapeOption()
        : KisPaintOpOption(i18n("Particle type"), false)
{
    m_checkable = false;
    m_options = new KisShapeOptionsWidget();
    connect(m_options->shapeBtn,SIGNAL(toggled(bool)),SIGNAL( sigSettingChanged()));
    connect(m_options->particleBtn,SIGNAL(toggled(bool)),SIGNAL( sigSettingChanged()));
    connect(m_options->pixelBtn,SIGNAL(toggled(bool)),SIGNAL( sigSettingChanged()));
    connect(m_options->shapeBox,SIGNAL(currentIndexChanged(int)),SIGNAL( sigSettingChanged()));
    connect(m_options->widthSpin,SIGNAL(valueChanged(int)),SIGNAL( sigSettingChanged()));
    connect(m_options->heightSpin,SIGNAL(valueChanged(int)),SIGNAL( sigSettingChanged()));
    connect(m_options->jitterShape,SIGNAL(toggled(bool)),SIGNAL( sigSettingChanged()));
    connect(m_options->heightPro,SIGNAL(valueChanged(double)),SIGNAL( sigSettingChanged()));
    connect(m_options->widthPro,SIGNAL(valueChanged(double)),SIGNAL( sigSettingChanged()));
    connect(m_options->proportionalBox,SIGNAL(toggled(bool)),SIGNAL( sigSettingChanged()));
    connect(m_options->gaussBox,SIGNAL(toggled(bool)),SIGNAL( sigSettingChanged()));

// turn off those
//     connect(m_options->maxTreshSpin,SIGNAL(valueChanged(double)),SIGNAL( sigSettingChanged()));
//     connect(m_options->minTreshSpin,SIGNAL(toggled(bool)),SIGNAL( sigSettingChanged()));
//     connect(m_options->renderBox,SIGNAL(toggled(bool)),SIGNAL( sigSettingChanged()));

    setConfigurationPage(m_options);
}

KisGridShapeOption::~KisGridShapeOption()
{
    // delete m_options; 
}

int KisGridShapeOption::object() const {
    if (m_options->shapeBtn->isChecked())
        return 0;
    if (m_options->particleBtn->isChecked())
        return 1;
    if (m_options->pixelBtn->isChecked())
        return 2;
    return -1;
}

int KisGridShapeOption::shape() const {
    return m_options->shapeBox->currentIndex();
}

int KisGridShapeOption::width() const {
    return m_options->widthSpin->value();
}

int KisGridShapeOption::height() const {
    return m_options->heightSpin->value();
}

bool KisGridShapeOption::jitterShapeSize() const {
    return m_options->jitterShape->isChecked();
}

qreal KisGridShapeOption::heightPerc() const {
    return m_options->heightPro->value();
}

qreal KisGridShapeOption::widthPerc() const {
    return m_options->widthPro->value(); 
}

bool KisGridShapeOption::proportional() const {
    return m_options->proportionalBox->isChecked();
}



// TODO
void KisGridShapeOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
//     setting->setProperty( "Grid/diameter", diameter() );
}

// TODO
void KisGridShapeOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
/*    m_options->diameterSpinBox->setValue( setting->getInt("Grid/diameter") );
    m_options->coverageSpin->setValue( setting->getDouble("Grid/coverage") );
    m_options->jitterSizeBox->setChecked( setting->getBool("Grid/jitterSize") );*/
}


qreal KisGridShapeOption::maxTresh() const
{
    return m_options->maxTreshSpin->value();
}


qreal KisGridShapeOption::minTresh() const
{
    return m_options->minTreshSpin->value();
}

bool KisGridShapeOption::highRendering() const
{
    return m_options->renderBox->isChecked();
}


bool KisGridShapeOption::gaussian() const
{
    return m_options->gaussBox->isChecked();
}
