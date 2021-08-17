/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGSELECTORWIDGETBASE_H
#define WGSELECTORWIDGETBASE_H

#include <KisVisualColorModel.h>
#include <QPointer>
#include <QWidget>

class KisDisplayColorConverter;

class WGSelectorWidgetBase : public QWidget
{
    Q_OBJECT
public:
    enum UiMode {
        DockerMode,
        PopupMode
    };

    explicit WGSelectorWidgetBase(QWidget *parent = nullptr, UiMode uiMode = UiMode::DockerMode);
    UiMode uiMode() const;
    void setUiMode(UiMode mode);
    void setDisplayConverter(const KisDisplayColorConverter *converter);
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
    UiMode m_uiMode {DockerMode};
};

#endif // WGSELECTORWIDGETBASE_H
