/*
 *  SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
 *  SPDX-FileCopyrightText: 2003 Patrick Julien <freak@codepimps.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "widgets/kis_iconwidget.h"

#include <QPainter>
#include <QIcon>
#include <QStylePainter>
#include <QStyleOptionToolButton>
#include <KoResource.h>

KisIconWidget::KisIconWidget(QWidget *parent, const QString &name)
    : KisPopupButton(parent)
{
    setObjectName(name);
    m_resource = 0;
}

void KisIconWidget::setThumbnail(const QImage &thumbnail)
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
    const qint32 border = 3;
    const qint32 iconWidth = width() - (border*2);
    const qint32 iconHeight = height() - (border*2);

    bool useCustomIcon = false;

    QPixmap pixmap(iconWidth * devicePixelRatioF(), iconHeight * devicePixelRatioF());
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);

    // Round off the corners of the preview
    QRegion clipRegion(0, 0, iconWidth, iconHeight);
    clipRegion -= QRegion(0, 0, 1, 1);
    clipRegion -= QRegion(iconWidth - 1, 0, 1, 1);
    clipRegion -= QRegion(iconWidth - 1, iconHeight - 1, 1, 1);
    clipRegion -= QRegion(0, iconHeight - 1, 1, 1);

    p.setClipRegion(clipRegion);
    p.setClipping(true);

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
        p.drawImage(QRect(0, 0, iconWidth, iconHeight), img);
        useCustomIcon = true;
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
        p.drawImage(QRect(0, 0, iconWidth, iconHeight), img);
        useCustomIcon = true;
    }
    p.setClipping(false);
    p.end();

    QStylePainter ps(this);
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    if (useCustomIcon) {
        opt.iconSize = QSize(iconWidth, iconHeight);
        opt.icon = QIcon(pixmap);
        opt.toolButtonStyle = Qt::ToolButtonIconOnly;
    }
    ps.drawComplexControl(QStyle::CC_ToolButton, opt);
}

