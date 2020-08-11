/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGColorPreviewPopup.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QPainter>

WGColorPreviewPopup::WGColorPreviewPopup(QWidget *parent)
    : QWidget(parent)
    , m_color(0, 0, 0)
    , m_previousColor(0, 0, 0, 0)
    , m_lastUsedColor(0, 0, 0, 0)
{
    setWindowFlags(Qt::ToolTip | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(100, 150);
}

void WGColorPreviewPopup::updatePosition(const QWidget *focus)
{
    const QWidget *parent = focus ? focus : parentWidget();
    if (!parent) {
        return;
    }

    QPoint parentPos = parent->mapToGlobal(QPoint(0,0));
    const QRect availRect = QApplication::desktop()->availableGeometry(this);
    QPoint targetPos;
    if (parentPos.x() - width() > availRect.x()) {
        targetPos =  QPoint(parentPos.x() - width(), parentPos.y());
    } else if (parentPos.x() + parent->width() + width() < availRect.right()) {
        targetPos = parent->mapToGlobal(QPoint(parent->width(), 0));
    } else if (parentPos.y() - height() > availRect.y()) {
        targetPos =  QPoint(parentPos.x(), parentPos.y() - height());
    } else {
        targetPos =  QPoint(parentPos.x(), parentPos.y() + parent->height());
    }
    move(targetPos);
}

void WGColorPreviewPopup::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e)
    QPainter painter(this);
    painter.fillRect(0, 0, width(), width(), m_color);
    painter.fillRect(50, width(), width(), height(), m_previousColor);
    painter.fillRect(0, width(), 50, height(), m_lastUsedColor);
}
