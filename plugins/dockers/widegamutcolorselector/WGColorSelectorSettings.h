/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGCOLORSELECTORSETTINGS_H
#define WGCOLORSELECTORSETTINGS_H

#include <QDialog>
#include <QScopedPointer>

#include "kis_preference_set_registry.h"

namespace Ui {
    class WGConfigWidget;
}

class WGColorSelectorSettings : public KisPreferenceSet
{
    Q_OBJECT
public:
    explicit WGColorSelectorSettings(QWidget *parent = 0);
    ~WGColorSelectorSettings() override;

    QString id() override;
    QString name() override;
    QString header() override;
    QIcon icon() override;

    static QString stringID();
public Q_SLOTS:
    void savePreferences() const override;
    void loadPreferences() override;
    void loadDefaultPreferences() override;

private:
    QScopedPointer<Ui::WGConfigWidget> m_ui;
};


class WGColorSelectorSettingsFactory : public KisAbstractPreferenceSetFactory
{
public:
    KisPreferenceSet* createPreferenceSet() override
    {
        return new WGColorSelectorSettings();
    }
    QString id() const override { return "WGColorSelectorSettings"; }
};

class WGColorSelectorSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WGColorSelectorSettingsDialog(QWidget *parent = 0);
private:
    WGColorSelectorSettings* m_widget;
};

#endif // WGCOLORSELECTORSETTINGS_H
