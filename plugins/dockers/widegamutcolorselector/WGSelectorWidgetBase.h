/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGSELECTORWIDGETBASE_H
#define WGSELECTORWIDGETBASE_H

#include <kis_display_color_converter.h>
#include <KisVisualColorModel.h>
#include <QPointer>
#include <QSharedPointer>
#include <QWidget>

class WGSelectorDisplayConfig : public QObject
{
    Q_OBJECT
public:
    WGSelectorDisplayConfig() = default;
    ~WGSelectorDisplayConfig() = default;
    const KisDisplayColorConverter *displayConverter() const;
    void setDisplayConverter(const KisDisplayColorConverter *converter);

    bool previewInPaintingCS() const { return m_previewInPaintingCS; }
    void setPreviewInPaintingCS(bool enabled);
Q_SIGNALS:
    void sigDisplayConfigurationChanged();
private:
    QPointer<const KisDisplayColorConverter> m_displayConverter;
    bool m_previewInPaintingCS {false};
};

typedef QSharedPointer<WGSelectorDisplayConfig> WGSelectorDisplayConfigSP;

class WGSelectorWidgetBase : public QWidget
{
    Q_OBJECT
public:
    enum UiMode {
        DockerMode,
        PopupMode
    };

    explicit WGSelectorWidgetBase(WGSelectorDisplayConfigSP displayConfig, QWidget *parent = nullptr, UiMode uiMode = UiMode::DockerMode);
    UiMode uiMode() const;
    void setUiMode(UiMode mode);
    WGSelectorDisplayConfigSP displayConfiguration() const;
    const KisDisplayColorConverter *displayConverter() const;
    /*!
     * \brief The position, relative to the top left corner, where the cursor
     *        of the cursor shall be when showing the popup.
     *
     * The default implementation returns the widget center.
     * \return desired cursor position relative to the top left corner
     */
    virtual QPoint popupOffset() const;
    virtual void setModel(KisVisualColorModelSP model);
    virtual void updateSettings();

Q_SIGNALS:
    void sigColorInteraction(bool active);
    void sigChannelValuesChanged(const QVector4D &values);
private:
    QPointer<const KisDisplayColorConverter> m_converter;
    WGSelectorDisplayConfigSP m_displayConfig;
    UiMode m_uiMode {DockerMode};
};

#endif // WGSELECTORWIDGETBASE_H
