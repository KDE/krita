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
#include "kis_dynaop_option.h"
#include <klocale.h>

#include "ui_wdgdynaoptions.h"

class KisDynaOpOptionsWidget: public QWidget, public Ui::WdgDynaOptions
{
public:
    KisDynaOpOptionsWidget(QWidget *parent = 0)
            : QWidget(parent) {
        setupUi(this);
    }
};

KisDynaOpOption::KisDynaOpOption()
        : KisPaintOpOption(i18n("Brush size"), false)
{
    m_checkable = false;
    m_options = new KisDynaOpOptionsWidget();
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

qreal KisDynaOpOption::xAngle() const
{
    return m_options->xAngleSPBox->value();
}

qreal KisDynaOpOption::yAngle() const
{
    return m_options->yAngleSPBox->value();
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

int KisDynaOpOption::circleRadius() const
{
    return m_options->circleRadiusSPBox->value();
}

bool KisDynaOpOption::enableLine() const
{
    return m_options->LineCBox->isChecked();
}

bool KisDynaOpOption::twoCircles() const
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

void KisDynaOpOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    Q_UNUSED(setting);
}

void KisDynaOpOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    Q_UNUSED(setting);
}


