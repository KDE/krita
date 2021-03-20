/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-only
 */
#include "pyqtpluginsettings.h"

#include "ui_manager.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <kconfiggroup.h>
#include <QTreeView>


#include "kis_config.h"
#include "kis_icon_utils.h"
#include "PythonPluginManager.h"

PyQtPluginSettings::PyQtPluginSettings(PythonPluginManager *pluginManager, QWidget *parent)
    : KisPreferenceSet(parent)
    , m_pluginManager(pluginManager)
    , m_page(new Ui::ManagerPage)
{
    m_page->setupUi(this);

    QSortFilterProxyModel* const proxy_model = new QSortFilterProxyModel(this);
    proxy_model->setSourceModel(pluginManager->model());
    m_page->pluginsList->setModel(proxy_model);
    m_page->pluginsList->resizeColumnToContents(0);
    m_page->pluginsList->sortByColumn(0, Qt::AscendingOrder);

    const bool is_enabled = bool(pluginManager);
    const bool is_visible = !is_enabled;
    m_page->errorLabel->setVisible(is_visible);
    m_page->pluginsList->setEnabled(is_enabled);
    m_page->textBrowser->setEnabled(is_enabled);

    connect(m_page->pluginsList, SIGNAL(clicked(QModelIndex)), SLOT(updateManual(QModelIndex)));
}

PyQtPluginSettings::~PyQtPluginSettings()
{
    delete m_page;
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


QIcon PyQtPluginSettings::icon()
{
    return KisIconUtils::loadIcon("python");
}


void PyQtPluginSettings::savePreferences() const
{
    Q_EMIT(settingsChanged());
}

void PyQtPluginSettings::loadPreferences()
{
}

void PyQtPluginSettings::loadDefaultPreferences()
{
}

void PyQtPluginSettings::updateManual(const QModelIndex &index)
{
    QModelIndex unsortedIndex = static_cast<QSortFilterProxyModel*>(m_page->pluginsList->model())->mapToSource(index);

    PythonPlugin *plugin = m_pluginManager->model()->plugin(unsortedIndex);
    if (plugin && !plugin->manual().isEmpty()) {
        QString manual = plugin->manual();
        if (manual.startsWith("<html")) {
            m_page->textBrowser->setHtml(manual);
        }
        else {
            m_page->textBrowser->setText(manual);
        }
    }
    else {
        m_page->textBrowser->setHtml("<html><body><h1>No Manual Available</h2></body></html>");
    }
}
