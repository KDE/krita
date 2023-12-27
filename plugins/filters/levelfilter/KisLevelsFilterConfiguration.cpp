/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QRegularExpression>

#include <kis_dom_utils.h>

#include "KisLevelsFilterConfiguration.h"

KisLevelsFilterConfiguration::KisLevelsFilterConfiguration(int channelCount, qint32 version, KisResourcesInterfaceSP resourcesInterface)
    : KisColorTransformationConfiguration(defaultName(), version, resourcesInterface)
{
    setChannelCount(channelCount);
    setDefaults();

}

KisLevelsFilterConfiguration::KisLevelsFilterConfiguration(int channelCount, KisResourcesInterfaceSP resourcesInterface)
    : KisLevelsFilterConfiguration(channelCount, defaultVersion(), resourcesInterface)
{}

KisLevelsFilterConfiguration::KisLevelsFilterConfiguration(const KisLevelsFilterConfiguration &rhs)
    : KisColorTransformationConfiguration(rhs)
    , m_transfers(rhs.m_transfers)
    , m_lightnessTransfer(rhs.m_lightnessTransfer)
{}

KisFilterConfigurationSP KisLevelsFilterConfiguration::clone() const
{
    return new KisLevelsFilterConfiguration(*this);
}

bool KisLevelsFilterConfiguration::isCompatible(const KisPaintDeviceSP dev) const
{
    return useLightnessMode() || (int)dev->compositionSourceColorSpace()->channelCount() == channelCount();
}

const QVector<KisLevelsCurve> KisLevelsFilterConfiguration::levelsCurves() const
{
    QVector<KisLevelsCurve> levelsCurves_;
    for (qint32 i = 0; i < channelCount(); ++i) {
        const QString levelsCurveStr = getString(QString("channel_") + KisDomUtils::toString(i), "");
        levelsCurves_.append(levelsCurveStr.isEmpty() ? KisLevelsCurve() : KisLevelsCurve(levelsCurveStr));
    }
    return levelsCurves_;
}

const KisLevelsCurve KisLevelsFilterConfiguration::lightnessLevelsCurve() const
{
    const QString levelsCurveStr = getString("lightness", "");
    return levelsCurveStr.isEmpty() ? KisLevelsCurve() : KisLevelsCurve(levelsCurveStr);
}

void KisLevelsFilterConfiguration::setLevelsCurves(const QVector<KisLevelsCurve> &newLevelsCurves)
{
    for (int i = 0; i < newLevelsCurves.size(); ++i) {
        setProperty(QString("channel_") + KisDomUtils::toString(i), newLevelsCurves[i].toString());
    }
    setChannelCount(newLevelsCurves.size());
    updateTransfers();
}

void KisLevelsFilterConfiguration::setLightnessLevelsCurve(const KisLevelsCurve &newLightnessLevelsCurve)
{
    setProperty("lightness", newLightnessLevelsCurve.toString());
}

void KisLevelsFilterConfiguration::updateTransfers()
{
    const QVector<KisLevelsCurve> lc = levelsCurves();
    m_transfers.resize(lc.size());
    for (int i = 0; i < lc.size(); i++) {
        m_transfers[i] = lc[i].uint16Transfer();
    }
}

void KisLevelsFilterConfiguration::updateLightnessTransfer()
{
    const KisLevelsCurve lightnessLevelsCurve_ = lightnessLevelsCurve();

    m_lightnessTransfer = lightnessLevelsCurve_.uint16Transfer();
}

const QVector<QVector<quint16>>& KisLevelsFilterConfiguration::transfers() const
{
    return m_transfers;
}

const QVector<quint16>& KisLevelsFilterConfiguration::lightnessTransfer() const
{
    return m_lightnessTransfer;
}

bool KisLevelsFilterConfiguration::useLightnessMode() const
{
    const QString mode = getString("mode", "");
    if (mode == "lightness") {
        return true;
    } else if (mode == "channels") {
        return false;
    }
    return defaultUseLightnessMode();
}

