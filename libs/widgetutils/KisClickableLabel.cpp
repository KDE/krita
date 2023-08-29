/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2023 Halla Rempt <halla@valdyas.org>
 * SPDX-FileCopyrightText: 2023 Alvin Wong <alvin@alvinhc.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KisClickableLabel.h"

#include <QResizeEvent>
#include <QtMath>

KisClickableLabel::KisClickableLabel(QWidget* parent)
    : QLabel(parent)
{
}

KisClickableLabel::~KisClickableLabel() {}

bool KisClickableLabel::hasHeightForWidth() const
{
    return true;
}

int KisClickableLabel::heightForWidth(int w) const
{
    if (m_pixmap.isNull()) {
        return height();
    }
    return qCeil(static_cast<qreal>(w) * m_pixmap.height() / m_pixmap.width());
}

QSize KisClickableLabel::minimumSizeHint() const
{
    return {};
}

QSize KisClickableLabel::sizeHint() const
{
    return {};
}

void KisClickableLabel::setUnscaledPixmap(QPixmap pixmap)
{
    m_pixmap = std::move(pixmap);
    setMaximumSize(m_pixmap.size());
    updatePixmap();
}

void KisClickableLabel::updatePixmap()
{
    if (!m_pixmap.isNull()) {
        // setPixmap(m_pixmap.scaled(width(), height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        setPixmap(m_pixmap.scaledToWidth(width(), Qt::SmoothTransformation));
    }
}

void KisClickableLabel::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    emit clicked();
}

void KisClickableLabel::resizeEvent(QResizeEvent */*event*/)
{
    updatePixmap();
}
