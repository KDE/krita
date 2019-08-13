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
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

#include "kis_config.h"
#include "KisMultiFeedRSSModel.h"
#include "QRegularExpression"


KisNewsDelegate::KisNewsDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void KisNewsDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    QStyleOptionViewItem optionCopy = option;
    initStyleOption(&optionCopy, index);

    QStyle *style = optionCopy.widget? optionCopy.widget->style() : QApplication::style();

    QTextDocument doc;
    doc.setHtml(optionCopy.text);
    doc.setDocumentMargin(10);

    /// Painting item without text
    optionCopy.text = QString();
    style->drawControl(QStyle::CE_ItemViewItem, &optionCopy, painter);

    QAbstractTextDocumentLayout::PaintContext ctx;

    // Highlighting text if item is selected
    if (optionCopy.state & QStyle::State_Selected) {
        ctx.palette.setColor(QPalette::Text, optionCopy.palette.color(QPalette::Active, QPalette::HighlightedText));
    }

    painter->translate(optionCopy.rect.left(), optionCopy.rect.top());
    QRect clip(0, 0, optionCopy.rect.width(), optionCopy.rect.height());
    doc.setPageSize(clip.size());
    doc.drawContents(painter, clip);
    painter->restore();
}

QSize KisNewsDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem optionCopy = option;
    initStyleOption(&optionCopy, index);

    QTextDocument doc;
    doc.setHtml(optionCopy.text);
    doc.setTextWidth(optionCopy.rect.width());
    return QSize(doc.idealWidth(), doc.size().height());
}

KisNewsWidget::KisNewsWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    m_rssModel = new MultiFeedRssModel(this);
    connect(m_rssModel, SIGNAL(feedDataChanged()), this, SLOT(rssDataChanged()));


    setCursor(Qt::PointingHandCursor);

    listNews->setModel(m_rssModel);
    listNews->setItemDelegate(new KisNewsDelegate(listNews));
    connect(listNews, SIGNAL(clicked(QModelIndex)), this, SLOT(itemSelected(QModelIndex)));
}

void KisNewsWidget::setAnalyticsTracking(QString text)
{
    analyticsTrackingParameters = text;
}

void KisNewsWidget::toggleNews(bool toggle)
{
    KisConfig cfg(false);
    cfg.writeEntry<bool>("FetchNews", toggle);

    if (toggle) {
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

        // append query string for analytics tracking if we set it
        if (analyticsTrackingParameters != "") {

            // use title in analytics query string
            QString linkTitle = idx.data(RssRoles::TitleRole).toString();
            linkTitle = linkTitle.simplified(); // trims and makes 1 white space
            linkTitle = linkTitle.replace(" ", "");

            analyticsTrackingParameters = analyticsTrackingParameters.append(linkTitle);
            QDesktopServices::openUrl(QUrl(link.append(analyticsTrackingParameters)));

        } else {
            QDesktopServices::openUrl(QUrl(link));
        }


    }
}

void KisNewsWidget::rssDataChanged()
{

    // grab the latest release post and URL for reference later
    // if we need to update
    for (int i = 0; i < m_rssModel->rowCount(); i++)
    {
       const QModelIndex &idx = m_rssModel->index(i);

       if (idx.isValid()) {

           // only use official release announcements to get version number
           if ( idx.data(RssRoles::CategoryRole).toString() !=  "Official Release") {
               continue;
           }

           QString linkTitle = idx.data(RssRoles::TitleRole).toString();

           // come up with a regex pattern to find version number
           QRegularExpression versionRegex("\\d\\.\\d\\.?\\d?\\.?\\d");

           QRegularExpressionMatch matched = versionRegex.match(linkTitle);

           // only take the top match for release version since that is the newest
           if (matched.hasMatch()) {
               newVersionNumber = matched.captured(0);
               newVersionLink = idx.data(RssRoles::LinkRole).toString();
               break;
           }

       }
    }

    // see if we need to update our version, or we are on a dev version
    calculateVersionUpdateStatus();
}

void KisNewsWidget::calculateVersionUpdateStatus()
{
    // do version compare to see if there is a new version available
    // also check to see if we are on a dev version (newer than newest release)
    QStringList currentVersionParts = qApp->applicationVersion().split(".");
    QStringList onlineReleaseAnnouncement = newVersionNumber.split(".");

    // is the major version different?
    if (onlineReleaseAnnouncement[0] > currentVersionParts[0] ) {
        needsVersionUpdate = true; // we are a major version behind
        return;
    }
    else if (onlineReleaseAnnouncement[0] < currentVersionParts[0] ) {
        isDevelopmentVersion = true;
        return;
    }

    // major versions are the same, so check minor versions
     if (onlineReleaseAnnouncement[1] > currentVersionParts[1] ) {
         needsVersionUpdate = true; // we are a minor version behind
         return;
     }
     else if (onlineReleaseAnnouncement[1] < currentVersionParts[1] ) {
         isDevelopmentVersion = true;
         return;
     }

     // minor versions are the same, so maybe bugfix version is different
     // sometimes we don't communicate this, implictly make 0 if it doesn't exist
     if (onlineReleaseAnnouncement[2].isNull()) {
         onlineReleaseAnnouncement[2] = "0";
     }
     if (currentVersionParts[2].isNull()) {
         currentVersionParts[2] = "0";
     }

     if (onlineReleaseAnnouncement[2] > currentVersionParts[2] ) {
         needsVersionUpdate = true; // we are a bugfix version behind
         return;
     }
     else if (onlineReleaseAnnouncement[2] < currentVersionParts[2] ) {
         isDevelopmentVersion = true;
         return;
     }
}
