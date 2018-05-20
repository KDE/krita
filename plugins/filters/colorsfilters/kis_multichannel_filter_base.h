/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_MULTICHANNEL_FILTER_BASE_H_
#define _KIS_MULTICHANNEL_FILTER_BASE_H_

#include <QPair>
#include <QList>

#include <filter/kis_color_transformation_filter.h>
#include <filter/kis_color_transformation_configuration.h>
#include <kis_config_widget.h>
#include <kis_paint_device.h>
#include "ui_wdg_perchannel.h"

#include "virtual_channel_info.h"

/**
 *
 */
class KisMultiChannelFilterConfigurationBase
        : public KisColorTransformationConfiguration
{
public:
    KisMultiChannelFilterConfigurationBase(int channelCount, const QString & name, qint32 version);
    ~KisMultiChannelFilterConfigurationBase() override;

    using KisFilterConfiguration::fromXML;
    using KisFilterConfiguration::toXML;
    using KisFilterConfiguration::fromLegacyXML;

    static QVector<VirtualChannelInfo> getVirtualChannels(const KoColorSpace *cs);

    void fromLegacyXML(const QDomElement& root) override;

    void fromXML(const QDomElement& e) override;
    void toXML(QDomDocument& doc, QDomElement& root) const override;

    void setCurves(QList<KisCubicCurve> &curves) override;
    bool isCompatible(const KisPaintDeviceSP) const override;

    const QVector<QVector<quint16> >& transfers() const;
    const QList<KisCubicCurve>& curves() const override;

    // TODO: cleanup
    static void initDefaultCurves(QList<KisCubicCurve> &curves, int nCh);

protected:
    QList<KisCubicCurve> m_curves;
    QVector<QVector<quint16> > m_transfers;

    void updateTransfers();
};

class WdgPerChannel : public QWidget, public Ui::WdgPerChannel
{
    Q_OBJECT

public:
    WdgPerChannel(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

/**
 *
 */
class KisMultiChannelConfigWidgetBase : public KisConfigWidget
{
    Q_OBJECT

public:
    KisMultiChannelConfigWidgetBase(QWidget * parent, KisPaintDeviceSP dev, Qt::WindowFlags f = 0);
    ~KisMultiChannelConfigWidgetBase() override;

//    void setConfiguration(const KisPropertiesConfigurationSP config) override;
//    KisPropertiesConfigurationSP configuration() const override;

protected Q_SLOTS:
    virtual void setActiveChannel(int ch);
    void logHistView();
    void resetCurve();

protected:
    void init();
    virtual void updateChannelRange() = 0;

    QVector<VirtualChannelInfo> m_virtualChannels;
    int m_activeVChannel = 0;

    // private routines
    inline QPixmap getHistogram();
    inline QPixmap createGradient(Qt::Orientation orient /*, int invert (not used now) */);

    // members
    WdgPerChannel * m_page;
    KisPaintDeviceSP m_dev;
    KisHistogram *m_histogram;
    mutable QList<KisCubicCurve> m_curves;

    // scales for displaying color numbers
    double m_scale = 1.0;
    double m_shift = 0.0;
    bool checkReset = false;

    /*

    m_page->
      cmbChannel
      chkLogarithmic
      resetButton
      hgradient, vgradient
      curveWidget


     */
};

#endif
