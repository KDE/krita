/* This file is part of the KDE project
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ArtisticTextToolSelection.h"
#include "ArtisticTextShape.h"

#include <KoCanvasBase.h>
#include <KoViewConverter.h>

#include <KDebug>

#include <QtGui/QPainter>
#include <QtGui/QFontMetrics>

ArtisticTextToolSelection::ArtisticTextToolSelection(KoCanvasBase *canvas, QObject *parent)
    : KoToolSelection(parent), m_canvas(canvas), m_currentShape(0)
    , m_selectionStart(-1), m_selectionCount(0)
{
    Q_ASSERT(m_canvas);
}

ArtisticTextToolSelection::~ArtisticTextToolSelection()
{

}

bool ArtisticTextToolSelection::hasSelection()
{
    return m_currentShape && m_selectionCount > 0;
}

void ArtisticTextToolSelection::setSelectedShape(ArtisticTextShape *textShape)
{
    if (textShape == m_currentShape)
        return;
    clear();
    m_currentShape = textShape;
}

ArtisticTextShape *ArtisticTextToolSelection::selectedShape() const
{
    return m_currentShape;
}

void ArtisticTextToolSelection::selectText(int from, int to)
{
    if (!m_currentShape)
        return;

    repaintDecoration();

    const int textCount = m_currentShape->plainText().count();
    m_selectionStart = qBound(0, from, textCount-1);
    m_selectionCount = qBound(from, to, textCount) - m_selectionStart;

    repaintDecoration();
}

int ArtisticTextToolSelection::selectionStart() const
{
    return m_selectionStart;
}

int ArtisticTextToolSelection::selectionCount() const
{
    return m_selectionCount;
}

void ArtisticTextToolSelection::clear()
{
    repaintDecoration();
    m_selectionStart = -1;
    m_selectionCount = 0;
}

void ArtisticTextToolSelection::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (!hasSelection())
        return;

    m_currentShape->applyConversion( painter, converter );
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 255, 127));
    painter.drawPath(outline());
}

QPainterPath ArtisticTextToolSelection::outline()
{
    if (!hasSelection())
        return QPainterPath();

    CharIndex charPos = m_currentShape->indexOfChar(m_selectionStart);
    if (charPos.first < 0)
        return QPainterPath();

    QPainterPath outline;

    QPolygonF polygon;
    polygon.reserve(m_selectionCount*4);

    QList<ArtisticTextRange> ranges = m_currentShape->text();
    const int selectionEnd = m_selectionStart+m_selectionCount;
    for (int charIndex = m_selectionStart; charIndex <= selectionEnd; ++charIndex) {
        const QPointF pos = m_currentShape->charPositionAt(charIndex);
        const qreal angle = m_currentShape->charAngleAt(charIndex);

        QTransform charTransform;
        charTransform.translate( pos.x() - 1, pos.y() );
        charTransform.rotate( 360. - angle );

        QFontMetrics metrics(m_currentShape->fontAt(charIndex));

        polygon.prepend(charTransform.map(QPointF(0.0, -metrics.ascent())));
        polygon.append(charTransform.map(QPointF(0.0, metrics.descent())));

        if (charIndex == selectionEnd)
            break;

        const qreal w = metrics.charWidth(ranges[charPos.first].text(), charPos.second);

        // advance character index within current range
        charPos.second++;
        // are we at the end of the current range ?
        if (charPos.second >= ranges[charPos.first].text().length()) {
            // go to start of next range
            charPos.first++;
            charPos.second = 0;
        }
        // check if next character has an y-offset
        if (charPos.first < ranges.size() && ranges[charPos.first].hasYOffset(charPos.second)) {
            // end selection sub range and start a new subrange
            polygon.prepend(charTransform.map(QPointF(w, -metrics.ascent())));
            polygon.append(charTransform.map(QPointF(w, metrics.descent())));
            outline.addPolygon(polygon);
            polygon.clear();
        }
    }

    // if we have some points left, add them to the outline as well
    if (!polygon.isEmpty()) {
        outline.addPolygon(polygon);
    }

    // transform to document coordinates
    return m_currentShape->absoluteTransformation(0).map(outline);
}

void ArtisticTextToolSelection::repaintDecoration()
{
    if (hasSelection())
        m_canvas->updateCanvas(outline().boundingRect());
}
