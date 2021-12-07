/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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

    void setView(KisViewManager *view) override;
    void setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface);

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