bool KisLevelsFilterConfiguration::showLogarithmicHistogram() const
{
    const QString mode = getString("histogram_mode", "");
    if (mode == "logarithmic") {
        return true;
    } else if (mode == "linear") {
        return false;
    }
    return defaultShowLogarithmicHistogram();
}

void KisLevelsFilterConfiguration::setUseLightnessMode(bool newUseLightnessMode)
{
    setProperty("mode", newUseLightnessMode ? "lightness" : "channels");
}

void KisLevelsFilterConfiguration::setShowLogarithmicHistogram(bool newShowLogarithmicHistogram)
{
    setProperty("histogram_mode", newShowLogarithmicHistogram ? "logarithmic" : "linear");
}

/**
 * The purpose of this function is to copy the values of the legacy
 * options (levels filter version < 2), that correspond to the lightness
 * levels adjustment, to the new and compact "lightness" option which
 * is used now.
 * Note that the "blackvalue", "whitevalue", "outblackvalue" and "outwhitevalue"
 * legacy properties span the range [0, 255] while the values in the "lightness"
 * property span the range [0, 1]
 */
void KisLevelsFilterConfiguration::setLightessLevelsCurveFromLegacyValues()
{
    const double inputBlackPoint = static_cast<double>(getInt("blackvalue", 0)) / 255.0;
    const double inputWhitePoint = static_cast<double>(getInt("whitevalue", 255)) / 255.0;
    const double inputGamma = getDouble("gammavalue", 1.0);
    const double outputBlackPoint = static_cast<double>(getInt("outblackvalue", 0)) / 255.0;
    const double outputWhitePoint = static_cast<double>(getInt("outwhitevalue", 255)) / 255.0;
    KisColorTransformationConfiguration::setProperty(
        "lightness",
        KisLevelsCurve(inputBlackPoint, inputWhitePoint, inputGamma, outputBlackPoint, outputWhitePoint).toString()
    );
}

/**
 * The purpose of this function is to copy the values of the new and
 * compact "lightness" option, that correspond to the lightness levels
 * adjustment, to the legacy options that where used before version 2 of
 * the filter. Storing the legacy options as well as the new ones
 * improves backwards compatibility of documents.
 * Note that the "blackvalue", "whitevalue", "outblackvalue" and "outwhitevalue"
 * legacy properties span the range [0, 255] while the values in the "lightness"
 * property span the range [0, 1]
 */
void KisLevelsFilterConfiguration::setLegacyValuesFromLightnessLevelsCurve()
{
    KisLevelsCurve lightnessLevelsCurve_ = lightnessLevelsCurve();
    KisColorTransformationConfiguration::setProperty("blackvalue", static_cast<int>(qRound(lightnessLevelsCurve_.inputBlackPoint() * 255.0)));
    KisColorTransformationConfiguration::setProperty("whitevalue", static_cast<int>(qRound(lightnessLevelsCurve_.inputWhitePoint() * 255.0)));
    KisColorTransformationConfiguration::setProperty("gammavalue", lightnessLevelsCurve_.inputGamma());
    KisColorTransformationConfiguration::setProperty("outblackvalue", static_cast<int>(qRound(lightnessLevelsCurve_.outputBlackPoint() * 255.0)));
    KisColorTransformationConfiguration::setProperty("outwhitevalue", static_cast<int>(qRound(lightnessLevelsCurve_.outputWhitePoint() * 255.0)));
}

/**
 * The options may be changed directly using the "setProperty" method of
 * the "KisPropertiesConfiguration". In this case we must intercept the
 * action to update the transfer function luts after setting the property.
 * if some legacy property is set (lightness levels properties prior to
 * version 2) then the legacy properties are copied to the new and
 * compact "lightness" property. Conversely, if the "lightness" property
 * is set, its values are copied to the legacy properties.
 */
