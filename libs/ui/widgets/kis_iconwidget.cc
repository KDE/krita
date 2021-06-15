/*
 *  SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
 *  SPDX-FileCopyrightText: 2003 Patrick Julien <freak@codepimps.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "widgets/kis_iconwidget.h"

#include <QPainter>
#include <QIcon>
#include <QStyleOption>
#include <KoResource.h>

KisIconWidget::KisIconWidget(QWidget *parent, const QString &name)
    : KisPopupButton(parent)
{
    setObjectName(name);
    m_resource = 0;
}

void KisIconWidget::KisIconWidget::setThumbnail(const QImage &thumbnail)
{
    m_thumbnail = thumbnail;
    update();
}

void KisIconWidget::setResource(KoResourceSP resource)
{
    m_resource = resource;
    update();
}

QSize KisIconWidget::preferredIconSize() const
{
    const qint32 cw = width();
    const qint32 ch = height();
    const qint32 border = 3;
    const qint32 iconWidth = cw - (border*2);
    const qint32 iconHeight = ch - (border*2);

    return QSize(iconWidth, iconHeight)*devicePixelRatioF();
}

void KisIconWidget::paintEvent(QPaintEvent *event)
{
    QToolButton::paintEvent(event);

    QPainter p;
    p.begin(this);

    const qint32 cw = width();
    const qint32 ch = height();
    const qint32 border = 3;
    const qint32 iconWidth = cw - (border*2);
    const qint32 iconHeight = ch - (border*2);

    // Round off the corners of the preview
    QRegion clipRegion(border, border, iconWidth, iconHeight);
    clipRegion -= QRegion(border, border, 1, 1);
    clipRegion -= QRegion(cw-border-1, border, 1, 1);
    clipRegion -= QRegion(cw-border-1, ch-border-1, 1, 1);
    clipRegion -= QRegion(border, ch-border-1, 1, 1);

    p.setClipRegion(clipRegion);
    p.setClipping(true);

    p.setBrush(this->palette().window());
    p.drawRect(QRect(0,0,cw,ch));

    if (!m_thumbnail.isNull()) {
        QImage img = QImage(iconWidth*devicePixelRatioF(), iconHeight*devicePixelRatioF(), QImage::Format_ARGB32);
        img.setDevicePixelRatio(devicePixelRatioF());
        img.fill(Qt::transparent);
        if (m_thumbnail.width() < iconWidth*devicePixelRatioF() || m_thumbnail.height() < iconHeight*devicePixelRatioF()) {
            QImage thumb = m_thumbnail.scaled(m_thumbnail.size()*devicePixelRatioF()); // first scale up to the high DPI so the pattern is visible
            thumb.setDevicePixelRatio(devicePixelRatioF());
            QPainter paint2;
            paint2.begin(&img);
            for (int x = 0; x < iconWidth; x += thumb.width()/devicePixelRatioF()) {
                for (int y = 0; y < iconHeight; y+= thumb.height()/devicePixelRatioF()) {
                    paint2.drawImage(x, y, thumb);
                }
            }
        } else {
            img = m_thumbnail.scaled(iconWidth*devicePixelRatioF(), iconHeight*devicePixelRatioF(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        p.drawImage(QRect(border, border, iconWidth, iconHeight), img);
    }
    else if (m_resource) {
        QImage img = QImage(iconWidth*devicePixelRatioF(), iconHeight*devicePixelRatioF(), QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        if (m_resource->image().width() < iconWidth*devicePixelRatioF() || m_resource->image().height() < iconHeight*devicePixelRatioF()) {
            QPainter paint2;
            paint2.begin(&img);
            for (int x = 0; x < iconWidth; x += m_resource->image().width()/devicePixelRatioF()) {
                for (int y = 0; y < iconHeight; y += m_resource->image().height()/devicePixelRatioF()) {
                    paint2.drawImage(x, y, m_resource->image());
                }
            }
        } else {
            img = m_resource->image().scaled(iconWidth*devicePixelRatioF(), iconHeight*devicePixelRatioF(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            img.setDevicePixelRatio(devicePixelRatioF());
        }
        p.drawImage(QRect(border, border, iconWidth, iconHeight), img);
    } else if (!icon().isNull()) {
        QSize size = QSize(22, 22);
        int border2 = qRound((cw - size.rwidth()) * 0.5);
        QImage image = icon().pixmap(size).toImage();
        p.drawImage(QRect(border2, border2, size.rwidth(), size.rheight()), image);
    }
    p.setClipping(false);
}

