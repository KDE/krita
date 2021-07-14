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

struct KisIconWidget::Private
{
    QImage thumbnail;
    KoResourceSP resource;
    QColor backgroundColor;
    QPixmap cachedIconPixmap;
    qint64 cachedResourceImageKey;
};

KisIconWidget::KisIconWidget(QWidget *parent, const QString &name)
    : KisPopupButton(parent)
    , m_d(new Private)
{
    m_d->backgroundColor = QColor(Qt::transparent);
    setObjectName(name);
    m_d->resource = nullptr;
}

KisIconWidget::~KisIconWidget()
{
    delete m_d;
}

void KisIconWidget::setThumbnail(const QImage &thumbnail)
{
    m_d->thumbnail = thumbnail;
    m_d->cachedIconPixmap = QPixmap();
    update();
}

void KisIconWidget::setResource(KoResourceSP resource)
{
    m_d->resource = resource;
    m_d->cachedIconPixmap = QPixmap();
    update();
}

void KisIconWidget::setBackgroundColor(const QColor &color)
{
    m_d->backgroundColor = color;
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

    auto makeIcon = [&](std::function<void (QPainter &)> paintFn) {
        QPixmap pixmap(iconWidth * devicePixelRatioF(), iconHeight * devicePixelRatioF());
        pixmap.setDevicePixelRatio(devicePixelRatioF());
        pixmap.fill(m_d->backgroundColor);
        QPainter p(&pixmap);

        // Round off the corners of the preview
        QRegion clipRegion(0, 0, iconWidth, iconHeight);
        clipRegion -= QRegion(0, 0, 1, 1);
        clipRegion -= QRegion(iconWidth - 1, 0, 1, 1);
        clipRegion -= QRegion(iconWidth - 1, iconHeight - 1, 1, 1);
        clipRegion -= QRegion(0, iconHeight - 1, 1, 1);

        p.setClipRegion(clipRegion);
        p.setClipping(true);
        paintFn(p);
        return pixmap;
    };

    auto paintThumbnailIcon = [&](QPainter &p) {
        QImage img = QImage(iconWidth*devicePixelRatioF(), iconHeight*devicePixelRatioF(), QImage::Format_ARGB32);
        img.setDevicePixelRatio(devicePixelRatioF());
        img.fill(m_d->backgroundColor);
        if (m_d->thumbnail.width() < iconWidth*devicePixelRatioF() || m_d->thumbnail.height() < iconHeight*devicePixelRatioF()) {
            QImage thumb = m_d->thumbnail.scaled(m_d->thumbnail.size()*devicePixelRatioF()); // first scale up to the high DPI so the pattern is visible
            thumb.setDevicePixelRatio(devicePixelRatioF());
            QPainter paint2;
            paint2.begin(&img);
            for (int x = 0; x < iconWidth; x += thumb.width()/devicePixelRatioF()) {
                for (int y = 0; y < iconHeight; y+= thumb.height()/devicePixelRatioF()) {
                    paint2.drawImage(x, y, thumb);
                }
            }
        } else {
            img = m_d->thumbnail.scaled(iconWidth*devicePixelRatioF(), iconHeight*devicePixelRatioF(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        p.drawImage(QRect(0, 0, iconWidth, iconHeight), img);
    };

    auto paintResourceIcon = [&](QPainter &p) {
        QImage img = QImage(iconWidth*devicePixelRatioF(), iconHeight*devicePixelRatioF(), QImage::Format_ARGB32);
        img.fill(m_d->backgroundColor);
        if (m_d->resource->image().width() < iconWidth*devicePixelRatioF() || m_d->resource->image().height() < iconHeight*devicePixelRatioF()) {
            QPainter paint2;
            paint2.begin(&img);
            for (int x = 0; x < iconWidth; x += m_d->resource->image().width()/devicePixelRatioF()) {
                for (int y = 0; y < iconHeight; y += m_d->resource->image().height()/devicePixelRatioF()) {
                    paint2.drawImage(x, y, m_d->resource->image());
                }
            }
        } else {
            img = m_d->resource->image().scaled(iconWidth*devicePixelRatioF(), iconHeight*devicePixelRatioF(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            img.setDevicePixelRatio(devicePixelRatioF());
        }
        p.drawImage(QRect(0, 0, iconWidth, iconHeight), img);
    };

    bool useCustomIcon = false;

    const bool isCachedPixmapOutdated = m_d->cachedIconPixmap.isNull()
        || m_d->cachedIconPixmap.width() != iconWidth
        || m_d->cachedIconPixmap.height() != iconHeight
        || m_d->cachedIconPixmap.devicePixelRatio() != devicePixelRatioF();

    if (!m_d->thumbnail.isNull()) {
        if (isCachedPixmapOutdated) {
            m_d->cachedIconPixmap = makeIcon(paintThumbnailIcon);
        }
        useCustomIcon = true;
    } else if (m_d->resource) {
        if (isCachedPixmapOutdated || m_d->cachedResourceImageKey != m_d->resource->image().cacheKey()) {
            m_d->cachedIconPixmap = makeIcon(paintResourceIcon);
        }
        useCustomIcon = true;
    }

    QStylePainter ps(this);
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    if (useCustomIcon) {
        opt.iconSize = QSize(iconWidth, iconHeight);
        opt.icon = QIcon(m_d->cachedIconPixmap);
        opt.toolButtonStyle = Qt::ToolButtonIconOnly;
    }
    ps.drawComplexControl(QStyle::CC_ToolButton, opt);
}

