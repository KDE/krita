// SPDX-License-Identifier: GPL-3.0-or-later
#include "KisSplashPreview.h"
#include <QPainter>

KisSplashPreview::KisSplashPreview(QWidget *parent)
    : QWidget(parent)
{
}

void KisSplashPreview::setPixmap(const QPixmap &pixmap)
{
    if (pixmap.cacheKey() != m_originalPixmap.cacheKey()) {
        m_originalPixmap = pixmap;
        m_scaledPixmap = QPixmap();
        update();
    }
}

void KisSplashPreview::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    if (!m_originalPixmap.isNull()) {
        qreal dpr = qMax(1.0, devicePixelRatioF());
        int w = width();
        int h = height();

        if (m_scaledPixmap.isNull()) {
            m_scaledPixmap = m_originalPixmap.scaled(qRound(qreal(w) * dpr),
                                                     qRound(qreal(h) * dpr),
                                                     Qt::KeepAspectRatio,
                                                     Qt::SmoothTransformation);
        }

        int pw = qRound(qreal(m_scaledPixmap.width()) / dpr);
        int ph = qRound(qreal(m_scaledPixmap.height()) / dpr);
        int x = (w - pw) / 2;
        int y = (h - ph) / 2;

        QPainter painter(this);
        painter.drawPixmap(QRect(x, y, pw, ph), m_scaledPixmap);
    }
}

void KisSplashPreview::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_scaledPixmap = QPixmap();
}
