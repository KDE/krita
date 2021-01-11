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

        azimuthAngleSelector1->setDecimals(0);
        azimuthAngleSelector1->setRange(0, 359);
        azimuthAngleSelector1->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_MenuButton);
        azimuthAngleSelector2->setDecimals(0);
        azimuthAngleSelector2->setRange(0, 359);
        azimuthAngleSelector2->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_MenuButton);
        azimuthAngleSelector3->setDecimals(0);
        azimuthAngleSelector3->setRange(0, 359);
        azimuthAngleSelector3->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_MenuButton);
        azimuthAngleSelector4->setDecimals(0);
        azimuthAngleSelector4->setRange(0, 359);
        azimuthAngleSelector4->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_MenuButton);

        elevationAngleSelector1->setDecimals(0);
        elevationAngleSelector1->setRange(0, 90);
        elevationAngleSelector1->setWrapping(false);
        elevationAngleSelector1->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_NoFlipOptions);
        elevationAngleSelector2->setDecimals(0);
        elevationAngleSelector2->setRange(0, 90);
        elevationAngleSelector2->setWrapping(false);
        elevationAngleSelector2->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_NoFlipOptions);
        elevationAngleSelector3->setDecimals(0);
        elevationAngleSelector3->setRange(0, 90);
        elevationAngleSelector3->setWrapping(false);
        elevationAngleSelector3->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_NoFlipOptions);
        elevationAngleSelector4->setDecimals(0);
        elevationAngleSelector4->setRange(0, 90);
        elevationAngleSelector4->setWrapping(false);
        elevationAngleSelector4->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_NoFlipOptions);
    }
};

class KisPhongBumpmapConfigWidget : public KisConfigWidget
{
    Q_OBJECT

public:
    KisPhongBumpmapConfigWidget(const KisPaintDeviceSP dev, QWidget *parent, Qt::WindowFlags f = Qt::WindowFlags());
    ~KisPhongBumpmapConfigWidget() override {}
    void setConfiguration(const KisPropertiesConfigurationSP config) override;
    KisPropertiesConfigurationSP configuration() const override;
    KisPhongBumpmapWidget *m_page;

private:
    KisPaintDeviceSP m_device;
private Q_SLOTS:
    void slotDisableHeightChannelCombobox(bool normalmapchecked);
};

#endif  //KIS_PHONG_BUMPMAP_CONFIG_WIDGET_H
