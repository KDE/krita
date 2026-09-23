/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-only
 */
#include "pyqtpluginsettings.h"

#include "ui_manager.h"

#include <QVBoxLayout>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QTextBrowser>

#include <kconfiggroup.h>

#include "kis_config.h"
#include "kis_icon_utils.h"
#include "PythonPluginManager.h"

enum TextBrowserTab {
    Manual,
    Status,
};

PyQtPluginSettings::PyQtPluginSettings(PythonPluginManager *pluginManager, QWidget *parent)
    : KisPreferenceSet(parent)
    , m_pluginManager(pluginManager)
    , m_page(new Ui::ManagerPage)
{
    m_page->setupUi(this);

    QSortFilterProxyModel* const proxy_model = new QSortFilterProxyModel(this);
    proxy_model->setSourceModel(pluginManager->model());
    m_page->pluginsList->setModel(proxy_model);
    m_page->pluginsList->resizeColumnToContents(PythonPluginsModel::COL_ENABLED);
    m_page->pluginsList->resizeColumnToContents(PythonPluginsModel::COL_NAME);
    m_page->pluginsList->header()->setSectionResizeMode(PythonPluginsModel::COL_COMMENT, QHeaderView::Stretch);
    m_page->pluginsList->header()->setSectionResizeMode(PythonPluginsModel::COL_STATUS, QHeaderView::ResizeToContents);
    m_page->pluginsList->header()->setStretchLastSection(false);
    m_page->pluginsList->sortByColumn(PythonPluginsModel::COL_NAME, Qt::AscendingOrder);
    m_page->pluginsList->setSortingEnabled(true);

    const bool is_enabled = bool(pluginManager);
    const bool is_visible = !is_enabled;
    m_page->errorLabel->setVisible(is_visible);
    m_page->pluginsList->setEnabled(is_enabled);
    m_page->textTabWidget->setEnabled(is_enabled);

    m_manualBrowser = new QTextBrowser();
    m_page->textTabWidget->addTab(m_manualBrowser, i18nc("@title:tab plugin manual", "Manual"));
    m_statusBrowser = new QTextBrowser();
    m_page->textTabWidget->addTab(m_statusBrowser, i18nc("@title:tab plugin status", "Status"));

    connect(m_page->pluginsList, SIGNAL(clicked(QModelIndex)), SLOT(updateTextBrowser(QModelIndex)));
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

void PyQtPluginSettings::updateTextBrowser(const QModelIndex &index)
{
    QModelIndex unsortedIndex = static_cast<QSortFilterProxyModel*>(m_page->pluginsList->model())->mapToSource(index);
    PythonPlugin *plugin = m_pluginManager->model()->plugin(unsortedIndex);
    if (!plugin) { return; }

    QString manual = plugin->manual();
    if (manual.isEmpty()) {
        m_manualBrowser->setHtml("<html><body><h1>No Manual Available</h2></body></html>");
    } else if (manual.startsWith("<html")) {
        m_manualBrowser->setHtml(manual);
    } else {
        m_manualBrowser->setText(manual);
    }

    m_statusBrowser->setHtml(
        QString(i18nc("%1 plugin file path", "Location: %1")).arg(plugin->desktopFilePath()) + "<br>" +
        (plugin->isEnabled() ? i18n("Enabled: true") : i18n("Enabled: false")) + "<br>" +
        QString(i18nc("%1 plugin status text",
                      "Status: %1")).arg(plugin->statusText()) + "<br>" +
        QString(i18nc("%1 plugin error message",
                      "Error: %1")).arg(!plugin->errorReason().isEmpty() ?
                          plugin->errorReason() : i18n("None")) + "<br>"
    );

    if (index.column() == PythonPluginsModel::COL_STATUS) {
        m_page->textTabWidget->setCurrentIndex(TextBrowserTab::Status);
    } else {
        m_page->textTabWidget->setCurrentIndex(TextBrowserTab::Manual);
    }
}
