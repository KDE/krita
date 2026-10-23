/*
 * SPDX-FileCopyrightText: 2018 boud <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2026 Arkady Flury <arkady.flury@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISNEWSWIDGET_H
#define KISNEWSWIDGET_H

#include <QWidget>
#include <QListView>
#include <QSet>
#include <QStyledItemDelegate>
#include <KisKineticScroller.h>

#include <ui_KisNewsPage.h>

class MultiFeedRssModel;

/**
 * @brief The KisNewsWidget class shows the latest news from Krita.org
 */
class KisNewsWidget : public QWidget, public Ui::KisNewsPage
{
    Q_OBJECT
public:
    explicit KisNewsWidget(QWidget *parent = nullptr);

    void toggleNewsLanguage(QString langCode, bool enabled);

Q_SIGNALS:
    void newsDataChanged();
    void newsStateChanged(bool hasNews);
    void setAnalyticsTracking(QString text);

protected:
    bool event(QEvent *event) override;

private Q_SLOTS:
    void toggleNews(bool toggle);
    void rssDataChanged();
    void dismissAll();
    void updateNewsState();
    void writeToConfig(QString link);

private:
    bool m_getNews {false};
    MultiFeedRssModel *m_rssModel {0};
    QSet<QString> m_enabledFeeds;

public Q_SLOTS:
    void slotScrollerStateChanged(QScroller::State state){ KisKineticScroller::updateCursor(this, state); }
};

#endif // KISNEWSWIDGET_H
