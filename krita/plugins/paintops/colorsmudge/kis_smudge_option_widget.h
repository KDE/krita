/*
    Copyright 2012  Silvio Heinrich <plassy@web.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either 
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public 
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KIS_SMUDGE_OPTION_WIDGET_H
#define KIS_SMUDGE_OPTION_WIDGET_H

#include <kis_curve_option_widget.h>

class QComboBox;

class KisSmudgeOptionWidget: public KisCurveOptionWidget
{
    Q_OBJECT
    
public:
    KisSmudgeOptionWidget(const QString& label, const QString& sliderLabel, const QString& name, bool checked);
    
    virtual void readOptionSetting(const KisPropertiesConfiguration* setting);
    
private slots:
    void slotCurrentIndexChanged(int index);
    
private:
    QComboBox* mCbSmudgeMode;
};

#endif // KIS_SMUDGE_OPTION_WIDGET_H