void KisLevelsFilterConfiguration::setProperty(const QString &name, const QVariant &value)
{
    KisColorTransformationConfiguration::setProperty(name, value);

    if (name == "lightness") {
        setLegacyValuesFromLightnessLevelsCurve();
        updateLightnessTransfer();
    } else if (name == "blackvalue" || name == "whitevalue" || name == "gammavalue" ||
               name == "outblackvalue" || name == "outwhitevalue") {
        setLightessLevelsCurveFromLegacyValues();
        updateLightnessTransfer();
    } else if (QRegularExpression("channel_\\d+").match(name).hasMatch()) {
        updateTransfers();
    }
}

int KisLevelsFilterConfiguration::channelCount() const
{
    return getInt("number_of_channels", 0);
}

void KisLevelsFilterConfiguration::setChannelCount(int newChannelCount)
{
    setProperty("number_of_channels", newChannelCount);
}

void KisLevelsFilterConfiguration::fromLegacyXML(const QDomElement& root)
{
    fromXML(root);
}

void KisLevelsFilterConfiguration::fromXML(const QDomElement& root)
{
    int version;
    version = root.attribute("version").toInt();

    QDomElement e = root.firstChild().toElement();
    QString attributeName;
    KisLevelsCurve lightnessLevelsCurve;
    QVector<KisLevelsCurve> levelsCurves;
    bool lightnessMode = defaultUseLightnessMode();
    bool logarithmicHistogram = defaultShowLogarithmicHistogram();

    if (version == 1) {
        while (!e.isNull()) {
            attributeName = e.attribute("name");
            if (attributeName == "gammavalue") {
                const double value = KisDomUtils::toDouble(e.text());
                lightnessLevelsCurve.setInputGamma(value);
            } else {
                const double value = KisDomUtils::toDouble(e.text()) / 255.0;
                if (attributeName == "blackvalue") {
                    lightnessLevelsCurve.setInputBlackPoint(value);
                } else if (attributeName == "whitevalue") {
                    lightnessLevelsCurve.setInputWhitePoint(value);
                } else if (attributeName == "outblackvalue") {
                    lightnessLevelsCurve.setOutputBlackPoint(value);
                } else if (attributeName == "outwhitevalue") {
                    lightnessLevelsCurve.setOutputWhitePoint(value);
                }
            }
            e = e.nextSiblingElement();
        }
    } else if (version == 2) {
        int numChannels = 0;
        QHash<int, KisLevelsCurve> unsortedLevelsCurves;
        KisLevelsCurve levelsCurve;

        while (!e.isNull()) {
            attributeName = e.attribute("name");
            if (attributeName == "mode") {
                lightnessMode = e.text() != "channels";
            } else if (attributeName == "histogram_mode") {
                logarithmicHistogram = e.text() == "logarithmic";
            } else if (attributeName == "lightness") {
                lightnessLevelsCurve.fromString(e.text());
            } else if (attributeName == "number_of_channels") {
                numChannels = e.text().toInt();
            } else {
                const QRegularExpression rx("channel_(\\d+)");
                const QRegularExpressionMatch match = rx.match(attributeName);
                if (match.hasMatch()) {
                    const int index = match.captured(1).toInt();
                    if (!e.text().isEmpty()) {
                        levelsCurve.fromString(e.text());
                        unsortedLevelsCurves[index] = levelsCurve;
                    }
                }
            }
            e = e.nextSiblingElement();
        }

        for (int i = 0; i < numChannels; ++i) {
            if (unsortedLevelsCurves.contains(i)) {
                levelsCurves.append(unsortedLevelsCurves[i]);
            } else {
                levelsCurves.append(defaultLevelsCurve());
            }
        }
    }

    setVersion(defaultVersion());
    setLevelsCurves(levelsCurves);
    setLightnessLevelsCurve(lightnessLevelsCurve);
    setUseLightnessMode(lightnessMode);
    setShowLogarithmicHistogram(logarithmicHistogram);
}

