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

class WGConfigNotifier;

class WGConfig
{
public:
    struct ShadeLine {
        ShadeLine() = default;
        ShadeLine(QVector4D grad, QVector4D offs=QVector4D(), int patchC=-1)
            : gradient(grad), offset(offs), patchCount(patchC) {}
        QVector4D gradient;
        QVector4D offset;
        int patchCount {-1}; // negative value means slider mode
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

    // popups
    int popupSize() const;
    void setPopupSize(int size);

    Qt::Orientation popupColorPatchOrientation() const;
    void setPopupColorPatchOrientation(Qt::Orientation orientation);

    QSize popupColorPatchSize() const;
    void setPopupColorPatchSize(QSize size);

    // shade selector
    static QVector<ShadeLine> defaultShadeSelectorLines();
    QVector<ShadeLine> shadeSelectorLines() const;
    void setShadeSelectorLines(const QVector<ShadeLine> &shadeLines);

    int shadeSelectorLineHeight() const;
    void setShadeSelectorLineHeight(int height);

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

    /**
     * @return the WGConfigNotifier singleton
     */
    static WGConfigNotifier* notifier();

    static const KisColorSelectorConfiguration defaultColorSelectorConfiguration;
    static const bool defaultQuickSettingsEnabled;
    static const int defaultShadeSelectorLineHeight;
    static const bool defaultShadeSelectorUpdateOnExternalChanges;
    static const bool defaultShadeSelectorUpdateOnInteractionEnd;
    static const bool defaultShadeSelectorUpdateOnRightClick;
private:
    /*mutable*/ KConfigGroup m_cfg;
    bool m_readOnly;
};

class WGConfigNotifier : public QObject
{
    Q_OBJECT
public:
    WGConfigNotifier() = default;
    WGConfigNotifier(const WGConfigNotifier&) = delete;
    ~WGConfigNotifier() override = default;

    WGConfigNotifier operator=(const WGConfigNotifier&) = delete;

    /**
     * Notify that the plugin configuration has changed. This will cause the
     * configChanged() signal to be emitted.
     */
    void notifyConfigChanged();
    /**
     * Notify that a setting which affects KisVisualColorSelector or the
     * KisVisualColorModel it uses has changed.
     */
    void notifySelectorConfigChanged();

Q_SIGNALS:
    /**
     * This signal is emitted whenever notifyConfigChanged() is called.
     */
    void configChanged();
    void selectorConfigChanged();
};

#endif // WGCONFIG_H
