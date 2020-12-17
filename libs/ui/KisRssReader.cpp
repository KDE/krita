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

#include "KisRssReader.h"

#include <QString>
#include <QFile>
#include <QXmlStreamReader>
#include <QUrl>

QString shortenHtml(QString html)
{
    html.replace(QLatin1String("<a"), QLatin1String("<i"));
    html.replace(QLatin1String("</a"), QLatin1String("</i"));
    uint firstParaEndXhtml = (uint) html.indexOf(QLatin1String("</p>"));
    uint firstParaEndHtml = (uint) html.indexOf(QLatin1String("<p>"), html.indexOf(QLatin1String("<p>"))+1);
    uint firstParaEndBr = (uint) html.indexOf(QLatin1String("<br"));
    uint firstParaEnd = qMin(firstParaEndXhtml, firstParaEndHtml);
    firstParaEnd = qMin(firstParaEnd, firstParaEndBr);
    return html.left(firstParaEnd);
}


KisRssReader::KisRssReader()
{

}

RssItem KisRssReader::parseItem() {
    RssItem item;
    item.source = requestUrl;
    item.blogIcon = blogIcon;
    item.blogName = blogName;
    while (!m_streamReader.atEnd()) {
        switch (m_streamReader.readNext()) {
        case QXmlStreamReader::StartElement:
            if (m_streamReader.name() == QLatin1String("title"))
                item.title = m_streamReader.readElementText();
            else if (m_streamReader.name() == QLatin1String("link"))
                item.link = m_streamReader.readElementText();
            else if (m_streamReader.name() == QLatin1String("pubDate")) {
                QString dateStr = m_streamReader.readElementText();
                item.pubDate = QDateTime::fromString(dateStr, Qt::RFC2822Date);
            }
            else if (m_streamReader.name() == QLatin1String("category"))
                item.category = m_streamReader.readElementText();
            else if (m_streamReader.name() == QLatin1String("description"))
                item.description = m_streamReader.readElementText(); //shortenHtml(streamReader.readElementText());
            break;
        case QXmlStreamReader::EndElement:
            if (m_streamReader.name() == QLatin1String("item"))
                return item;
            break;
        default:
            break;

        }
    }
    return RssItem();
}

RssItemList KisRssReader::parseStream(QXmlStreamReader &streamReader) {
    RssItemList list;
    while (!streamReader.atEnd()) {
        switch (streamReader.readNext()) {
        case QXmlStreamReader::StartElement:
            if (streamReader.name() == QLatin1String("item"))
                list.append(parseItem());
            else if (streamReader.name() == QLatin1String("title"))
                blogName = streamReader.readElementText();
            else if (streamReader.name() == QLatin1String("link")) {
                if (!streamReader.namespaceUri().isEmpty())
                    break;
                QString favIconString(streamReader.readElementText());
                QUrl favIconUrl(favIconString);
                favIconUrl.setPath(QLatin1String("favicon.ico"));
                blogIcon = favIconUrl.toString();
                blogIcon = QString(); // XXX: fix the favicon on krita.org!
            }
            break;
        default:
            break;
        }
    }
    return list;
}

RssItemList KisRssReader::parse(QNetworkReply *reply) {
    QUrl source = reply->request().url();
    requestUrl = source.toString();
    m_streamReader.setDevice(reply);

    return parseStream(m_streamReader);
}

RssItemList KisRssReader::parse(QFile &file) {
    requestUrl = file.fileName();
    file.open(QIODevice::ReadOnly);
    m_streamReader.setDevice(&file);

    RssItemList itemList(parseStream(m_streamReader));

    file.close();
    return itemList;
}
