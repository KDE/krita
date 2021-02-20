/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoCheckerBoardPainter.h"
#include <QPainter>

KoCheckerBoardPainter::KoCheckerBoardPainter(int checkerSize)
    : m_checkerSize(checkerSize)
    , m_lightColor(Qt::lightGray)
    , m_darkColor(Qt::darkGray)
{
    createChecker();
}

void KoCheckerBoardPainter::setCheckerColors(const QColor &lightColor, const QColor &darkColor)
{
    m_lightColor = lightColor;
    m_darkColor = darkColor;
    createChecker();
}

void KoCheckerBoardPainter::setCheckerSize(int checkerSize)
{
    m_checkerSize = checkerSize;
    createChecker();
}

void KoCheckerBoardPainter::paint(QPainter &painter, const QRectF &rect, const QPointF &patternOrigin) const
{
    QBrush brush(m_checker);
    brush.setTransform(QTransform::fromTranslate(patternOrigin.x(), patternOrigin.y()));
    painter.fillRect(rect, brush);
}

void KoCheckerBoardPainter::paint(QPainter &painter, const QRectF &rect) const
{
    paint(painter, rect, QPointF());
}

void KoCheckerBoardPainter::createChecker()
{
    m_checker = QPixmap(2 * m_checkerSize, 2 * m_checkerSize);
    QPainter p(&m_checker);
    p.fillRect(0, 0, m_checkerSize, m_checkerSize, m_lightColor);
    p.fillRect(m_checkerSize, 0, m_checkerSize, m_checkerSize, m_darkColor);
    p.fillRect(0, m_checkerSize, m_checkerSize, m_checkerSize, m_darkColor);
    p.fillRect(m_checkerSize, m_checkerSize, m_checkerSize, m_checkerSize, m_lightColor);
    p.end();
}
