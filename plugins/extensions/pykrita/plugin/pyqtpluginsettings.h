/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-only
 */
#ifndef PYQTPLUGINSETTINGS_H
#define PYQTPLUGINSETTINGS_H

#include "kis_preference_set_registry.h"

namespace Ui
{
class ManagerPage;
}

class QIcon;
class PythonPluginManager;

class PyQtPluginSettings : public KisPreferenceSet
{
    Q_OBJECT
public:
    PyQtPluginSettings(PythonPluginManager *pluginManager,
                       QWidget *parent = nullptr);
    ~PyQtPluginSettings() override;

    QString id() override;
    QString name() override;
    QString header() override;
    QIcon icon() override;

public Q_SLOTS:
    void savePreferences() const override;
    void loadPreferences() override;
    void loadDefaultPreferences() override;

Q_SIGNALS:
    void settingsChanged() const;

private Q_SLOTS:

    void updateManual(const QModelIndex &index);

private:
    PythonPluginManager *m_pluginManager;
    Ui::ManagerPage *m_page;

};


class PyQtPluginSettingsUpdateRepeater : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void settingsUpdated();

public Q_SLOTS:
    void updateSettings() {
        Q_EMIT settingsUpdated();
    }
};


class PyQtPluginSettingsFactory : public KisAbstractPreferenceSetFactory
{
public:
    PyQtPluginSettingsFactory(PythonPluginManager *engine)
        : m_pluginManager(engine)
    {
    }

    KisPreferenceSet *createPreferenceSet() override
    {
        PyQtPluginSettings* ps = new PyQtPluginSettings(m_pluginManager);
        QObject::connect(ps, SIGNAL(settingsChanged()), &repeater, SLOT(updateSettings()), Qt::UniqueConnection);
        return ps;
    }
    QString id() const override
    {
        return "PyQtSettings";
    }
    PyQtPluginSettingsUpdateRepeater repeater;
    PythonPluginManager *m_pluginManager;
};




#endif // PYQTPLUGINSETTINGS_H
