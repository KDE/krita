/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGColorPreviewToolTip.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QScreen>
#include <QPainter>

#include <cmath>

WGColorPreviewToolTip::WGColorPreviewToolTip(QWidget *parent)
    : QWidget(parent)
    , m_color(0, 0, 0)
    , m_previousColor(0, 0, 0, 0)
    , m_lastUsedColor(0, 0, 0, 0)
{
    setWindowFlags(Qt::ToolTip | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(100, 150);
    QString iconFile(":/dark_krita_tool_freehand.svg");
    if (QFile(iconFile).exists()) {
        m_brushIcon.addFile(iconFile, QSize(16, 16));
    }
    iconFile = ":/light_krita_tool_freehand.svg";
    if (QFile(iconFile).exists()) {
        m_brushIcon.addFile(iconFile, QSize(16, 16), QIcon::Normal, QIcon::On);
    }
}

void WGColorPreviewToolTip::updatePosition(const QWidget *focus)
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

qreal WGColorPreviewToolTip::estimateBrightness(QColor col)
{
    // a cheap approximation of luma assuming sRGB with gamma = 2.0
    return std::sqrt(col.redF() * col.redF() * 0.21 +
                     col.greenF() * col.greenF() * 0.71 +
                     col.blueF() * col.blueF() * 0.08);
}

void WGColorPreviewToolTip::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e)
    QPainter painter(this);
    painter.fillRect(0, 0, width(), width(), m_color);
    painter.fillRect(0, width(), width()/2, height(), m_previousColor);
    painter.fillRect(width()/2, width(), width(), height(), m_lastUsedColor);

    QPixmap icon;
    QWindow *window = windowHandle();
    if (window && m_lastUsedColor.alpha() > 0) {
        QIcon::State iconState;
        iconState = estimateBrightness(m_lastUsedColor) > 0.5 ? QIcon::Off : QIcon::On;
        icon = m_brushIcon.pixmap(window, QSize(16, 16), QIcon::Normal, iconState);
    }
    if (!icon.isNull()) {
        painter.drawPixmap(width() - 18, height() - 18, icon);
    }
}
