/*
 * KDE. Krita Project.
 *
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_HALFTONE_CONFIG_WIDGET_H
#define KIS_HALFTONE_CONFIG_WIDGET_H

#include <kis_config_widget.h>
#include <kis_paint_device.h>

#include <QVector>
#include <QList>

#include "ui_KisHalftoneConfigWidget.h"

class KoChannelInfo;

class KisHalftoneConfigPageWidget;

class KisHalftoneConfigWidget : public KisConfigWidget
{
    Q_OBJECT
public:
    KisHalftoneConfigWidget(QWidget *parent, const KisPaintDeviceSP dev);
    ~KisHalftoneConfigWidget() override;

    KisPropertiesConfigurationSP configuration() const override;
    void setConfiguration(const KisPropertiesConfigurationSP config) override;

private:
    Ui_HalftoneConfigWidget m_ui;
    const KisPaintDeviceSP m_paintDevice;
    QList<KoChannelInfo *> m_channelsInfo;
    QString m_colorModelId;
    KisHalftoneConfigPageWidget *m_intensityWidget;
    QVector<KisHalftoneConfigPageWidget*> m_channelWidgets;
    
private Q_SLOTS:
    void slot_comboBoxMode_currentIndexChanged(int index);
};

#endif
