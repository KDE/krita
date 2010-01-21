/*
 * Copyright (c) 2009,2010 Lukáš Tvrdý (lukast.dev@gmail.com)
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
    connect(m_options->shapeCBox,SIGNAL(currentIndexChanged(int)),SIGNAL( sigSettingChanged()));
    setConfigurationPage(m_options);
}

KisGridShapeOption::~KisGridShapeOption()
{
    delete m_options; 
}


int KisGridShapeOption::shape() const {
    return m_options->shapeCBox->currentIndex();
}

void KisGridShapeOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty( GRIDSHAPE_SHAPE, shape() );
}


void KisGridShapeOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    m_options->shapeCBox->setCurrentIndex(setting->getInt(GRIDSHAPE_SHAPE));
}
