#ifndef MOCKMULTIFEEDRSSMODEL_H
#define MOCKMULTIFEEDRSSMODEL_H

#include <QObject>

#include <KisMultiFeedRSSModel.h>
#include "kritaui_export.h"

class KRITAUI_EXPORT MockMultiFeedRssModel : public MultiFeedRssModel
{
    Q_OBJECT

public:
    explicit MockMultiFeedRssModel(QObject *parent = 0);

    void addFeed(const QString& feed);

    /**
     * @brief to be called in the setup phase of the unittest, before call to addFeed
     * @param rssFile
     */
    void loadFeedData(const RssItemList& feed);
};

#endif // MOCKMULTIFEEDRSSMODEL_H
