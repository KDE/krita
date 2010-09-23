/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>

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

#include "SelectionDecorator.h"

#include <KoShape.h>
#include <KoSelection.h>
#include <kstandarddirs.h>
#include <kglobal.h>

#define HANDLE_DISTANCE 10

K_GLOBAL_STATIC(QImage, s_rotateCursor)

KoFlake::Position SelectionDecorator::m_hotPosition = KoFlake::TopLeftCorner;

SelectionDecorator::SelectionDecorator(KoFlake::SelectionHandle arrows,
        bool rotationHandles, bool shearHandles)
: m_rotationHandles(rotationHandles)
, m_shearHandles(shearHandles)
, m_arrows(arrows)
, m_handleRadius( 3 )
{
    if(s_rotateCursor == 0) {
        s_rotateCursor->load(KStandardDirs::locate("lib", "flake/rotate.png"));
    }
}

void SelectionDecorator::setSelection(KoSelection *selection) {
    m_selection = selection;
}

void SelectionDecorator::setHandleRadius( int radius ) {
    m_handleRadius = radius;
}

void SelectionDecorator::setHotPosition( KoFlake::Position hotPosition )
{
    m_hotPosition = hotPosition;
}

KoFlake::Position SelectionDecorator::hotPosition()
{
    return m_hotPosition;
}

