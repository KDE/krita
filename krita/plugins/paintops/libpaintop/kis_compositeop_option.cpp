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

#include <klocale.h>
#include <KoIcon.h>

#include <kis_cmb_composite.h>
#include <KoCompositeOp.h>
#include <kis_types.h>
#include <KoID.h>
#include <ui_wdgCompositeOpOption.h>
#include <kis_composite_ops_model.h>

KisCompositeOpOption::KisCompositeOpOption(bool createConfigWidget):
    KisPaintOpOption(i18n("Blending Mode"), KisPaintOpOption::brushCategory(), true),
    m_createConfigWidget(createConfigWidget)
{
    m_checkable         = false;
    m_prevCompositeOpID = KoCompositeOpRegistry::instance().getDefaultCompositeOp().id();
    m_currCompositeOpID = m_prevCompositeOpID;
    
    if(createConfigWidget) {
        QWidget* widget = new QWidget();
        
        Ui_wdgCompositeOpOption ui;
        ui.setupUi(widget);
        ui.bnEraser->setIcon(koIcon("draw-eraser"));
        
        m_label    = ui.lbChoosenMode;
        m_list     = ui.list;
        m_bnEraser = ui.bnEraser;
        
        setConfigurationPage(widget);
        
        connect(ui.list    , SIGNAL(activated(const QModelIndex&)), this, SLOT(slotCompositeOpChanged(const QModelIndex&)));
        connect(ui.bnEraser, SIGNAL(toggled(bool))                , this, SLOT(slotEraserToggled(bool)));
    }
}

KisCompositeOpOption::~KisCompositeOpOption()
{
}

void KisCompositeOpOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty("CompositeOp", m_currCompositeOpID);
}

void KisCompositeOpOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    QString ompositeOpID = setting->getString("CompositeOp", KoCompositeOpRegistry::instance().getDefaultCompositeOp().id());
    KoID    compositeOp = KoCompositeOpRegistry::instance().getKoID(ompositeOpID);
    changeCompositeOp(compositeOp);
}

void KisCompositeOpOption::changeCompositeOp(const KoID& compositeOp)
{
    if(compositeOp.id() == m_currCompositeOpID)
        return;
    
    m_prevCompositeOpID = m_currCompositeOpID;
    m_currCompositeOpID = compositeOp.id();
    
    if(m_createConfigWidget) {
        m_label->setText(compositeOp.name());
        m_bnEraser->blockSignals(true);
        m_bnEraser->setChecked(m_currCompositeOpID == "erase");
        m_bnEraser->blockSignals(false);
    }
    
    emit sigSettingChanged();
}


void KisCompositeOpOption::slotCompositeOpChanged(const QModelIndex& index)
{
    KoID compositeOp;
    
    if(m_list->entryAt(compositeOp, index.row()))
        changeCompositeOp(compositeOp);
}

void KisCompositeOpOption::slotEraserToggled(bool toggled)
{
    if(toggled)
        changeCompositeOp(KoCompositeOpRegistry::instance().getKoID("erase"));
    else
        changeCompositeOp(KoCompositeOpRegistry::instance().getKoID(m_prevCompositeOpID));
}
