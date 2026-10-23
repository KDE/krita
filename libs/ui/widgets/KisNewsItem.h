/*
 * SPDX-FileCopyrightText: 2026 Arkady Flury <arkady.flury@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISNEWSITEM_H
#define KISNEWSITEM_H

#include <QWidget>
#include "kritaui_export.h"


#include <ui_KisNewsItem.h>

class KRITAUI_EXPORT KisNewsItem : public QWidget, public Ui::KisNewsItem
{
    Q_OBJECT
public:
    explicit KisNewsItem(QString title,QString date,QString description,QString link,bool isTop,QWidget *parent = nullptr);
    ~KisNewsItem() override;

Q_SIGNALS:
    void updateNewsState();
    void writeToConfig(QString link);

public Q_SLOTS:
    void dismiss(bool log=true);
    void setAnalyticsTracking(QString text);

private Q_SLOTS:
    void notificationReleased();

private:
    QString webLink;
    QString m_analyticsTrackingParameters;
    void openLink();

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
};



#endif // KISNEWSITEM_H