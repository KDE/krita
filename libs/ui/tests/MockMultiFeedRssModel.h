/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef MOCKMULTIFEEDRSSMODEL_H
#define MOCKMULTIFEEDRSSMODEL_H

#include <QObject>

#include <KisMultiFeedRSSModel.h>

class MockMultiFeedRssModel : public MultiFeedRssModel
{
    Q_OBJECT

public:
    explicit MockMultiFeedRssModel(QObject *parent = 0);

    void addFeed(const QString& feed) override;

    /**
     * @brief to be called in the setup phase of the unittest, before call to addFeed
     * @param rssFile
     */
    void loadFeedData(const RssItemList& feed);
};

#endif // MOCKMULTIFEEDRSSMODEL_H
