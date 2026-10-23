/*
 * SPDX-FileCopyrightText: 2026 Arkady Flury <arkady.flury@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisNewsItem.h"
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>
#include <kis_painting_tweaks.h>
#include "kis_icon_utils.h"
#include <KisNewsWidget.h>
#include "kis_config.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>

KisNewsItem::KisNewsItem(QString title,QString date,QString description,QString link,bool isTop,QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    titleLabel->setText(title);
    dateLabel->setText(i18n("(")+date+i18n(")"));
    descriptionLablel->setText(description);
    webLink = link;

    QSizePolicy sizePolicy = notifyIcon->sizePolicy();

    sizePolicy.setRetainSizeWhenHidden(true);

    btnDismiss->setSizePolicy(sizePolicy);
    notifyIcon->setSizePolicy(sizePolicy);

    topDevider->setVisible(!isTop);

    notifyIcon->setIcon(KisIconUtils::loadIcon("notification-dot"));

    connect(btnDismiss, SIGNAL(clicked()), this, SLOT(dismiss()));
    connect(notifyIcon, SIGNAL(released()), this, SLOT(notificationReleased()));

    updateGeometry();
}


KisNewsItem::~KisNewsItem()
{

}


void KisNewsItem::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    if (this->rect().contains(this->mapFromGlobal(QCursor::pos()))) {
        openLink();
    }
}


void KisNewsItem::notificationReleased()
{
    if (this->rect().contains(notifyIcon->mapFromGlobal(QCursor::pos()))) {
        openLink();
    }
}


void KisNewsItem::openLink()
{
    dismiss();
    if (m_analyticsTrackingParameters != "") {

        // use title in analytics query string
        QString linkTitle = titleLabel->text();
        linkTitle = linkTitle.simplified(); // trims and makes 1 white space
        linkTitle = linkTitle.replace(" ", "");

        m_analyticsTrackingParameters = m_analyticsTrackingParameters.append(linkTitle);
        QDesktopServices::openUrl(QUrl(webLink.append(m_analyticsTrackingParameters)));

    } else {
        QDesktopServices::openUrl(QUrl(webLink));
    }
}

void KisNewsItem::setAnalyticsTracking(QString text)
{
    m_analyticsTrackingParameters = text;
}


void KisNewsItem::dismiss(bool log)
{
    btnDismiss->hide();
    notifyIcon->hide();

    QSizePolicy sizePolicy = notifyIcon->sizePolicy();
    sizePolicy.setHorizontalPolicy(QSizePolicy::Ignored);
    notifyIcon->setSizePolicy(sizePolicy);

    updateGeometry();

    if (log) {
        Q_EMIT writeToConfig(webLink);
        parentWidget()->updateGeometry();
    }

    Q_EMIT updateNewsState();
}
