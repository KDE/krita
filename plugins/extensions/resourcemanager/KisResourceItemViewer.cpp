/*
 *  SPDX-FileCopyrightText: 2023 Srirupa Datta <srirupa.sps@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisResourceItemViewer.h"

#include "ResourceListViewModes.h"
#include "KisPopupButton.h"
#include <KoIcon.h>
#include <kis_config.h>
#include <QMenu>

KisResourceItemViewer::KisResourceItemViewer(Viewer type, QWidget *parent) :
    KisPopupButton(parent),
    m_mode(ListViewMode::IconGrid),
    m_type(type)
{
    QMenu* viewModeMenu = new QMenu(this);
    KisConfig cfg(true);

    viewModeMenu->setStyleSheet("margin: 6px");
    setArrowVisible(false);
    setAutoRaise(true);

    // View Modes Btns
    viewModeMenu->addSection(i18nc("@title Which elements to display (e.g., thumbnails or details)", "Display"));

    if (m_type == Viewer::TableAvailable) {
        m_mode = (cfg.readEntry<quint32>("ResourceItemsBCSearch.viewMode", 1) == 1)? ListViewMode::IconGrid : ListViewMode::Detail;
    } else if (m_type == Viewer::ResourceManager) {
        m_mode = (cfg.readEntry<quint32>("ResourceItemsRM.viewMode", 1) == 1)? ListViewMode::IconGrid : ListViewMode::Detail;
    } else {
        m_mode = (cfg.readEntry<quint32>("ResourceItemsBCSelected.viewMode", 1) == 1)? ListViewMode::IconGrid : ListViewMode::Detail;
    }

    QActionGroup *actionGroup = new QActionGroup(viewModeMenu);

    QAction* action = viewModeMenu->addAction(KisIconUtils::loadIcon("view-preview"), i18n("Thumbnails"));
    action->setCheckable(true);
    action->setChecked(m_mode == ListViewMode::IconGrid);
    action->setActionGroup(actionGroup);
    connect(action, SIGNAL(triggered()), this, SLOT(slotViewThumbnail()));

    action = viewModeMenu->addAction(KisIconUtils::loadIcon("view-list-details"), i18n("Details"));
    action->setCheckable(true);
    action->setChecked(m_mode == ListViewMode::Detail);
    action->setActionGroup(actionGroup);
    connect(action, SIGNAL(triggered()), this, SLOT(slotViewDetails()));

    setPopupWidget(viewModeMenu);
    setPopupMode(QToolButton::InstantPopup);
    setIcon(KisIconUtils::loadIcon("view-choose"));

    if (m_mode == ListViewMode::IconGrid) {
        slotViewThumbnail();
    } else {
        slotViewDetails();
    }

}

KisResourceItemViewer::~KisResourceItemViewer()
{
}

void KisResourceItemViewer::slotViewThumbnail()
{
    KisConfig cfg(false);
    if (m_type == Viewer::TableAvailable) {
        cfg.writeEntry("ResourceItemsBCSearch.viewMode", qint32(1));
    } else if (m_type == Viewer::ResourceManager) {
        cfg.writeEntry("ResourceItemsRM.viewMode", qint32(1));
    } else {
        cfg.writeEntry("ResourceItemsBCSelected.viewMode", qint32(1));
    }
    Q_EMIT onViewThumbnail();
}

void KisResourceItemViewer::slotViewDetails()
{
    KisConfig cfg(false);
    if (m_type == Viewer::TableAvailable) {
        cfg.writeEntry("ResourceItemsBCSearch.viewMode", qint32(0));
    } else if (m_type == Viewer::ResourceManager) {
        cfg.writeEntry("ResourceItemsRM.viewMode", qint32(0));
    } else {
        cfg.writeEntry("ResourceItemsBCSelected.viewMode", qint32(0));
    }
    Q_EMIT onViewDetails();
}

