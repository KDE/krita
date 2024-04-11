/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGConfigSelectorTypes.h"

#include <kconfig.h>
#include <kis_debug.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>

#include <QApplication>
#include <QGlobalStatic>
#include <QStringList>
#include <QTextStream>
#include <QThread>

namespace WGConfig {

Q_GLOBAL_STATIC(WGConfigNotifier, s_notifier_instance)

WGConfig::WGConfig(bool readOnly)
    : m_cfg( KSharedConfig::openConfig()->group(configGroupName()))
    , m_readOnly(readOnly)
{
    if (!readOnly) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(qApp && qApp->thread() == QThread::currentThread());
    }
}

WGConfig::~WGConfig()
{
    if (m_readOnly) return;

    if (qApp && qApp->thread() != QThread::currentThread()) {
        dbgKrita << "WARNING: WGConfig: requested config synchronization from nonGUI thread! Called from:" << kisBacktrace();
        return;
    }

    m_cfg.sync();
}

QString WGConfig::configGroupName()
{
    return QStringLiteral("WideGamutColorSelector");
}

KisColorSelectorConfiguration WGConfig::colorSelectorConfiguration() const
{
    QString config = m_cfg.readEntry("colorSelectorConfiguration", QString());
    return config.isEmpty() ? defaultColorSelectorConfiguration : KisColorSelectorConfiguration(config);
}

void WGConfig::setColorSelectorConfiguration(const KisColorSelectorConfiguration &config)
{
    m_cfg.writeEntry("colorSelectorConfiguration", config.toString());
}

QVector<KisColorSelectorConfiguration> WGConfig::favoriteConfigurations(bool defaultValue) const
{
    if (defaultValue) {
        return defaultFavoriteConfigurations();
    }

    QVector<KisColorSelectorConfiguration> favoriteConfigs;
    QString favorites = m_cfg.readEntry("favoriteSelectorConfigurations", QString());
    if (favorites.isEmpty()) {
        return defaultFavoriteConfigurations();
    }
    QStringList favoriteList(favorites.split(';'));
    for (QString &fav: favoriteList) {
        favoriteConfigs.append(KisColorSelectorConfiguration(fav));
    }
    return favoriteConfigs;
}

QVector<KisColorSelectorConfiguration> WGConfig::defaultFavoriteConfigurations()
{
    using KCSC = KisColorSelectorConfiguration;
    QVector<KCSC> defaults;
    defaults.append(KCSC(KCSC::Triangle, KCSC::Ring, KCSC::SV, KCSC::H));
    defaults.append(KCSC(KCSC::Square, KCSC::Ring, KCSC::SV, KCSC::H));
    defaults.append(KCSC(KCSC::Wheel, KCSC::Slider, KCSC::VH, KCSC::hsvS));
    return defaults;
}

void WGConfig::setFavoriteConfigurations(const QVector<KisColorSelectorConfiguration> &favoriteConfigs)
{
    QStringList favoriteList;
    for (const KisColorSelectorConfiguration &fav: favoriteConfigs) {
        favoriteList.append(fav.toString());
    }
    m_cfg.writeEntry("favoriteSelectorConfigurations", favoriteList.join(';'));
}

QVector<ShadeLine> WGConfig::defaultShadeSelectorLines()
{
    QVector<ShadeLine> defaultLines;
    defaultLines.append(ShadeLine(QVector4D(0.3, 0, 0, 0)));
    defaultLines.append(ShadeLine(QVector4D(0, -0.5, 0, 0)));
    defaultLines.append(ShadeLine(QVector4D(0, 0, 0.5, 0)));
    defaultLines.append(ShadeLine(QVector4D(0, -0.2, 0.2, 0)));
    return defaultLines;
}

QVector<ShadeLine> WGConfig::shadeSelectorLines(bool defaultValue) const
{
    if (defaultValue) {
        return defaultShadeSelectorLines();
    }

    QString configString = m_cfg.readEntry("minimalShadeSelectorLines", QString());
    if (configString.isEmpty()) {
        return defaultShadeSelectorLines();
    }
    QVector<ShadeLine> shadeLines;
    QStringList shadeLineList(configString.split('|'));
    for (const QString &line: qAsConst(shadeLineList)) {
        QVector4D gradient, offset;
        int patchCount = -1;
        QStringList values = line.split(';');
        if (values.size() >= 4) {
            for (int i = 0; i < 4; i++) {
                gradient[i] = qBound(-1.0f, values.at(i).toFloat(), 1.0f);
            }
        }
        if (values.size() >= 8) {
            for (int i = 0; i < 4; i++) {
                offset[i] = qBound(-1.0f, values.at(i + 4).toFloat(), 1.0f);
            }
        }
        if (values.size() >= 9) {
            patchCount = qBound(-1, values.at(8).toInt(), 99);
        }
        shadeLines.append(ShadeLine(gradient, offset, patchCount));
    }
    return shadeLines;
}

void WGConfig::setShadeSelectorLines(const QVector<ShadeLine> &shadeLines)
{
    QStringList shadeLineList;
    for (const ShadeLine &line: shadeLines) {
        QString lineString;
        QTextStream stream(&lineString);
        for (int i = 0; i < 4; i++) {
            stream << line.gradient[i] << ';';
        }
        for (int i = 0; i < 4; i++) {
            stream << line.offset[i] << ';';
        }
        stream << line.patchCount;
        shadeLineList.append(lineString);
    }
    m_cfg.writeEntry("minimalShadeSelectorLines", shadeLineList.join('|'));
}

