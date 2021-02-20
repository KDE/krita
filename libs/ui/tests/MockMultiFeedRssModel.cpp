/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "MockMultiFeedRssModel.h"

#include <testutil.h>
#include <QFile>
#include <QObject>
#include <KisMultiFeedRSSModel.h>

MockMultiFeedRssModel::MockMultiFeedRssModel(QObject *parent)
    : MultiFeedRssModel(parent)
{

}

void MockMultiFeedRssModel::addFeed(const QString &feed)
{
    Q_UNUSED(feed);

    emit feedDataChanged();
}

void MockMultiFeedRssModel::loadFeedData(const RssItemList& feed)
{
    m_aggregatedFeed = feed;
    sortAggregatedFeed();
    setArticleCount(m_aggregatedFeed.size());
    beginResetModel();
    endResetModel();
}
