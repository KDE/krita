/*
 * SPDX-FileCopyrightText: 2018 boud <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2026 Arkady Flury <arkady.flury@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include <QScrollBar>
#include <KisNewsItem.h>

#include "kis_config.h"
#include "KisMultiFeedRSSModel.h"
#include <KisKineticScroller.h>

KisNewsWidget::KisNewsWidget(QWidget *parent)
    : QWidget(parent)
    , m_getNews(false)
    , m_rssModel(0)
{
    setupUi(this);

    QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(scrollArea);
    if (scroller) {
        connect(scroller, SIGNAL(stateChanged(QScroller::State)),
                this, SLOT(slotScrollerStateChanged(QScroller::State)));
    }


    m_rssModel = new MultiFeedRssModel(this);
    connect(m_rssModel, SIGNAL(feedDataChanged()), this, SLOT(rssDataChanged()), Qt::UniqueConnection);
}


bool KisNewsWidget::event(QEvent *event) {
    if (event->type() == QEvent::StyleChange) {
        QList<QWidget*> allChildren = scrollAreaWidgetContents->findChildren<QWidget*>();

        for (QWidget* child : allChildren) {
            child->setStyleSheet(this->styleSheet());
        }
    }
    return QWidget::event(event);
}

void KisNewsWidget::toggleNewsLanguage(QString langCode, bool enabled)
{
    // Sanity check: Since the code is adding the language code directly into
    // the URL, this prevents any nasty surprises with malformed URLs.
    Q_FOREACH(const char &ch, langCode.toLatin1()) {
        bool isValidChar = ((ch >= 'a' && ch <= 'z') || ch == '-' || ch == '@');
        if (!isValidChar) {
            warnUI << "Ignoring attempt to toggle malformed news lang:" << langCode;
            return;
        }
    }

    QString feed = QStringLiteral("https://krita.org/%1/index.xml").arg(langCode);
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


void KisNewsWidget::rssDataChanged()
{
    QLayout *layout = scrollAreaWidgetContents->layout();
    if (layout) {
        QLayoutItem *item;
        while ((item = layout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
    }

    KConfigGroup newsGroup = KSharedConfig::openConfig()->group("DismissedNews");

    for (int i = 0;i<m_rssModel->rowCount();i++){
        QModelIndex index = m_rssModel->index(i);

        QString link = index.data(KisRssReader::RssRoles::LinkRole).toString();

        KisNewsItem *newsItem = new KisNewsItem(index.data(KisRssReader::RssRoles::TitleRole).toString(),
                                                index.data(KisRssReader::RssRoles::PubDateRole).toString(),
                                                index.data(KisRssReader::RssRoles::DescriptionRole).toString().remove(QRegularExpression("<[^>]*>")).split( "\n" ).at(1),
                                                //gets first paragraph of description (all descriptions start with a "\n")
                                                link,
                                                i==0,//if top of list remove devider
                                                this);
        scrollAreaWidgetContents->layout()->addWidget(newsItem);

        connect(newsItem, SIGNAL(updateNewsState()), this, SLOT(updateNewsState()));
        connect(newsItem, SIGNAL(writeToConfig(QString)), this, SLOT(writeToConfig(QString)));
        connect(this, SIGNAL(setAnalyticsTracking(QString)), newsItem, SLOT(setAnalyticsTracking(QString)));

        if (newsGroup.readEntry<bool>(link,false)){
            newsItem->dismiss(false);
        }
    }

    scrollAreaWidgetContents->updateGeometry();

    updateNewsState();
    Q_EMIT newsDataChanged();
}

void KisNewsWidget::dismissAll()
{
    QList<QWidget*> allChildren = scrollAreaWidgetContents->findChildren<QWidget*>();

    for (QWidget* child : allChildren) {
        if (KisNewsItem* newsItem = qobject_cast<KisNewsItem*>(child)) {
            newsItem->dismiss();
        }
    }

    Q_EMIT newsStateChanged(false);

    scrollAreaWidgetContents->updateGeometry();
}

void KisNewsWidget::updateNewsState()
{
    bool hasNews = false;

    KConfigGroup newsGroup = KSharedConfig::openConfig()->group("DismissedNews");

    for (int i = 0;i<m_rssModel->rowCount();i++){
        QModelIndex index = m_rssModel->index(i);

        QString link = index.data(KisRssReader::RssRoles::LinkRole).toString();

        if (!newsGroup.readEntry<bool>(link,false)){
            hasNews = true;
            break;
        }
    }

    Q_EMIT newsStateChanged(hasNews);
}

void KisNewsWidget::writeToConfig(QString link)
{
    KConfigGroup newsGroup = KSharedConfig::openConfig()->group("DismissedNews");
    newsGroup.writeEntry<bool>(link, true);
}
