/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2025 Agata Cacko <cacko.azh@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisColorSamplerPreviewPreview.h"

#include <QPainter>
#include <QPainterPath>

#include <kis_global.h>
#include <kis_algebra_2d.h>
#include <kis_painting_tweaks.h>

#include <QDebug>


KisColorSamplerPreviewPreview::KisColorSamplerPreviewPreview(QWidget *parent)
    : QLabel(parent)
{
}

void KisColorSamplerPreviewPreview::setDiameter(int diameter)
{
    m_diameter = diameter;
    repaint();
}

void KisColorSamplerPreviewPreview::setOutlineEnabled(bool enabled)
{
    m_outlineEnabled = enabled;
    repaint();
}

void KisColorSamplerPreviewPreview::setThickness(qreal thickness)
{
    m_thickness = thickness;
    repaint();
}

void KisColorSamplerPreviewPreview::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);


    QPainter painter(this);
    painter.save();

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::transparent);


    QPainterPath edge;

    QRect viewRect = rect();
    viewRect.adjust(m_horizontalWidgetMargins, 0, -m_horizontalWidgetMargins, 0);


    QRect edgeRect = viewRect;
    QRect edgeRectSmaller = kisGrowRect(viewRect, -1);


    edge.addRoundedRect(edgeRect, m_roundingMargin, m_roundingMargin);

    QColor outsideColor = palette().base().color();
    painter.setBrush(outsideColor);

    painter.fillPath(edge, outsideColor);


    int maxWidth = viewRect.width();
    int thicknessPixels = m_thickness*m_diameter;
    int innerWidth = m_diameter - 2*thicknessPixels;

    int outsideWidths = maxWidth - innerWidth - 2*thicknessPixels;
    int outsideWidth = outsideWidths/2;

    // if there are any inaccuracies, no worries, it doesn't have to be exact

    QRect leftSide = QRect(QPoint(), QPoint(thicknessPixels, viewRect.height()));
    leftSide.translate(viewRect.topLeft());
    QRect rightSide = leftSide;
    leftSide.translate(outsideWidth, 0);
    rightSide.translate(maxWidth - outsideWidth - thicknessPixels, 0);

    QRect leftBottom = QRect(QPoint(leftSide.left(), leftSide.top() + leftSide.height()/2), QPoint(leftSide.bottomRight()));
    QRect leftTop = QRect(leftSide.topLeft(), QPoint(leftSide.right(), leftSide.top() + leftSide.height()/2));

    QRect rightBottom = QRect(QPoint(rightSide.left(), rightSide.top() + rightSide.height()/2), QPoint(rightSide.bottomRight()));
    QRect rightTop = QRect(rightSide.topLeft(), QPoint(rightSide.right(), rightSide.top() + rightSide.height()/2));

    painter.setBrush(m_sampledColorTop);
    painter.fillRect(leftTop, m_sampledColorTop);
    painter.fillRect(leftBottom, m_sampledColorBottom);

    painter.fillRect(rightTop, m_sampledColorTop);
    painter.fillRect(rightBottom, m_sampledColorBottom);

    painter.setBrush(Qt::transparent);
    if (m_outlineEnabled) {
        painter.setPen(QPen(m_outlineColor, 2));
        painter.drawRect(leftSide.adjusted(0, -m_verticalOutlineMargins, 0, m_verticalOutlineMargins));
        painter.drawRect(rightSide.adjusted(0, -m_verticalOutlineMargins, 0, m_verticalOutlineMargins));
    }

    QColor crossColor = Qt::white;
    crossColor.setAlphaF(0.4);
    painter.setPen(crossColor);

    QPointF center = viewRect.topLeft() + QPoint(viewRect.width()/2, viewRect.height()/2);
    painter.drawLine(center - QPointF(m_crossLength, 0), center - QPointF(m_crossSpace, 0));
    painter.drawLine(center + QPointF(m_crossLength, 0), center + QPointF(m_crossSpace, 0));

    painter.drawLine(center - QPointF(0, m_crossLength), center - QPointF(0, m_crossSpace));
    painter.drawLine(center + QPointF(0, m_crossLength), center + QPointF(0, m_crossSpace));


    edge.clear();
    QColor darkTransparent = palette().dark().color();
    darkTransparent = KisPaintingTweaks::blendColors(outsideColor, darkTransparent, 0.6);

    painter.setPen(QPen(darkTransparent, 2));
    edge.addRoundedRect(edgeRectSmaller, m_roundingMargin, m_roundingMargin);
    painter.drawPath(edge);

    painter.restore();

}
