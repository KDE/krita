/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_paint_action_type_option.h"
#include <klocale.h>

#include <QWidget>
#include <QRadioButton>

#include "ui_wdgincremental.h"

class KisPaintActionWidget: public QWidget, public Ui::WdgIncremental
{
public:
    KisPaintActionWidget(QWidget *parent = 0)
            : QWidget(parent) {
        setupUi(this);
    }
};


KisPaintActionTypeOption::KisPaintActionTypeOption()
        : KisPaintOpOption(i18n("Painting Mode"), false)
{
    m_checkable = false;
    m_optionWidget = new KisPaintActionWidget();
    connect(m_optionWidget->radioBuildup, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_optionWidget->radioWash, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));

    m_optionWidget->hide();
    setConfigurationPage(m_optionWidget);
}


KisPaintActionTypeOption::~KisPaintActionTypeOption()
{
}


enumPaintActionType KisPaintActionTypeOption::paintActionType() const
{
    if (m_optionWidget->radioBuildup->isChecked()) {
        return BUILDUP;
    } else if (m_optionWidget->radioWash->isChecked()) {
        return WASH;
    } else {
        return WASH;
    }
}

void KisPaintActionTypeOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty("PaintOpAction", paintActionType());
}

void KisPaintActionTypeOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    enumPaintActionType type = (enumPaintActionType)setting->getInt("PaintOpAction", WASH);
    m_optionWidget->radioBuildup->setChecked(type == BUILDUP);
    m_optionWidget->radioWash->setChecked(type == WASH);
}

