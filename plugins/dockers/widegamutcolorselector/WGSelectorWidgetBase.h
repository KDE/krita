/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGSELECTORWIDGETBASE_H
#define WGSELECTORWIDGETBASE_H

#include <KisVisualColorModel.h>
#include <QWidget>

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
    virtual void setModel(KisVisualColorModelSP model);
    virtual void updateSettings();

Q_SIGNALS:
    void sigColorInteraction(bool active);
    void sigChannelValuesChanged(const QVector4D &values);
private:
    UiMode m_uiMode {DockerMode};
};

#endif // WGSELECTORWIDGETBASE_H
