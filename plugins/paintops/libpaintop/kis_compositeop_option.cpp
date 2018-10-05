/*
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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
#include "kis_compositeop_option.h"

#include <klocalizedstring.h>
#include <kis_icon.h>

#include <kis_cmb_composite.h>
#include <KoCompositeOpRegistry.h>
#include <kis_types.h>
#include <KoID.h>
#include <ui_wdgCompositeOpOption.h>
#include <kis_composite_ops_model.h>
#include "kis_signals_blocker.h"

KisCompositeOpOption::KisCompositeOpOption(bool createConfigWidget):
    KisPaintOpOption(KisPaintOpOption::GENERAL, true),
    m_createConfigWidget(createConfigWidget),
    m_eraserMode(false)
{
    m_checkable         = false;
    m_currCompositeOpID = KoCompositeOpRegistry::instance().getDefaultCompositeOp().id();

    if (createConfigWidget) {
        QWidget* widget = new QWidget();

        Ui_wdgCompositeOpOption ui;
        ui.setupUi(widget);
        ui.bnEraser->setIcon(KisIconUtils::loadIcon("draw-eraser"));

        m_label    = ui.lbChoosenMode;
        m_list     = ui.list;
        m_bnEraser = ui.bnEraser;

        setConfigurationPage(widget);

        connect(ui.list    , SIGNAL(clicked(QModelIndex)), this, SLOT(slotCompositeOpChanged(QModelIndex)));
        connect(ui.bnEraser, SIGNAL(toggled(bool))                , this, SLOT(slotEraserToggled(bool)));
    }

    setObjectName("KisCompositeOpOption");

}

KisCompositeOpOption::~KisCompositeOpOption()
{
}

void KisCompositeOpOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    setting->setProperty("CompositeOp", m_currCompositeOpID);
    setting->setProperty("EraserMode", m_eraserMode);
}

void KisCompositeOpOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    QString ompositeOpID = setting->getString("CompositeOp", KoCompositeOpRegistry::instance().getDefaultCompositeOp().id());
    KoID    compositeOp = KoCompositeOpRegistry::instance().getKoID(ompositeOpID);
    changeCompositeOp(compositeOp);

    const bool eraserMode = setting->getBool("EraserMode", false);
    slotEraserToggled(eraserMode);
}

void KisCompositeOpOption::changeCompositeOp(const KoID& compositeOp)
{
    if (compositeOp.id() == m_currCompositeOpID)
        return;

    m_currCompositeOpID = compositeOp.id();

    if (m_createConfigWidget) {
        m_label->setText(compositeOp.name());
    }

    emitSettingChanged();
}


void KisCompositeOpOption::slotCompositeOpChanged(const QModelIndex& index)
{
    Q_UNUSED(index);

    KoID compositeOp = m_list->selectedCompositeOp();
    changeCompositeOp(compositeOp);
}

void KisCompositeOpOption::slotEraserToggled(bool toggled)
{
    if (m_bnEraser->isChecked() != toggled) {
        KisSignalsBlocker b(m_bnEraser);
        m_bnEraser->setChecked(toggled);
    }

    m_eraserMode = toggled;

    emitSettingChanged();
}
