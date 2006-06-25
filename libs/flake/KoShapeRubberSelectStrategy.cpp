/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoShapeRubberSelectStrategy.h"

#include <QPainter>
#include <QMouseEvent>

#include "KoPointerEvent.h"
#include "KoShapeManager.h"
#include "KoSelection.h"
#include "KoCanvasBase.h"
#include "KoTool.h"

KoShapeRubberSelectStrategy::KoShapeRubberSelectStrategy( KoTool *tool, KoCanvasBase *canvas, const QPointF &clicked )
: KoInteractionStrategy(tool, canvas )
, m_selectRect(clicked, QSizeF(0, 0))
{
}


KoShapeRubberSelectStrategy::~KoShapeRubberSelectStrategy()
{
}


void KoShapeRubberSelectStrategy::paint( QPainter &painter, KoViewConverter &converter)
{
    painter.setRenderHint( QPainter::Antialiasing, false );

    QColor selectColor( Qt::blue ); // TODO make configurable
    selectColor.setAlphaF( 0.5 );
    QBrush sb( selectColor, Qt::SolidPattern );
    painter.setPen( QPen( sb, 0 ) );
    painter.setBrush( sb );
    QRectF paintRect = converter.documentToView(m_selectRect);
    if(painter.hasClipping())
        paintRect = paintRect.intersect(painter.clipRegion().boundingRect());
    painter.drawRect( paintRect);
}


void KoShapeRubberSelectStrategy::handleMouseMove(const QPointF &point, Qt::KeyboardModifiers modifiers) {
    QPointF old = m_selectRect.bottomRight();
    m_selectRect.setBottomRight( point );
/*
    +---------------|--+
    |               |  |    We need to figure out rects A and B based on the two points. BUT
    |          old  | A|    we need to do that even if the points are switched places
    |             \ |  |    (i.e. the rect got smaller) and even if the rect is mirrored
    +---------------+  |    in either the horizontal or vertical axis.
    |       B          |
    +------------------+
                        `- point
*/
    QPointF x1 = old;
    x1.setY(m_selectRect.topLeft().y());
    QRectF A(x1, QSizeF(point.x() - x1.x(), point.y() - x1.y()));
    A = A.normalized();
    m_canvas->updateCanvas(A);

    QPointF x2 = old;
    x2.setX(m_selectRect.topLeft().x());
    QRectF B(x2, QSizeF(point.x() - x2.x(), point.y() - x2.y()));
    B = B.normalized();
    m_canvas->updateCanvas(B);
}

void KoShapeRubberSelectStrategy::finishInteraction()
{
    KoSelection * selection = m_canvas->shapeManager()->selection();
    const QList<KoShape *> &shapes = m_canvas->shapeManager()->shapes();
    foreach ( KoShape * object, shapes )
    {
        if ( object->boundingRect().intersects( m_selectRect ) )
        {
            selection->select( object );
        }
    }
    m_parent->repaintDecorations();
    m_canvas->updateCanvas(m_selectRect.normalized());
}

const QRectF KoShapeRubberSelectStrategy::selectRect() const {
    return m_selectRect.normalized();
}
