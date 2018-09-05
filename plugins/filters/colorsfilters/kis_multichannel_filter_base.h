/*
 * This file is part of Krita
 *
 * Copyright (c) 2018 Jouni Pentikainen <joupent@gmail.com>
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
 * Base class for filters which use curves to operate on multiple channels.
 */
class KisMultiChannelFilter : public KisColorTransformationFilter
{
public:
    bool needsTransparentPixels(const KisFilterConfigurationSP config, const KoColorSpace *cs) const override;

    /**
     * Get a list of adjustable channels for the color space.
     * If maxChannels is non-negative, the number of channels is capped to the number. This is useful configurations
     * from older documents (created in versions which supported fewer channels).
     */
    static QVector<VirtualChannelInfo> getVirtualChannels(const KoColorSpace *cs, int maxChannels = -1);
    static int findChannel(const QVector<VirtualChannelInfo> &virtualChannels, const VirtualChannelInfo::Type &channelType);

protected:
    KisMultiChannelFilter(const KoID &id, const QString &entry);
};

/**
 * Base class for configurations of KisMultiChannelFilter subclasses
 */
class KisMultiChannelFilterConfiguration : public KisColorTransformationConfiguration
{
public:
    KisMultiChannelFilterConfiguration(int channelCount, const QString & name, qint32 version);
    ~KisMultiChannelFilterConfiguration() override;

    using KisFilterConfiguration::fromXML;
    using KisFilterConfiguration::toXML;
    using KisFilterConfiguration::fromLegacyXML;

    void fromLegacyXML(const QDomElement& root) override;

    void fromXML(const QDomElement& e) override;
    void toXML(QDomDocument& doc, QDomElement& root) const override;

    void setCurves(QList<KisCubicCurve> &curves) override;
    bool isCompatible(const KisPaintDeviceSP) const override;

    const QVector<QVector<quint16> >& transfers() const;
    const QList<KisCubicCurve>& curves() const override;

protected:
    int m_channelCount;
    QList<KisCubicCurve> m_curves;
    QVector<QVector<quint16>> m_transfers;

    void init();
    void updateTransfers();

    virtual KisCubicCurve getDefaultCurve() = 0;
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
 * Base class for configuration widgets of KisMultiChannelFilter subclasses
 */
class KisMultiChannelConfigWidget : public KisConfigWidget
{
    Q_OBJECT

public:
    KisMultiChannelConfigWidget(QWidget * parent, KisPaintDeviceSP dev, Qt::WindowFlags f = 0);
    ~KisMultiChannelConfigWidget() override;

    void setConfiguration(const KisPropertiesConfigurationSP config) override;

protected Q_SLOTS:
    void logHistView();
    void resetCurve();
    void slotChannelSelected(int index);

protected:
    void init();
    void resetCurves();
    void setActiveChannel(int ch);

    virtual void updateChannelControls() = 0;
    virtual KisPropertiesConfigurationSP getDefaultConfiguration() = 0;

    inline QPixmap getHistogram();
    inline QPixmap createGradient(Qt::Orientation orient /*, int invert (not used now) */);

    QVector<VirtualChannelInfo> m_virtualChannels;
    int m_activeVChannel = 0;
    mutable QList<KisCubicCurve> m_curves;

    KisPaintDeviceSP m_dev;
    WdgPerChannel * m_page;
    KisHistogram *m_histogram;
};

#endif
