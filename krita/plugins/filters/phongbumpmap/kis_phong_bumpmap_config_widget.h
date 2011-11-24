/*
 *  Copyright (c) 2010-2011 José Luis Vergara <pentalis@gmail.com>
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

#ifndef KIS_PHONG_BUMPMAP_CONFIG_WIDGET_H
#define KIS_PHONG_BUMPMAP_CONFIG_WIDGET_H

#include "ui_wdgphongbumpmap.h"
#include "kis_paint_device.h"
#include "kis_config_widget.h"
#include "kis_image.h"

class KisNodeModel;

class KisPhongBumpmapWidget : public QWidget, public Ui::WdgPhongBumpmap
{
    Q_OBJECT

public:
    KisPhongBumpmapWidget(QWidget *parent) : QWidget(parent)
    {
        setupUi(this);
        
        ambientReflectivityKisDoubleSliderSpinBox  -> setRange(0, 1, 2);
        diffuseReflectivityKisDoubleSliderSpinBox  -> setRange(0, 1, 2);
        specularReflectivityKisDoubleSliderSpinBox -> setRange(0, 1, 2);
        shinynessExponentKisSliderSpinBox          -> setRange(1, 200);
        
        ambientReflectivityKisDoubleSliderSpinBox  -> setValue(0.1);
        diffuseReflectivityKisDoubleSliderSpinBox  -> setValue(0.5);
        specularReflectivityKisDoubleSliderSpinBox -> setValue(0.5);
        shinynessExponentKisSliderSpinBox          -> setValue(40);
    }
};

class KisPhongBumpmapConfigWidget : public KisConfigWidget
{
    Q_OBJECT

public:
    KisPhongBumpmapConfigWidget(const KisPaintDeviceSP dev, const KisImageWSP image, QWidget *parent, Qt::WFlags f = 0);
    virtual ~KisPhongBumpmapConfigWidget() {}
    void setConfiguration(const KisPropertiesConfiguration *config);
    KisPropertiesConfiguration *configuration() const;
    KisPhongBumpmapWidget *m_page;

private:
    KisPaintDeviceSP m_device;
    KisImageWSP m_image;
};

#endif  //KIS_PHONG_BUMPMAP_CONFIG_WIDGET_H
