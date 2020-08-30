/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGQUICKSETTINGSWIDGET_H
#define WGQUICKSETTINGSWIDGET_H

#include <KisVisualColorModel.h>
#include <QWidget>

class QAbstractButton;
class QButtonGroup;
class Ui_QuickSettingsWidget;
class KisVisualColorSelector;

class WGQuickSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WGQuickSettingsWidget(QWidget *parent = 0, KisVisualColorSelector *selector = 0);
    ~WGQuickSettingsWidget() override;

protected:
    void showEvent(QShowEvent *event) override;

private Q_SLOTS:
    void slotColorGroupToggled(int id, bool checked);

private:
    Ui_QuickSettingsWidget *m_ui;
    QButtonGroup *m_modelGroup;
    KisVisualColorSelector *m_selector;
};

#endif // WGQUICKSETTINGSWIDGET_H
