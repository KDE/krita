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
#include <kis_categorized_list_view.h>
#include <KoCompositeOp.h>
#include <kis_types.h>
#include <KoID.h>

KisCompositeOpOption::KisCompositeOpOption(bool creatConfigWidget):
    KisPaintOpOption(i18n("Blending Mode"), KisPaintOpOption::brushCategory(), true)
{
    m_checkable = false;
    
    if(creatConfigWidget) {
        KisCategorizedListModel<QString>* model = new KisCategorizedListModel<QString>();
        KisCategorizedListView*           view  = new KisCategorizedListView();
        
        const KoIDList& categories = KoCompositeOpRegistry::instance().getCategories();
        
        for(KoIDList::const_iterator cat=categories.begin(); cat!=categories.end(); ++cat) {
            const KoIDList& modes = KoCompositeOpRegistry::instance().getCompositeOps(cat->id());
            
            for(KoIDList::const_iterator mod=modes.begin(); mod!=modes.end(); ++mod)
                model->addItem(cat->name(), mod->name());
        }
        
        view->setModel(model);
        setConfigurationPage(view);
    }
}


void KisCompositeOpOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
//     setting->setProperty(AIRBRUSH_ENABLED, isChecked());
//     setting->setProperty(AIRBRUSH_RATE, m_optionWidget->sliderRate->value());
}

void KisCompositeOpOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
//     setChecked(setting->getBool(AIRBRUSH_ENABLED));
//     m_optionWidget->sliderRate->setValue(setting->getInt(AIRBRUSH_RATE,100));
}

