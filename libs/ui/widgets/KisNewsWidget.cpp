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
#include <QRegularExpression>

#include "kis_config.h"
#include "KisMultiFeedRSSModel.h"

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
    doc.setDocumentMargin(6);
    doc.setHtml(optionCopy.text);
    doc.setTextWidth(optionCopy.rect.width());

    /// Painting item without text
    optionCopy.text = QString();
    style->drawControl(QStyle::CE_ItemViewItem, &optionCopy, painter);

    QAbstractTextDocumentLayout::PaintContext ctx;

    // Highlighting text if item is selected
    QColor textColor;
    if (optionCopy.state & QStyle::State_Selected) {
        textColor = optionCopy.palette.color(QPalette::Active, QPalette::HighlightedText);
    } else {
        textColor = optionCopy.palette.color(QPalette::Text);
    }
    ctx.palette.setColor(QPalette::Text, textColor);

    painter->translate(optionCopy.rect.left(), optionCopy.rect.top());
    QRect clip(0, 0, optionCopy.rect.width(), optionCopy.rect.height());
    ctx.clip = clip;
    doc.setPageSize(clip.size());
    doc.documentLayout()->draw(painter, ctx);

    painter->restore();
}

QSize KisNewsDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem optionCopy = option;
    initStyleOption(&optionCopy, index);

    QTextDocument doc;
    doc.setDocumentMargin(6);
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
    listNews->viewport()->setAutoFillBackground(false);
    listNews->installEventFilter(this);

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

bool KisNewsWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == listNews && event->type() == QEvent::Leave) {
        listNews->clearSelection();
        listNews->setCurrentIndex(QModelIndex());
    }
    return QWidget::eventFilter(watched, event);
}

void KisNewsWidget::toggleNewsLanguage(QString langCode, bool enabled)
{
    // Sanity check: Since the code is adding the language code directly into
    // the URL, this prevents any nasty surprises with malformed URLs.
    Q_FOREACH(const char &ch, langCode.toLatin1()) {
        bool isValidChar = (ch >= 'a' && ch <= 'z') || ch == '-';
        if (!isValidChar) {
            warnUI << "Ignoring attempt to toggle malformed news lang:" << langCode;
            return;
        }
    }

    QString feed = QStringLiteral("https://krita.org/%1/feed/").arg(langCode);
    if (enabled) {
        m_enabledFeeds.insert(feed);
        if (m_getNews) {
            m_rssModel->addFeed(feed);
        }
    } else {
        m_enabledFeeds.remove(feed);
        if (m_getNews) {
            m_rssModel->removeFeed(feed);
        }
    }
}

void KisNewsWidget::toggleNews(bool toggle)
{
    m_getNews = toggle;

    KisConfig cfg(false);
    cfg.writeEntry<bool>("FetchNews", toggle);

    Q_FOREACH(const QString &feed, m_enabledFeeds) {
        if (toggle) {
            m_rssModel->addFeed(feed);
        } else {
            m_rssModel->removeFeed(feed);
        }
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