void SelectionDecorator::paint(QPainter &painter, const KoViewConverter &converter)
{
    QRectF handleArea;
    painter.save();

    // save the original painter transformation
    QTransform painterMatrix = painter.worldTransform();

    painter.setPen( Qt::green );
    bool editable=false;
    foreach (KoShape *shape, m_selection->selectedShapes(KoFlake::StrippedSelection)) {
        // apply the shape transformation on top of the old painter transformation
        painter.setWorldTransform( shape->absoluteTransformation(&converter) * painterMatrix );
        // apply the zoom factor
        KoShape::applyConversion( painter, converter );
        // draw the shape bounding rect
        painter.drawRect( QRectF( QPointF(), shape->size() ) );

        if (!shape->isGeometryProtected())
            editable = true;
    }

    if (m_selection->count() > 1) {
        // more than one shape selected, so we need to draw the selection bounding rect
        painter.setPen( Qt::blue );
        // apply the selection transformation on top of the old painter transformation
        painter.setWorldTransform(m_selection->absoluteTransformation(&converter) * painterMatrix);
        // apply the zoom factor
        KoShape::applyConversion(painter, converter);
        // draw the selection bounding rect
        painter.drawRect(QRectF(QPointF(), m_selection->size()));
        // save the selection bounding rect for later drawing the selection handles
        handleArea = QRectF(QPointF(), m_selection->size());
    } else if (m_selection->firstSelectedShape()) {
        // only one shape selected, so we compose the correct painter matrix
        painter.setWorldTransform(m_selection->firstSelectedShape()->absoluteTransformation(&converter) * painterMatrix);
        KoShape::applyConversion(painter, converter);
        // save the only selected shapes bounding rect for later drawing the handles
        handleArea = QRectF(QPointF(), m_selection->firstSelectedShape()->size());
    }

    painterMatrix = painter.worldTransform();
    painter.restore();

    // if we have no editable shape selected there is no need drawing the selection handles
    if (!editable)
        return;

    painter.save();

    painter.setTransform(QTransform());
    painter.setRenderHint( QPainter::Antialiasing, false );

    painter.setPen(Qt::black);
    painter.setBrush(Qt::yellow);

    QPolygonF outline = painterMatrix.map( handleArea );

    // the 8 move rects
    QRectF rect( QPointF(0.5,0.5), QSizeF(2*m_handleRadius,2*m_handleRadius) );
    rect.moveCenter(outline.value(0));
    painter.drawRect(rect);
    rect.moveCenter(outline.value(1));
    painter.drawRect(rect);
    rect.moveCenter(outline.value(2));
    painter.drawRect(rect);
    rect.moveCenter(outline.value(3));
    painter.drawRect(rect);
    rect.moveCenter((outline.value(0)+outline.value(1))/2);
    painter.drawRect(rect);
    rect.moveCenter((outline.value(1)+outline.value(2))/2);
    painter.drawRect(rect);
    rect.moveCenter((outline.value(2)+outline.value(3))/2);
    painter.drawRect(rect);
    rect.moveCenter((outline.value(3)+outline.value(0))/2);
    painter.drawRect(rect);

    // draw the hot position
    painter.setBrush(Qt::red);
    QPointF pos;
    switch( m_hotPosition ) {
    case KoFlake::TopLeftCorner: pos = handleArea.topLeft(); break;
    case KoFlake::TopRightCorner: pos = handleArea.topRight(); break;
    case KoFlake::BottomLeftCorner: pos = handleArea.bottomLeft(); break;
    case KoFlake::BottomRightCorner: pos = handleArea.bottomRight(); break;
    case KoFlake::CenteredPosition: pos = handleArea.center(); break;
    }
    rect.moveCenter( painterMatrix.map(pos ));
    painter.drawRect(rect);

    painter.restore();

#if 0
    // draw the move arrow(s)
    if(m_arrows != KoFlake::NoHandle && bounds.width() > 45 && bounds.height() > 45) {
        qreal x1,x2,y1,y2; // 2 is where the arrow head is
        switch(m_arrows) {
            case KoFlake::TopMiddleHandle:
                x1=bounds.center().x(); x2=x1; y2=bounds.y()+8; y1=y2+20;
                break;
            case KoFlake::TopRightHandle:
                x2=bounds.right()-8; x1=x2-20; y2=bounds.y()+8; y1=y2+20;
                break;
            case KoFlake::RightMiddleHandle:
                x2=bounds.right()-8; x1=x2-20; y1=bounds.center().y(); y2=y1;
                break;
            case KoFlake::BottomRightHandle:
                x2=bounds.right()-8; x1=x2-20; y2=bounds.bottom()-8; y1=y2-20;
                break;
            case KoFlake::BottomMiddleHandle:
                x1=bounds.center().x(); x2=x1; y2=bounds.bottom()-8; y1=y2-20;
                break;
            case KoFlake::BottomLeftHandle:
                x2=bounds.left()+8; x1=x2+20; y2=bounds.bottom()-8; y1=y2-20;
                break;
            case KoFlake::LeftMiddleHandle:
                x2=bounds.left()+8; x1=x2+20; y1=bounds.center().y(); y2=y1;
                break;
            default:
            case KoFlake::TopLeftHandle:
                x2=bounds.left()+8; x1=x2+20; y2=bounds.y()+8; y1=y2+20;
                break;
        }
        painter.drawLine(QLineF(x1, y1, x2, y2));
        //pen.setColor(Qt::white);
        //painter.setPen(pen);
        //painter.drawLine(QLineF(x1-1, y1-1, x2-1, y2-1));
    }

    QPointF border(HANDLE_DISTANCE, HANDLE_DISTANCE);
    bounds.adjust(-border.x(), -border.y(), border.x(), border.y());

    if(m_rotationHandles) {
        painter.save();
        painter.translate(bounds.x(), bounds.y());
        QRectF rect(QPointF(0,0), QSizeF(22, 22));
        painter.drawImage(rect, *s_rotateCursor, rect);
        painter.translate(bounds.width(), 0);
        painter.rotate(90);
        if(bounds.width() > 45 && bounds.height() > 45)
            painter.drawImage(rect, *s_rotateCursor, rect);
        painter.translate(bounds.height(), 0);
        painter.rotate(90);
        painter.drawImage(rect, *s_rotateCursor, rect);
        painter.translate(bounds.width(), 0);
        painter.rotate(90);
        if(bounds.width() > 45 && bounds.height() > 45)
            painter.drawImage(rect, *s_rotateCursor, rect);
        painter.restore();
    }

    /*if(m_shearHandles) {
        pen.setWidthF(0);
        painter.setPen(pen);
        QRectF rect(bounds.topLeft(), QSizeF(6, 6));
        rect.moveLeft(bounds.x() + bounds.width() /2 -3);
        painter.drawRect(rect);
        rect.moveBottom(bounds.bottom());
        painter.drawRect(rect);
        rect.moveLeft(bounds.left());
        rect.moveTop(bounds.top() + bounds.width() / 2 -3);
        painter.drawRect(rect);
        rect.moveRight(bounds.right());
        painter.drawRect(rect);
    } */
#endif
}

