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
    , m_getNews(false)
    , m_rssModel(0)
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
    m_analyticsTrackingParameters = text;
}

void KisNewsWidget::toggleNews(bool toggle)
{
    KisConfig cfg(false);
    cfg.writeEntry<bool>("FetchNews", toggle);

    if (toggle) {
//        m_rssModel->addFeed(QLatin1String("https://krita.org/en/feed/"));
        m_rssModel->addFeed(QLatin1String("http://localhost/feed.xml"));
    }
    else {
//        m_rssModel->removeFeed(QLatin1String("https://krita.org/en/feed/"));
        m_rssModel->removeFeed(QLatin1String("http://localhost/feed.xml"));
    }
}

void KisNewsWidget::itemSelected(const QModelIndex &idx)
{
    if (idx.isValid()) {
        QString link = idx.data(KisRssReader::RssRoles::LinkRole).toString();

        // append query string for analytics tracking if we set it
        if (m_analyticsTrackingParameters != "") {

            // use title in analytics query string
            QString linkTitle = idx.data(KisRssReader::RssRoles::TitleRole).toString();
            linkTitle = linkTitle.simplified(); // trims and makes 1 white space
            linkTitle = linkTitle.replace(" ", "");

            m_analyticsTrackingParameters = m_analyticsTrackingParameters.append(linkTitle);
            QDesktopServices::openUrl(QUrl(link.append(m_analyticsTrackingParameters)));

        } else {
            QDesktopServices::openUrl(QUrl(link));
        }


    }
}

void KisNewsWidget::rssDataChanged()
{
    emit newsDataChanged();
}
