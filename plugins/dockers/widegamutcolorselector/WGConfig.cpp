/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGConfig.h"

#include <kconfig.h>
#include <kis_debug.h>

#include <QApplication>
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

const bool WGConfig::defaultQuickSettingsEnabled = true;
