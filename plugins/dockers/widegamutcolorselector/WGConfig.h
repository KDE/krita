/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGCONFIG_H
#define WGCONFIG_H

#include <KisColorSelectorConfiguration.h>

#include <ksharedconfig.h>
#include <kconfiggroup.h>

class WGConfig
{
public:
    WGConfig(bool readOnly = true);
    ~WGConfig();

    static QString configGroupName();

    bool quickSettingsEnabled() const;
    void setQuickSettingsEnabled(bool enabled);

    QVector<KisColorSelectorConfiguration> favoriteConfigurations() const;
    static QVector<KisColorSelectorConfiguration> defaultFavoriteConfigurations();
    void setFavoriteConfigurations(const QVector<KisColorSelectorConfiguration> &favoriteConfigs);

    template<class T>
    void writeEntry(const QString& name, const T& value) {
        m_cfg.writeEntry(name, value);
    }

    template<class T>
    T readEntry(const QString& name, const T& defaultValue=T()) {
        return m_cfg.readEntry(name, defaultValue);
    }

    static const bool defaultQuickSettingsEnabled;
private:
    /*mutable*/ KConfigGroup m_cfg;
    bool m_readOnly;
};

#endif // WGCONFIG_H
