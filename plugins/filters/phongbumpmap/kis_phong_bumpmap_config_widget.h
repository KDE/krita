/*
 *  Copyright (c) 2010-2011 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
