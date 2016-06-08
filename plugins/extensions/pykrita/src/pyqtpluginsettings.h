/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@kogmbh.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef PYQTPLUGINSETTINGS_H
#define PYQTPLUGINSETTINGS_H

#include "kis_preference_set_registry.h"
#include "engine.h"

namespace Ui
{
class ManagerPage;
}

class KIcon;

class PyQtPluginSettings : public KisPreferenceSet
{
    Q_OBJECT
public:

    PyQtPluginSettings(PyKrita::Engine *engine, QWidget *parent = 0);
    ~PyQtPluginSettings();

    virtual QString id();
    virtual QString name();
    virtual QString header();
    virtual KIcon icon();

public Q_SLOTS:
    void savePreferences() const;
    void loadPreferences();
    void loadDefaultPreferences();

Q_SIGNALS:
    void settingsChanged() const;

private:
    Ui::ManagerPage *m_manager;
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

    PyQtPluginSettingsFactory(PyKrita::Engine *engine) {
        m_engine = engine;
    }

    KisPreferenceSet* createPreferenceSet() {
        PyQtPluginSettings* ps = new PyQtPluginSettings(m_engine);
        QObject::connect(ps, SIGNAL(settingsChanged()), &repeater, SLOT(updateSettings()), Qt::UniqueConnection);
        return ps;
    }
    virtual QString id() const {
        return "ColorSelectorSettings";
    }
    PyQtPluginSettingsUpdateRepeater repeater;
    PyKrita::Engine *m_engine;
};




#endif // PYQTPLUGINSETTINGS_H