void addParamNode(QDomDocument& doc,
                  QDomElement& root,
                  const QString &name,
                  const QString &value,
                  bool internal = false)
{
    QDomText text = doc.createTextNode(value);
    QDomElement t = doc.createElement("param");
    t.setAttribute("name", name);
    if (internal) {
        t.setAttribute("type", "internal");
    }
    t.appendChild(text);
    root.appendChild(t);
}

void KisLevelsFilterConfiguration::toXML(QDomDocument& doc, QDomElement& root) const
{
    /**
     * levels curve param follows this format:
     *      "input_black_point;input_white_point;input_gamma;output_black_point;output_white_point"
     * 
     * For backwards compatibility, the "blackvalue" etc. values are also saved.
     * Those contain the values of the lightness mode levels.
     * 
     * @code
     * <params version=2>
     *      <param name="mode">lightness</param>
     *      <param name="histogram_mode">logarithmic</param>
     *      <param name="lightness">0;1;0.4;0;1</param>
     *      <param name="number_of_channels">3</param>
     *      <param name="channel_0">0;1;1;0;1</param>
     *      <param name="channel_1">0.2;0.8;1.2;0.25;0.75</param>
     *      <param name="channel_2">0;1;0.6;0;1</param>
     * 
     *      <param type="internal" name="blackvalue">0</param>
     *      <param type="internal" name="whitevalue">255</param>
     *      <param type="internal" name="gammavalue">1</param>
     *      <param type="internal" name="outblackvalue">0</param>
     *      <param type="internal" name="outwhitevalue">255</param>
     * </params>
     * @endcode
     */

    root.setAttribute("version", version());

    QDomText text;
    QDomElement t;

    addParamNode(doc, root, "mode", useLightnessMode() ? "lightness" : "channels");
    addParamNode(doc, root, "histogram_mode", showLogarithmicHistogram() ? "logarithmic" : "linear");
    addParamNode(doc, root, "lightness", lightnessLevelsCurve().toString());
    addParamNode(doc, root, "number_of_channels", KisDomUtils::toString(channelCount()));

    const QVector<KisLevelsCurve> levelsCurves_ = levelsCurves();
    for (int i = 0; i < levelsCurves_.size(); ++i) {
        const QString name = QString("channel_") + KisDomUtils::toString(i);
        const QString value = levelsCurves_[i].toString();
        addParamNode(doc, root, name, value);
    }
    const KisLevelsCurve lightnessCurve_ = lightnessLevelsCurve();
    addParamNode(doc, root, "blackvalue", KisDomUtils::toString(static_cast<int>(qRound(lightnessCurve_.inputBlackPoint() * 255.0))), true);
    addParamNode(doc, root, "whitevalue", KisDomUtils::toString(static_cast<int>(qRound(lightnessCurve_.inputWhitePoint() * 255.0))), true);
    addParamNode(doc, root, "gammavalue", KisDomUtils::toString(lightnessCurve_.inputGamma()), true);
    addParamNode(doc, root, "outblackvalue", KisDomUtils::toString(static_cast<int>(qRound(lightnessCurve_.outputBlackPoint() * 255.0))), true);
    addParamNode(doc, root, "outwhitevalue", KisDomUtils::toString(static_cast<int>(qRound(lightnessCurve_.outputWhitePoint() * 255.0))), true);
}

void KisLevelsFilterConfiguration::setDefaults()
{
    setUseLightnessMode(defaultUseLightnessMode());
    setShowLogarithmicHistogram(defaultShowLogarithmicHistogram());
    setLightnessLevelsCurve(defaultLevelsCurve());

    QVector<KisLevelsCurve> levelsCurves_;
    for (int i = 0; i < channelCount(); ++i) {
        levelsCurves_.append(defaultLevelsCurve());
    }
    setLevelsCurves(levelsCurves_);

    updateTransfers();
    updateLightnessTransfer();
}
