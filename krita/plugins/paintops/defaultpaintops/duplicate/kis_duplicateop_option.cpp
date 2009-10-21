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
#include "kis_duplicateop_option.h"
#include <klocale.h>

#include <QWidget>
#include <QRadioButton>

#include "ui_wdgduplicateop.h"
#include <kis_image.h>
#include <kis_perspective_grid.h>

class KisDuplicateOpOptionsWidget: public QWidget, public Ui::DuplicateOpOptionsWidget
{
public:
    KisDuplicateOpOptionsWidget(QWidget *parent = 0)
            : QWidget(parent) {
        setupUi(this);
    }
    KisImageWSP m_image;
protected:
    void showEvent(QShowEvent* event) {
        QWidget::showEvent(event);
        cbPerspective->setEnabled(m_image && m_image->perspectiveGrid() && m_image->perspectiveGrid()->countSubGrids() == 1);
    }
};


KisDuplicateOpOption::KisDuplicateOpOption()
        : KisPaintOpOption(i18n("Painting Mode"), false)
{
    m_checkable = false;
    m_optionWidget = new KisDuplicateOpOptionsWidget();
    connect(m_optionWidget->cbHealing, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_optionWidget->cbPerspective, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));

    setConfigurationPage(m_optionWidget);
}


KisDuplicateOpOption::~KisDuplicateOpOption()
{
}

bool KisDuplicateOpOption::healing() const
{
    return m_optionWidget->cbHealing->isChecked();
}

void KisDuplicateOpOption::setHealing(bool healing)
{
    m_optionWidget->cbHealing->setChecked(healing);
}

bool KisDuplicateOpOption::correctPerspective() const
{
    return m_optionWidget->cbPerspective->isChecked();
}

void KisDuplicateOpOption::setPerspective(bool perspective)
{
    m_optionWidget->cbPerspective->setChecked(perspective);
}

void KisDuplicateOpOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty("Duplicateop/Healing", healing());
    setting->setProperty("Duplicateop/CorrectPerspective", correctPerspective());
}

void KisDuplicateOpOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    m_optionWidget->cbHealing->setChecked(setting->getBool("Duplicateop/Healing", false));
    m_optionWidget->cbPerspective->setChecked(setting->getBool("Duplicateop/CorrectPerspective", false));
    emit sigSettingChanged();
}

void KisDuplicateOpOption::setImage(KisImageWSP image)
{
    m_optionWidget->m_image = image;
}
