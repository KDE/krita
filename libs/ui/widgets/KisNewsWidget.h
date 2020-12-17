/*
 * SPDX-FileCopyrightText: 2018 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISNEWSWIDGET_H
#define KISNEWSWIDGET_H

#include <QWidget>
#include <QListView>
#include <QSet>
#include <QStyledItemDelegate>

#include <ui_KisNewsPage.h>

class MultiFeedRssModel;

class KisNewsDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    KisNewsDelegate(QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

/**
 * @brief The KisNewsWidget class shows the latest news from Krita.org
 */
class KisNewsWidget : public QWidget, public Ui::KisNewsPage
{
    Q_OBJECT
public:
    explicit KisNewsWidget(QWidget *parent = nullptr);

    void setAnalyticsTracking(QString text);
    void toggleNewsLanguage(QString langCode, bool enabled);

Q_SIGNALS:
    void newsDataChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private Q_SLOTS:
    void toggleNews(bool toggle);
    void itemSelected(const QModelIndex &idx);
    void rssDataChanged();

private:
    bool m_getNews {false};
    MultiFeedRssModel *m_rssModel {0};
    QString m_analyticsTrackingParameters;
    QSet<QString> m_enabledFeeds;
};

#endif // KISNEWSWIDGET_H
