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
#ifndef _KIS_MERGED_PAINT_OPTION_H_
#define _KIS_MERGED_PAINT_OPTION_H_

#include <kis_paintop_option.h>
#include <krita_export.h>
#include <klocale.h>

#include <QLabel>

class PAINTOP_EXPORT KisMergetPaintOption : public KisPaintOpOption
{
public:
    KisMergetPaintOption():
        KisPaintOpOption(i18n("Merged Paint"), KisPaintOpOption::brushCategory(), false) { }

    virtual bool isCheckable() { return true; }
    
    virtual void writeOptionSetting(KisPropertiesConfiguration* setting) const {
        setting->setProperty("MergedPaint", isChecked());
    }
    
    virtual void readOptionSetting(const KisPropertiesConfiguration* setting) {
        bool enabled = setting->getBool("MergedPaint");
        setChecked(enabled);
    }
};

class PAINTOP_EXPORT KisMergetPaintOptionWidget: public KisMergetPaintOption
{
public:
    KisMergetPaintOptionWidget()
    {
        QLabel* lable = new QLabel(i18n("Use all visible layers merged together for painting operations."));
        lable->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);
        setConfigurationPage(lable);
    }
};

#endif // _KIS_MERGED_PAINT_OPTION_H_
