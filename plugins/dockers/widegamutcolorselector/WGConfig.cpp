/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGConfig.h"

#include <kconfig.h>
#include <kis_debug.h>

#include <QApplication>
#include <QStringList>
#include <QTextStream>
#include <QThread>

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
    return QString("WideGamutColorSelector");
}

bool WGConfig::quickSettingsEnabled() const
{
    return m_cfg.readEntry("quickSettingsMenuEnabled", defaultQuickSettingsEnabled);
}

void WGConfig::setQuickSettingsEnabled(bool enabled)
{
    m_cfg.writeEntry("quickSettingsMenuEnabled", enabled);
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

QVector<KisColorSelectorConfiguration> WGConfig::favoriteConfigurations() const
{
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

QVector<WGConfig::ShadeLine> WGConfig::defaultShadeSelectorLines()
{
    QVector<ShadeLine> defaultLines;
    defaultLines.append(ShadeLine(QVector4D(0.3, 0, 0, 0)));
    defaultLines.append(ShadeLine(QVector4D(0, -0.5, 0, 0)));
    defaultLines.append(ShadeLine(QVector4D(0, 0, 0.5, 0)));
    defaultLines.append(ShadeLine(QVector4D(0, -0.2, 0.2, 0)));
    return defaultLines;
}

QVector<WGConfig::ShadeLine> WGConfig::shadeSelectorLines() const
{
    QString configString = m_cfg.readEntry("minimalShadeSelectorLines", QString());
    if (configString.isEmpty()) {
        return defaultShadeSelectorLines();
    }
    QVector<ShadeLine> shadeLines;
    QStringList shadeLineList(configString.split('|'));
    for (QString &line: shadeLineList) {
        QVector4D gradient, offset;
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
        shadeLines.append(ShadeLine(gradient, offset));
    }
    return shadeLines;
}

void WGConfig::setShadeSelectorLines(const QVector<WGConfig::ShadeLine> &shadeLines)
{
    QStringList shadeLineList;
    for (const ShadeLine &line: shadeLines) {
        QString lineString;
        QTextStream stream(&lineString);
        for (int i = 0; i < 4; i++) {
            stream << line.gradient[i] << ';';
        }
        for (int i = 0; i < 3; i++) {
            stream << line.offset[i] << ';';
        }
        stream << line.offset[3];
        shadeLineList.append(lineString);
    }
    m_cfg.writeEntry("minimalShadeSelectorLines", shadeLineList.join('|'));
}

bool WGConfig::shadeSelectorUpdateOnExternalChanges() const
{
    return m_cfg.readEntry("shadeSelectorUpdateOnExternalChanges", defaultShadeSelectorUpdateOnExternalChanges);
}

void WGConfig::setShadeSelectorUpdateOnExternalChanges(bool enabled)
{
    m_cfg.writeEntry("shadeSelectorUpdateOnExternalChanges", enabled);
}

bool WGConfig::shadeSelectorUpdateOnInteractionEnd() const
{
    return m_cfg.readEntry("shadeSelectorUpdateOnInteractionEnd", defaultShadeSelectorUpdateOnInteractionEnd);
}

void WGConfig::setShadeSelectorUpdateOnInteractionEnd(bool enabled)
{
    m_cfg.writeEntry("shadeSelectorUpdateOnInteractionEnd", enabled);
}

bool WGConfig::shadeSelectorUpdateOnRightClick() const
{
    return m_cfg.readEntry("shadeSelectorUpdateOnRightClick", defaultShadeSelectorUpdateOnRightClick);
}

void WGConfig::setShadeSelectorUpdateOnRightClick(bool enabled)
{
    m_cfg.writeEntry("shadeSelectorUpdateOnRightClick", enabled);
}

const KisColorSelectorConfiguration WGConfig::defaultColorSelectorConfiguration =
        KisColorSelectorConfiguration(KisColorSelectorConfiguration::Triangle,
                                      KisColorSelectorConfiguration::Ring,
                                      KisColorSelectorConfiguration::SV,
                                      KisColorSelectorConfiguration::H);
const bool WGConfig::defaultQuickSettingsEnabled = true;
const bool WGConfig::defaultShadeSelectorUpdateOnExternalChanges = true;
const bool WGConfig::defaultShadeSelectorUpdateOnInteractionEnd = false;
const bool WGConfig::defaultShadeSelectorUpdateOnRightClick = true;
