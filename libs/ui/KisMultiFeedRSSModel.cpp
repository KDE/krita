/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** Other Usage
**
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**************************************************************************/

#include "KisMultiFeedRSSModel.h"

#include <QTimer>
#include <QThread>
#include <QXmlStreamReader>
#include <QCoreApplication>
#include <QLocale>
#include <QFile>

#include <QNetworkRequest>
#include <QNetworkReply>
#include <KisNetworkAccessManager.h>

#include <KisRssReader.h>

MultiFeedRssModel::MultiFeedRssModel(QObject *parent) :
    QAbstractListModel(parent),
    m_networkAccessManager(new KisNetworkAccessManager),
    m_articleCount(0)
{
    initialize();
}

MultiFeedRssModel::MultiFeedRssModel(KisNetworkAccessManager* nam, QObject* parent)
    : QAbstractListModel(parent),
      m_networkAccessManager(nam),
      m_articleCount(0)
{
    initialize();
}


MultiFeedRssModel::~MultiFeedRssModel()
{
}

QHash<int, QByteArray> MultiFeedRssModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames[KisRssReader::RssRoles::TitleRole] = "title";
    roleNames[KisRssReader::RssRoles::DescriptionRole] = "description";
    roleNames[KisRssReader::RssRoles::PubDateRole] = "pubDate";
    roleNames[KisRssReader::RssRoles::LinkRole] = "link";
    roleNames[KisRssReader::RssRoles::CategoryRole] = "category";
    roleNames[KisRssReader::RssRoles::BlogNameRole] = "blogName";
    roleNames[KisRssReader::RssRoles::BlogIconRole] = "blogIcon";
    return roleNames;
}

void MultiFeedRssModel::addFeed(const QString& feed)
{
    if (m_sites.contains(feed)) {
        // do not add the feed twice
        return;
    }

    m_sites << feed;
    const QUrl feedUrl(feed);
    QMetaObject::invokeMethod(m_networkAccessManager, "getUrl",
                              Qt::QueuedConnection, Q_ARG(QUrl, feedUrl));
}

bool sortForPubDate(const RssItem& item1, const RssItem& item2)
{
    return item1.pubDate > item2.pubDate;
}

void MultiFeedRssModel::appendFeedData(QNetworkReply *reply)
{
    KisRssReader reader;
    m_aggregatedFeed.append(reader.parse(reply));
    sortAggregatedFeed();
    setArticleCount(m_aggregatedFeed.size());
    beginResetModel();
    endResetModel();

    emit feedDataChanged();
}

void MultiFeedRssModel::sortAggregatedFeed()
{
    std::sort(m_aggregatedFeed.begin(), m_aggregatedFeed.end(), sortForPubDate);
}

void MultiFeedRssModel::initialize()
{
    connect(m_networkAccessManager, SIGNAL(finished(QNetworkReply*)),
            SLOT(appendFeedData(QNetworkReply*)), Qt::QueuedConnection);
}

void MultiFeedRssModel::removeFeed(const QString &feed)
{
    QMutableListIterator<RssItem> it(m_aggregatedFeed);
    while (it.hasNext()) {
        RssItem item = it.next();
        if (item.source == feed)
            it.remove();
    }
    setArticleCount(m_aggregatedFeed.size());

    m_sites.removeOne(feed);
}

int MultiFeedRssModel::rowCount(const QModelIndex &) const
{
    return m_aggregatedFeed.size();
}

QVariant MultiFeedRssModel::data(const QModelIndex &index, int role) const
{

    RssItem item = m_aggregatedFeed.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
    {
        QString text = item.description.left(90).append("...");
        if (text.startsWith("<p>")) {
            text.insert(2, " style=\"margin-top: 4px\"");
        }
        return QString("<b><a href=\"" + item.link + "\">" + item.title + "</a></b>"
               "<br><small>(" + item.pubDate.toLocalTime().toString(Qt::DefaultLocaleShortDate) + ") "
               + text + "</small>");
    }
    case KisRssReader::RssRoles::TitleRole:
        return item.title;
    case KisRssReader::RssRoles::DescriptionRole:
        return item.description;
    case KisRssReader::RssRoles::PubDateRole:
        return item.pubDate.toString("dd-MM-yyyy hh:mm");
    case KisRssReader::RssRoles::LinkRole:
        return item.link;
    case KisRssReader::RssRoles::CategoryRole:
        return item.category;
    case KisRssReader::RssRoles::BlogNameRole:
        return item.blogName;
    case KisRssReader::RssRoles::BlogIconRole:
        return item.blogIcon;
    }

    return QVariant();
}
