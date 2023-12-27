/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_LEVELS_FILTER_CONFIGURATION_H
#define KIS_LEVELS_FILTER_CONFIGURATION_H

#include <filter/kis_color_transformation_configuration.h>
#include <kis_paint_device.h>
#include <KisLevelsCurve.h>

class KisLevelsFilterConfiguration : public KisColorTransformationConfiguration
{
public:
    KisLevelsFilterConfiguration(int channelCount, qint32 version, KisResourcesInterfaceSP resourcesInterface);
    KisLevelsFilterConfiguration(int channelCount, KisResourcesInterfaceSP resourcesInterface);
    KisLevelsFilterConfiguration(const KisLevelsFilterConfiguration &rhs);

    KisFilterConfigurationSP clone() const override;

    static inline QString defaultName() { return "levels"; }
    static constexpr qint32 defaultVersion() { return 2; }
    static inline KisLevelsCurve defaultLevelsCurve() { return KisLevelsCurve(); }
    static constexpr bool defaultUseLightnessMode() { return true; }
    static constexpr bool defaultShowLogarithmicHistogram() { return false; }

    using KisFilterConfiguration::fromXML;
    using KisFilterConfiguration::toXML;
    using KisFilterConfiguration::fromLegacyXML;
    
    void fromLegacyXML(const QDomElement& root) override;
    void fromXML(const QDomElement& e) override;
    void toXML(QDomDocument& doc, QDomElement& root) const override;

    void setProperty(const QString &name, const QVariant &value) override;

    const QVector<KisLevelsCurve> levelsCurves() const;
    const KisLevelsCurve lightnessLevelsCurve() const;
    void setLevelsCurves(const QVector<KisLevelsCurve> &newLevelsCurves);
    void setLightnessLevelsCurve(const KisLevelsCurve &newLightnessLevelsCurve);
    const QVector<QVector<quint16>>& transfers() const;
    const QVector<quint16>& lightnessTransfer() const;

    bool useLightnessMode() const;
    bool showLogarithmicHistogram() const;
    void setUseLightnessMode(bool newUseLightnessMode);
    void setShowLogarithmicHistogram(bool newShowLogarithmicHistogram);

    bool isCompatible(const KisPaintDeviceSP) const override;

    void setDefaults();

private:
    QVector<QVector<quint16>> m_transfers;
    QVector<quint16> m_lightnessTransfer;

    int channelCount() const;
    void setChannelCount(int newChannelCount);

    void setLightessLevelsCurveFromLegacyValues();
    void setLegacyValuesFromLightnessLevelsCurve();

    void updateTransfers();
    void updateLightnessTransfer();
};

#endif
