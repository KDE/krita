/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QDomDocument>
#include <QDomElement>
#include <QDomText>

#include <kis_dom_utils.h>

#include "KisLevelsFilterConfiguration.h"

KisLevelsFilterConfiguration::KisLevelsFilterConfiguration(int channelCount, qint32 version, KisResourcesInterfaceSP resourcesInterface)
    : KisColorTransformationConfiguration(defaultName(), version, resourcesInterface)
    , m_channelCount(channelCount)
{
    setDefaults();
}

KisLevelsFilterConfiguration::KisLevelsFilterConfiguration(int channelCount, KisResourcesInterfaceSP resourcesInterface)
    : KisLevelsFilterConfiguration(channelCount, defaultVersion(), resourcesInterface)
{}

KisLevelsFilterConfiguration::KisLevelsFilterConfiguration(const KisLevelsFilterConfiguration &rhs)
    : KisColorTransformationConfiguration(rhs)
    , m_channelCount(rhs.m_channelCount)
    , m_levelsCurves(rhs.m_levelsCurves)
    , m_lightnessLevelsCurve(rhs.m_lightnessLevelsCurve)
    , m_transfers(rhs.m_transfers)
    , m_lightnessTransfer(rhs.m_lightnessTransfer)
    , m_showLogarithmicHistogram(rhs.m_showLogarithmicHistogram)
    , m_useLightnessMode(rhs.m_useLightnessMode)
{}

KisFilterConfigurationSP KisLevelsFilterConfiguration::clone() const
{
    return new KisLevelsFilterConfiguration(*this);
}

bool KisLevelsFilterConfiguration::isCompatible(const KisPaintDeviceSP dev) const
{
    return m_useLightnessMode || (int)dev->compositionSourceColorSpace()->channelCount() == m_channelCount;
}

const QVector<KisLevelsCurve>& KisLevelsFilterConfiguration::levelsCurves() const
{
    return m_levelsCurves;
}

const KisLevelsCurve& KisLevelsFilterConfiguration::lightnessLevelsCurve() const
{
    return m_lightnessLevelsCurve;
}

void KisLevelsFilterConfiguration::setLevelsCurves(const QVector<KisLevelsCurve> &newLevelsCurves)
{
    m_levelsCurves = newLevelsCurves;
    m_channelCount = newLevelsCurves.size();
    updateTransfers();
}

void KisLevelsFilterConfiguration::setLightnessLevelsCurve(const KisLevelsCurve &newLightnessLevelsCurve)
{
    m_lightnessLevelsCurve = newLightnessLevelsCurve;
    updateLightnessTransfer();
}

void KisLevelsFilterConfiguration::updateTransfers()
{
    m_transfers.resize(m_channelCount);
    for (int i = 0; i < m_channelCount; i++) {
        m_transfers[i] = m_levelsCurves[i].uint16Transfer();
    }
}

void KisLevelsFilterConfiguration::updateLightnessTransfer()
{
    m_lightnessTransfer = m_lightnessLevelsCurve.uint16Transfer();
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
    return m_useLightnessMode;
}

bool KisLevelsFilterConfiguration::showLogarithmicHistogram() const
{
    return m_showLogarithmicHistogram;
}

void KisLevelsFilterConfiguration::setUseLightnessMode(bool newUseLightnessMode)
{
    m_useLightnessMode = newUseLightnessMode;
}

void KisLevelsFilterConfiguration::setShowLogarithmicHistogram(bool newShowLogarithmicHistogram)
{
    m_showLogarithmicHistogram = newShowLogarithmicHistogram;
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
                QRegExp rx("channel_(\\d+)");
                if (rx.indexIn(attributeName, 0) != -1) {
                    const int index = rx.cap(1).toInt();
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

    addParamNode(doc, root, "mode", m_useLightnessMode ? "lightness" : "channels");
    addParamNode(doc, root, "histogram_mode", m_showLogarithmicHistogram ? "logarithmic" : "linear");
    addParamNode(doc, root, "lightness", m_lightnessLevelsCurve.toString());
    addParamNode(doc, root, "number_of_channels", KisDomUtils::toString(m_channelCount));

    for (int i = 0; i < m_levelsCurves.size(); ++i) {
        const QString name = QString("channel_") + KisDomUtils::toString(i);
        const QString value = m_levelsCurves[i].toString();
        addParamNode(doc, root, name, value);
    }

    addParamNode(doc, root, "blackvalue", KisDomUtils::toString(static_cast<int>(qRound(m_lightnessLevelsCurve.inputBlackPoint() * 255.0))), true);
    addParamNode(doc, root, "whitevalue", KisDomUtils::toString(static_cast<int>(qRound(m_lightnessLevelsCurve.inputWhitePoint() * 255.0))), true);
    addParamNode(doc, root, "gammavalue", KisDomUtils::toString(m_lightnessLevelsCurve.inputGamma()), true);
    addParamNode(doc, root, "outblackvalue", KisDomUtils::toString(static_cast<int>(qRound(m_lightnessLevelsCurve.outputBlackPoint() * 255.0))), true);
    addParamNode(doc, root, "outwhitevalue", KisDomUtils::toString(static_cast<int>(qRound(m_lightnessLevelsCurve.outputWhitePoint() * 255.0))), true);
}

bool KisLevelsFilterConfiguration::compareTo(const KisPropertiesConfiguration *rhs) const
{
    const KisLevelsFilterConfiguration *otherConfig = dynamic_cast<const KisLevelsFilterConfiguration *>(rhs);

    return
        otherConfig
        && KisFilterConfiguration::compareTo(rhs)
        && m_channelCount == otherConfig->m_channelCount
        && m_levelsCurves == otherConfig->m_levelsCurves
        && m_lightnessLevelsCurve == otherConfig->m_lightnessLevelsCurve
        && m_transfers == otherConfig->m_transfers
        && m_lightnessTransfer == otherConfig->m_lightnessTransfer;
}

void KisLevelsFilterConfiguration::setDefaults()
{
    m_useLightnessMode = defaultUseLightnessMode();
    m_showLogarithmicHistogram = defaultShowLogarithmicHistogram();
    m_lightnessLevelsCurve = defaultLevelsCurve();

    m_levelsCurves.clear();
    for (int i = 0; i < m_channelCount; ++i) {
        m_levelsCurves.append(defaultLevelsCurve());
    }
    updateTransfers();
    updateLightnessTransfer();
}
