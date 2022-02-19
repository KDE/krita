/*
 * SPDX-FileCopyrightText: 2022 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "KisRepaintDebugger.h"

#include <KLocalizedString>

#include <QPaintDevice>
#include <QPainter>
#include <QPaintEvent>
#include <QThread>
#include <QApplication>
#include <QMessageBox>
#include <QDebug>

bool KisRepaintDebugger::enabled()
{
    static bool enabled = qEnvironmentVariableIntValue("KRITA_DEBUG_REPAINT") == 1;
    return enabled;
}

void KisRepaintDebugger::paint(QPaintDevice *paintDevice, const QRect &widgetRect)
{
    paint(paintDevice, &widgetRect, 1);
}

void KisRepaintDebugger::paint(QPaintDevice *paintDevice, const QVector<QRect> &widgetRects)
{
    paint(paintDevice, widgetRects.constData(), widgetRects.size());
}

void KisRepaintDebugger::paint(QPaintDevice *paintDevice, const QPaintEvent *event)
{
    paint(paintDevice, event->rect());
}

void KisRepaintDebugger::paintFull(QPaintDevice *pd)
{
    if (!enabled()) {
        return;
    }
    const QRect rect = QRectF(QPointF(), QSizeF(pd->width(), pd->height()) * pd->devicePixelRatioF())
            .toAlignedRect();
    paint(pd, &rect, 1);
}

void KisRepaintDebugger::paint(QPaintDevice *paintDevice, const QRect *widgetRects, size_t count)
{
    if (!enabled()) {
        return;
    }
    constexpr int ALPHA = 63;
    static QVector<QColor> colors {
        QColor(255, 0, 0, ALPHA),
        QColor(0, 255, 0, ALPHA),
        QColor(0, 0, 255, ALPHA),
        QColor(255, 255, 0, ALPHA),
        QColor(255, 0, 255, ALPHA),
        QColor(0, 255, 255, ALPHA),
    };
    m_colorIndex = (m_colorIndex + 1) % colors.size();
    QPainter gc(paintDevice);
    for (size_t i = 0; i < count; i++) {
        gc.fillRect(widgetRects[i], colors[m_colorIndex]);
    }
}
