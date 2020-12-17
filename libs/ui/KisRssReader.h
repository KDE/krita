/**************************************************************************
**
** This file is part of Qt Creator
**
** SPDX-FileCopyrightText: 2011 Nokia Corporation and /or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
**
** SPDX-License-Identifier: LGPL-2.1-only
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

#ifndef KISRSSREADER_H
#define KISRSSREADER_H

#include <QString>
#include <QDateTime>
#include <QXmlStreamReader>
#include <QNetworkReply>
#include <QFile>

#include <kritaui_export.h>

struct RssItem {
    QString source;
    QString title;
    QString link;
    QString description;
    QString category;
    QString blogName;
    QString blogIcon;
    QDateTime pubDate;

};
typedef QList<RssItem> RssItemList;

Q_DECLARE_METATYPE(RssItem);


class KRITAUI_EXPORT KisRssReader
{
public:
    KisRssReader();

    enum RssRoles {
        TitleRole = Qt::UserRole + 1,
        DescriptionRole,
        LinkRole,
        PubDateRole,
        CategoryRole,
        BlogNameRole,
        BlogIconRole
    };

    RssItem parseItem();
    RssItemList parseStream(QXmlStreamReader& streamReader);
    RssItemList parse(QNetworkReply *reply);
    RssItemList parse(QFile& file);

private:
    QXmlStreamReader m_streamReader;
    QString requestUrl;
    QString blogIcon;
    QString blogName;
};

#endif // KISRSSREADER_H
