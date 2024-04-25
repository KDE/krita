/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2018 Jouni Pentikainen <joupent@gmail.com>
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _KIS_CROSSCHANNEL_FILTER_H_
#define _KIS_CROSSCHANNEL_FILTER_H_

#include <QPair>
#include <QList>

#include <filter/kis_color_transformation_filter.h>
#include <filter/kis_color_transformation_configuration.h>
#include <kis_config_widget.h>
#include <kis_paint_device.h>
#include "ui_wdg_perchannel.h"

#include "virtual_channel_info.h"

#include "kis_multichannel_filter_base.h"
#include <KisCurveWidgetControlsManager.h>

/**
 * Filter which applies a relative adjustment to a (virtual) color channel based on the value of another.
 * The amount of adjustment for a given input is controlled by a user-defined curve.
 */
class KisCrossChannelFilter : public KisMultiChannelFilter
{
public:
    KisCrossChannelFilter();
    ~KisCrossChannelFilter() override;

    KisConfigWidget * createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
    KisFilterConfigurationSP factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;

    KoColorTransformation* createTransformation(const KoColorSpace *cs, const KisFilterConfigurationSP config) const override;

    static inline KoID id() {
        return KoID("crosschannel", i18n("Cross-channel color adjustment"));
    }
};

class KisCrossChannelFilterConfiguration : public KisMultiChannelFilterConfiguration
{
public:
    KisCrossChannelFilterConfiguration(int channelCount, const KoColorSpace *cs, KisResourcesInterfaceSP resourcesInterface);
    KisCrossChannelFilterConfiguration(const KisCrossChannelFilterConfiguration&rhs);

    ~KisCrossChannelFilterConfiguration() override;

    KisFilterConfigurationSP clone() const override;

    const QVector<int> driverChannels() const;

    void setDriverChannels(QVector<int> driverChannels);
    using KisFilterConfiguration::fromXML;

    using KisFilterConfiguration::toXML;
    void fromXML(const QDomElement& e) override;
    void toXML(QDomDocument& doc, QDomElement& root) const override;

    KisCubicCurve getDefaultCurve() override;

    virtual bool compareTo(const KisPropertiesConfiguration* rhs) const override;

    void setProperty(const QString& name, const QVariant& value) override;

private:
    const KoColorSpace *m_colorSpace {nullptr};
    QVector<int> m_driverChannels;

    /**
     * @brief Takes a driver property name with format "driver#", where # is the
     *        index of the channel and puts the index on the "channelIndex"
     *        parameter
     * @param name A string with format "driver#"
     * @param channelIndex An int where the decoded channel index is stored
     * @return true if "name" had a valid format
     * @return false if "name" had an invalid format
     */
    bool channelIndexFromDriverPropertyName(const QString& name, int& channelIndex) const;
};

class KisCrossChannelConfigWidget : public KisMultiChannelConfigWidget
{
    Q_OBJECT

public:
    KisCrossChannelConfigWidget(QWidget * parent, KisPaintDeviceSP dev, Qt::WindowFlags f = Qt::WindowFlags());
    ~KisCrossChannelConfigWidget() override;

    void setConfiguration(const KisPropertiesConfigurationSP config) override;
    KisPropertiesConfigurationSP configuration() const override;

protected:
    void updateChannelControls() override;

    virtual KisPropertiesConfigurationSP getDefaultConfiguration() override;

private Q_SLOTS:
    void slotDriverChannelSelected(int index);

private:
    QVector<int> m_driverChannels;
    QScopedPointer<KisCurveWidgetControlsManagerInt> m_curveControlsManager;
};

#endif
