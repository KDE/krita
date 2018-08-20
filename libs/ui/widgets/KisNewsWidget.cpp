/*
 * Copyright (c) 2018 boud <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "KisNewsWidget.h"

#include <QDesktopServices>
#include <QUrl>

#include "kis_config.h"
#include "KisMultiFeedRSSModel.h"

KisNewsWidget::KisNewsWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    KisConfig cfg(true);
    m_getNews = cfg.readEntry<bool>("FetchNews", false);
    chkShowNews->setChecked(m_getNews);
    connect(chkShowNews, SIGNAL(toggled(bool)), this, SLOT(toggleNews(bool)));
    m_rssModel = new MultiFeedRssModel(this);

    if (m_getNews) {
        m_rssModel->addFeed(QLatin1String("https://krita.org/en/feed/"));
    }
    else {
        m_rssModel->removeFeed(QLatin1String("https://krita.org/en/feed/"));
    }
    listNews->setModel(m_rssModel);
    connect(listNews, SIGNAL(clicked(const QModelIndex&)), this, SLOT(itemSelected(const QModelIndex&)));
}

void KisNewsWidget::toggleNews(bool toggle)
{
    KisConfig cfg(false);
    cfg.writeEntry<bool>("FetchNews", toggle);
    m_getNews = toggle;

    if (m_getNews) {
        m_rssModel->addFeed(QLatin1String("https://krita.org/en/feed/"));
    }
    else {
        m_rssModel->removeFeed(QLatin1String("https://krita.org/en/feed/"));
    }
}

void KisNewsWidget::itemSelected(const QModelIndex &idx)
{
    if (idx.isValid()) {
        QString link = idx.data(RssRoles::LinkRole).toString();
        QDesktopServices::openUrl(QUrl(link));
    }
}
