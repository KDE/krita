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
#include "pyqtpluginsettings.h"

#include "ui_manager.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <kconfiggroup.h>

#include <KoIcon.h>


#include "kis_config.h"


PyQtPluginSettings::PyQtPluginSettings(PyKrita::Engine *engine, QWidget *parent) :
    KisPreferenceSet(parent),
    m_manager(new Ui::ManagerPage)
{
    m_manager->setupUi(this);

    QSortFilterProxyModel* const proxy_model = new QSortFilterProxyModel(this);
    proxy_model->setSourceModel(engine);
    m_manager->pluginsList->setModel(proxy_model);
    m_manager->pluginsList->resizeColumnToContents(0);
    m_manager->pluginsList->sortByColumn(0, Qt::AscendingOrder);

    const bool is_enabled = bool(engine);
    const bool is_visible = !is_enabled;
    m_manager->errorLabel->setVisible(is_visible);
    m_manager->pluginsList->setEnabled(is_enabled);

}

PyQtPluginSettings::~PyQtPluginSettings()
{
    delete m_manager;
}

QString PyQtPluginSettings::id()
{
    return QString("pykritapluginmanager");
}

QString PyQtPluginSettings::name()
{
    return header();
}

QString PyQtPluginSettings::header()
{
    return QString(i18n("Python Plugin Manager"));
}


KIcon PyQtPluginSettings::icon()
{
    return koIcon("applications-development");
}


void PyQtPluginSettings::savePreferences() const
{
    emit settingsChanged();
}

void PyQtPluginSettings::loadPreferences()
{
}

void PyQtPluginSettings::loadDefaultPreferences()
{
}
