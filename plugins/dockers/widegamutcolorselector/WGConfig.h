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

#include <QVector4D>

class WGConfig
{
public:
    struct ShadeLine {
        ShadeLine(QVector4D grad, QVector4D offs=QVector4D()): gradient(grad), offset(offs) {}
        QVector4D gradient;
        QVector4D offset;
    };

    WGConfig(bool readOnly = true);
    ~WGConfig();

    static QString configGroupName();

    bool quickSettingsEnabled() const;
    void setQuickSettingsEnabled(bool enabled);

    KisColorSelectorConfiguration colorSelectorConfiguration() const;
    void setColorSelectorConfiguration(const KisColorSelectorConfiguration &config);

    QVector<KisColorSelectorConfiguration> favoriteConfigurations() const;
    static QVector<KisColorSelectorConfiguration> defaultFavoriteConfigurations();
    void setFavoriteConfigurations(const QVector<KisColorSelectorConfiguration> &favoriteConfigs);

    static QVector<ShadeLine> defaultShadeSelectorLines();
    QVector<ShadeLine> shadeSelectorLines() const;
    void setShadeSelectorLines(const QVector<ShadeLine> &shadeLines);

    bool shadeSelectorUpdateOnExternalChanges() const;
    void setShadeSelectorUpdateOnExternalChanges(bool enabled);

    bool shadeSelectorUpdateOnInteractionEnd() const;
    void setShadeSelectorUpdateOnInteractionEnd(bool enabled);

    bool shadeSelectorUpdateOnRightClick() const;
    void setShadeSelectorUpdateOnRightClick(bool enabled);

    template<class T>
    void writeEntry(const QString& name, const T& value) {
        m_cfg.writeEntry(name, value);
    }

    template<class T>
    T readEntry(const QString& name, const T& defaultValue=T()) {
        return m_cfg.readEntry(name, defaultValue);
    }

    static const KisColorSelectorConfiguration defaultColorSelectorConfiguration;
    static const bool defaultQuickSettingsEnabled;
    static const bool defaultShadeSelectorUpdateOnExternalChanges;
    static const bool defaultShadeSelectorUpdateOnInteractionEnd;
    static const bool defaultShadeSelectorUpdateOnRightClick;
private:
    /*mutable*/ KConfigGroup m_cfg;
    bool m_readOnly;
};

#endif // WGCONFIG_H
