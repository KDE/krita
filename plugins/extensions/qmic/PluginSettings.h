/*
 *  Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-only
 */
#ifndef PLUGINSETTINGS_H
#define PLUGINSETTINGS_H

#include "kis_preference_set_registry.h"

class QIcon;

#include "ui_WdgQMicSettings.h"

class PluginSettings : public KisPreferenceSet, public Ui::WdgQMicSettings
{
    Q_OBJECT
public:

    PluginSettings(QWidget *parent = 0);
    ~PluginSettings();

    virtual QString id();
    virtual QString name();
    virtual QString header();
    virtual QIcon icon();

    static QString gmicQtPath();

public Q_SLOTS:
    void savePreferences() const;
    void loadPreferences();
    void loadDefaultPreferences();

Q_SIGNALS:
    void settingsChanged() const;
};


class PluginSettingsUpdateRepeater : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void settingsUpdated();

public Q_SLOTS:
    void updateSettings() {
        Q_EMIT settingsUpdated();
    }
};


class PluginSettingsFactory : public KisAbstractPreferenceSetFactory
{
public:

    PluginSettingsFactory() {
    }

    KisPreferenceSet* createPreferenceSet() {
        PluginSettings* ps = new PluginSettings();
        QObject::connect(ps, SIGNAL(settingsChanged()), &repeater, SLOT(updateSettings()), Qt::UniqueConnection);
        return ps;
    }
    virtual QString id() const {
        return "QMicSettings";
    }
    PluginSettingsUpdateRepeater repeater;
};




#endif // PLUGINSETTINGS_H