const KoColorSpace* WGConfig::customSelectionColorSpace(bool defaultValue) const
{
    const KoColorSpace *cs = 0;

    if (!defaultValue) {
        QString modelID = m_cfg.readEntry("customColorSpaceModel", "RGBA");
        QString depthID = m_cfg.readEntry("customColorSpaceDepthID", "U8");
        QString profile = m_cfg.readEntry("customColorSpaceProfile", "");

        cs = KoColorSpaceRegistry::instance()->colorSpace(modelID, depthID, profile);
    }

    if (!cs) {
        cs = KoColorSpaceRegistry::instance()->rgb8();
    }

    return cs;
}

void WGConfig::setCustomSelectionColorSpace(const KoColorSpace *cs)
{
    if(cs) {
        m_cfg.writeEntry("customColorSpaceModel", cs->colorModelId().id());
        m_cfg.writeEntry("customColorSpaceDepthID", cs->colorDepthId().id());
        if (cs->profile()) {
            m_cfg.writeEntry("customColorSpaceProfile", cs->profile()->name());
        }
    }
}

WGConfigNotifier *notifier()
{
    return s_notifier_instance;
}

const KisColorSelectorConfiguration WGConfig::defaultColorSelectorConfiguration =
        KisColorSelectorConfiguration(KisColorSelectorConfiguration::Triangle,
                                      KisColorSelectorConfiguration::Ring,
                                      KisColorSelectorConfiguration::SV,
                                      KisColorSelectorConfiguration::H);

void WGConfigNotifier::notifyConfigChanged()
{
    Q_EMIT configChanged();
}

void WGConfigNotifier::notifySelectorConfigChanged()
{
    Q_EMIT selectorConfigChanged();
}

// ======== Static Configuration Object Instantiation ========

const ColorPatches colorHistory {
    { "colorHistory.orientation", Qt::Horizontal, Qt::Horizontal, Qt::Vertical, true },
    { "colorHistory.patchSize", QSize(16,16), QSize(10,10), QSize(99,99), true },
    { "colorHistory.maxCount", 30, 2, 200, true },
    { "colorHistory.rows", 1, 1, 20, true },
    { "colorHistory.scrolling", ScrollLongitudinal, ScrollNone, ScrollLaterally, true }
};

const ColorPatches commonColors {
    { "commonColors.orientation", Qt::Horizontal, Qt::Horizontal, Qt::Vertical, true },
    { "commonColors.patchSize", QSize(16,16), QSize(10,10), QSize(99,99), true },
    { "commonColors.maxCount", 20, 2, 200, true },
    { "commonColors.rows", 1, 1, 20, true },
    { "commonColors.scrolling", ScrollLongitudinal, ScrollNone, ScrollLaterally, true }
};

const ColorPatches popupPatches {
    { "popupColorPatchOrientation", Qt::Horizontal, Qt::Horizontal, Qt::Vertical, true },
    { "popupColorPatchSize", QSize(32,32), QSize(10,10), QSize(99,99), true },
    { "popupPatches.maxCount", 30, 2, 200, true },
    { "popupPatches.rows", 1, 1, 20, true },
    { "popupPatches.scrolling", ScrollLongitudinal, ScrollNone, ScrollLaterally, true }
};

const GenericSetting<bool> proofToPaintingColors {"proofToPaintingColors", false};
const GenericSetting<bool> colorHistoryEnabled {"colorHistoryEnabled", true};
const GenericSetting<bool> commonColorsEnabled {"commonColorsEnabled", true};
const GenericSetting<bool> colorHistoryShowClearButton { "colorHistoryShowClearButton", false };
const GenericSetting<bool> commonColorsAutoUpdate { "commonColorsAutoUpdate", false };

const GenericSetting<bool> quickSettingsEnabled { "quickSettingsMenuEnabled", true };
const NumericSetting<int> popupSize { "popupSize", 300, 100, 500, true };

const NumericSetting<int> shadeSelectorLineHeight { "shadeSelectorLineHeight", 10, 8, 99 };
const GenericSetting<bool> shadeSelectorUpdateOnExternalChanges { "shadeSelectorUpdateOnExternalChanges", true };
const GenericSetting<bool> shadeSelectorUpdateOnInteractionEnd { "shadeSelectorUpdateOnInteractionEnd", false };
const GenericSetting<bool> shadeSelectorUpdateOnRightClick { "shadeSelectorUpdateOnRightClick", true };

const NumericSetting<KisVisualColorModel::ColorModel> rgbColorModel {
    "rgbColorModel", KisVisualColorModel::HSV, KisVisualColorModel::HSV, KisVisualColorModel::HSY, true
};
const NumericSetting<KisVisualColorSelector::RenderMode> selectorRenderMode {
    "renderMode", KisVisualColorSelector::DynamicBackground, KisVisualColorSelector::StaticBackground, KisVisualColorSelector::CompositeBackground, true
};

} // namespace WGConfig
