/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2009 Carlos Licea <carlos.licea@kdemail.net>
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

#include "GuidesTool.h"
#include "GuidesToolOptionWidget.h"
#include "InsertGuidesToolOptionWidget.h"

#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoCanvasResourceProvider.h>
#include <KoViewConverter.h>
#include <KoGuidesData.h>

#include <QtGui/QPainter>

class GuidesTool::Private
{
public:
    enum EditMode { None, AddGuide, MoveGuide, EditGuide };
    Private() : orientation(Qt::Horizontal), index(-1), position(0), mode(None), options(0)
        , isMoving(false)
    {}
    Qt::Orientation orientation;
    int index;
    qreal position;
    EditMode mode;
    GuidesToolOptionWidget * options;
    InsertGuidesToolOptionWidget * insert;
    bool isMoving;
};

GuidesTool::GuidesTool( KoCanvasBase * canvas )
    : KoGuidesTool( canvas ), d( new Private() )
{
}

GuidesTool::~GuidesTool()
{
    delete d;
}

void GuidesTool::paint( QPainter &painter, const KoViewConverter &converter )
{
    if( d->mode == Private::None )
        return;

    if( d->mode == Private::EditGuide && d->index == -1 )
        return;

    KoCanvasController * controller = m_canvas->canvasController();
    QPoint documentOrigin = m_canvas->documentOrigin();
    QPoint canvasOffset( controller->canvasOffsetX(), controller->canvasOffsetY() );

    QPointF start, end;
    if( d->orientation == Qt::Horizontal )
    {
        qreal left = -canvasOffset.x() - documentOrigin.x();
        qreal right = left + m_canvas->canvasWidget()->width();
        start = QPointF( left, converter.documentToViewY( d->position ) );
        end = QPointF( right, converter.documentToViewY( d->position ) );
    }
    else
    {
        qreal top = -canvasOffset.y() - documentOrigin.y();
        qreal bottom = top + m_canvas->canvasWidget()->height();
        start = QPointF( converter.documentToViewX( d->position ), top );
        end = QPointF( converter.documentToViewX( d->position ), bottom );
    }
    painter.setPen( Qt::red );
    painter.drawLine( start, end );
}

void GuidesTool::repaintDecorations()
{
    if( d->mode == Private::None )
        return;

    QRectF rect;
    KoCanvasController * controller = m_canvas->canvasController();
    QPoint documentOrigin = m_canvas->documentOrigin();
    QPoint canvasOffset( controller->canvasOffsetX(), controller->canvasOffsetY() ); 
    if( d->orientation == Qt::Horizontal )
    {
        qreal pixelBorder = m_canvas->viewConverter()->viewToDocumentY( 2.0 );
        rect.setTop( d->position - pixelBorder );
        rect.setBottom( d->position + pixelBorder );
        rect.setLeft( m_canvas->viewConverter()->viewToDocumentX( -canvasOffset.x()-documentOrigin.x() ) );
        rect.setWidth( m_canvas->viewConverter()->viewToDocumentX( m_canvas->canvasWidget()->width() ) );
    }
    else
    {
        qreal pixelBorder = m_canvas->viewConverter()->viewToDocumentX( 2.0 );
        rect.setLeft( d->position - pixelBorder );
        rect.setRight( d->position + pixelBorder );
        rect.setTop( m_canvas->viewConverter()->viewToDocumentY( -canvasOffset.y()-documentOrigin.y() ) );
        rect.setHeight( m_canvas->viewConverter()->viewToDocumentY( m_canvas->canvasWidget()->height() ) );
    }
    m_canvas->updateCanvas( rect );
}

void GuidesTool::activate(bool temporary)
{
    Q_UNUSED(temporary);

    if( d->mode != Private::None )
        useCursor( d->orientation == Qt::Horizontal ? Qt::SizeVerCursor : Qt::SizeHorCursor, true );
    else
        useCursor( Qt::ArrowCursor );
    if( temporary )
        m_canvas->canvasWidget()->grabMouse();

    if( d->options )
    {
        KoGuidesData * guidesData = m_canvas->guidesData();
        if( ! guidesData )
            return;
        d->options->setHorizontalGuideLines( guidesData->horizontalGuideLines() );
        d->options->setVerticalGuideLines( guidesData->verticalGuideLines() );
        d->options->selectGuideLine( d->orientation, d->index );
        d->options->setUnit( m_canvas->unit() );
    }
}

void GuidesTool::deactivate()
{
    m_canvas->canvasWidget()->releaseMouse();
}

void GuidesTool::mousePressEvent( KoPointerEvent *event )
{
    GuideLine line = guideLineAtPosition( event->point );
    if( line.second >= 0 )
    {
        guideLineSelected( line.first, static_cast<uint>( line.second ) );
        d->isMoving = true;
    }
}

void GuidesTool::mouseMoveEvent( KoPointerEvent *event )
{
    if( d->mode == Private::None )
    {
        useCursor( Qt::ArrowCursor );
        return;
    }

    if( d->mode == Private::EditGuide && ! d->isMoving )
    {
        GuideLine line = guideLineAtPosition( event->point );
        if( line.second < 0 )
            useCursor( Qt::ArrowCursor );
        else
            useCursor( line.first == Qt::Horizontal ? Qt::SizeVerCursor : Qt::SizeHorCursor );
    }
    else
    {
        repaintDecorations();
        d->position = d->orientation == Qt::Horizontal ? event->point.y() : event->point.x();
        updateGuidePosition( d->position );
        repaintDecorations();
    }
}

void GuidesTool::mouseReleaseEvent( KoPointerEvent *event )
{
    Q_UNUSED(event);

    KoGuidesData * guidesData = m_canvas->guidesData();
    if( ! guidesData )
    {
        event->ignore();
        return;
    }

    if( d->mode == Private::AddGuide )
    {
        // add the new guide line
        guidesData->addGuideLine( d->orientation, d->position );
    }
    else if( d->mode == Private::EditGuide )
    {
        if( d->isMoving )
        {
            d->isMoving = false;
            if( d->orientation == Qt::Horizontal )
                d->options->setHorizontalGuideLines( guidesData->horizontalGuideLines() );
            else
                d->options->setVerticalGuideLines( guidesData->verticalGuideLines() );
            d->options->selectGuideLine( d->orientation, d->index );
        }
    }

    if( d->mode != Private::EditGuide )
        emit done();
}

void GuidesTool::mouseDoubleClickEvent( KoPointerEvent *event )
{
    KoGuidesData * guidesData = m_canvas->guidesData();
    if( ! guidesData )
    {
        event->ignore();
        return;
    }

    repaintDecorations();

    // get guide line at position
    GuideLine line = guideLineAtPosition( event->point );
    if( line.second < 0 )
    {
        // no guide line hit -> insert a new one
        d->orientation = d->options->orientation();
        d->position = d->orientation == Qt::Horizontal ? event->point.y() : event->point.x();
        // no guide line hit -> insert a new one
        guidesData->addGuideLine( d->orientation, d->position );
        if( d->orientation == Qt::Horizontal )
        {
            d->options->setHorizontalGuideLines( guidesData->horizontalGuideLines() );
            d->index = guidesData->horizontalGuideLines().count()-1;
        }
        else
        {
            d->options->setVerticalGuideLines( guidesData->verticalGuideLines() );
            d->index = guidesData->verticalGuideLines().count()-1;
        }
        d->options->selectGuideLine( d->orientation, d->index );
    }
    else
    {
        // guide line hit -> remove it
        QList<qreal> lines;
        if( line.first == Qt::Horizontal )
        {
            lines = guidesData->horizontalGuideLines();
            lines.removeAt( line.second );
            guidesData->setHorizontalGuideLines( lines );
            d->options->setHorizontalGuideLines( lines );
            d->index = -1;
        }
        else
        {
            lines = guidesData->verticalGuideLines();
            lines.removeAt( line.second );
            guidesData->setVerticalGuideLines( lines );
            d->options->setVerticalGuideLines( lines );
            d->index = -1;
        }
    }

    repaintDecorations();
}

void GuidesTool::addGuideLine( Qt::Orientation orientation, qreal position )
{
    d->orientation = orientation;
    d->index = -1;
    d->position = position;
    d->mode = Private::AddGuide;
}

void GuidesTool::moveGuideLine( Qt::Orientation orientation, uint index )
{
    d->orientation = orientation;
    d->index = index;
    d->mode = Private::MoveGuide;
}

void GuidesTool::editGuideLine( Qt::Orientation orientation, uint index )
{
    d->orientation = orientation;
    d->index = index;
    d->mode = Private::EditGuide;
}

void GuidesTool::updateGuidePosition( qreal position )
{
    if( d->mode == Private::MoveGuide || d->mode == Private::EditGuide )
    {
        KoGuidesData * guidesData = m_canvas->guidesData();
        if( guidesData )
        {
            if( d->orientation == Qt::Horizontal )
            {
                QList<qreal> guideLines = guidesData->horizontalGuideLines();
                guideLines[d->index] = position;
                guidesData->setHorizontalGuideLines( guideLines );
            }
            else
            {
                QList<qreal> guideLines = guidesData->verticalGuideLines();
                guideLines[d->index] = position;
                guidesData->setVerticalGuideLines( guideLines );
            }
        }
    }
}

void GuidesTool::guideLineSelected( Qt::Orientation orientation, uint index )
{
    KoGuidesData * guidesData = m_canvas->guidesData();
    if( ! guidesData )
        return;

    repaintDecorations();

    d->orientation = orientation;
    d->index = index;

    if( d->orientation == Qt::Horizontal )
        d->position = guidesData->horizontalGuideLines()[index];
    else
        d->position = guidesData->verticalGuideLines()[index];

    repaintDecorations();
}

void GuidesTool::guideLinesChanged( Qt::Orientation orientation )
{
    KoGuidesData * guidesData = m_canvas->guidesData();
    if( ! guidesData )
        return;

    repaintDecorations();

    if( orientation == Qt::Horizontal )
        guidesData->setHorizontalGuideLines( d->options->horizontalGuideLines() );
    else
        guidesData->setVerticalGuideLines( d->options->verticalGuideLines() );

    if( orientation == d->orientation )
    {
        QList<qreal> lines;
        if( d->orientation == Qt::Horizontal )
            lines = guidesData->horizontalGuideLines();
        else
            lines = guidesData->verticalGuideLines();

        int oldIndex = d->index;

        if( lines.count() == 0 )
            d->index = -1;
        else if( d->index >= lines.count() )
            d->index = 0;

        if( d->index >= 0 )
            d->position = lines[d->index];

        if( oldIndex != d->index )
            d->options->selectGuideLine( d->orientation, d->index );
    }

    repaintDecorations();
}

GuidesTool::GuideLine GuidesTool::guideLineAtPosition( const QPointF &position )
{
    int index = -1;
    Qt::Orientation orientation = Qt::Horizontal;

    // check if we are on a guide line
    KoGuidesData * guidesData = m_canvas->guidesData();
    if( guidesData && guidesData->showGuideLines() ) {
        qreal handleRadius = m_canvas->resourceProvider()->handleRadius();
        qreal minDistance = m_canvas->viewConverter()->viewToDocumentX( handleRadius );
        uint i = 0;
        foreach( qreal guidePos, guidesData->horizontalGuideLines() ) {
            qreal distance = qAbs( guidePos - position.y() );
            if( distance < minDistance ) {
                orientation = Qt::Horizontal;
                index = i;
                minDistance = distance;
            }
            i++;
        }
        i = 0;
        foreach( qreal guidePos, guidesData->verticalGuideLines() )
        {
            qreal distance = qAbs( guidePos - position.x() );
            if( distance < minDistance ) {
                orientation = Qt::Vertical;
                index = i;
                minDistance = distance;
            }
            i++;
        }
    }

    return QPair<Qt::Orientation,int>( orientation, index );
}

void GuidesTool::resourceChanged( int key, const QVariant & res )
{
    if( key == KoCanvasResource::Unit )
    {
        if( d->options )
            d->options->setUnit( m_canvas->unit() );
    }
}

QMap< QString, QWidget* > GuidesTool::createOptionWidgets()
{
    QMap< QString, QWidget* > optionWidgets;
    if( d->mode != Private::EditGuide ) {
        d->options = new GuidesToolOptionWidget();

        connect( d->options, SIGNAL(guideLineSelected(Qt::Orientation,uint)),
                this, SLOT(guideLineSelected(Qt::Orientation,uint)) );

        connect( d->options, SIGNAL(guideLinesChanged(Qt::Orientation)),
                this, SLOT(guideLinesChanged(Qt::Orientation)) );

        optionWidgets.insert( "Guides Editor", d->options );

        d->insert = new InsertGuidesToolOptionWidget();

        connect( d->insert, SIGNAL(createGuides(GuidesTransaction*)),
                 this, SLOT(insertorCreateGuidesSlot(GuidesTransaction*)) );

        optionWidgets.insert( "Guides Insertor", d->insert );
    }
    return optionWidgets;
}

void GuidesTool::insertorCreateGuidesSlot( GuidesTransaction* result )
{
    QPoint documentStart = canvas()->documentOrigin();
    KoGuidesData * guidesData = m_canvas->guidesData();
    const QSizeF pageSize = m_canvas->resourceProvider()->resource(KoCanvasResource::PageSize).value<QSizeF>();

    QList< qreal > verticalLines;
    QList< qreal > horizontalLines;
    //save previous lines if requested
    if( !result->erasePreviousGuides ) {
        verticalLines.append( guidesData->verticalGuideLines() );
        horizontalLines.append( guidesData->horizontalGuideLines() );
    }

    //vertical guides
    if( result->insertVerticalEdgesGuides ) {
        verticalLines << 0 << pageSize.width();
    }

    int lastI = result->verticalGuides;
    qreal verticalJumps = pageSize.width() / (qreal)( result->verticalGuides + 1 );
    for( int i = 1 ; i <= lastI; ++i ) {
        verticalLines << verticalJumps * (qreal)i;
    }

    //horizontal guides
    lastI = result->horizontalGuides;
    if( result->insertHorizontalEdgesGuides ) {
        horizontalLines << 0 << pageSize.height();
    }

    qreal horizontalJumps = pageSize.height() / (qreal)( result->horizontalGuides + 1 );
    for( int i = 1 ; i <= lastI; ++i ) {
        horizontalLines << horizontalJumps * (qreal)i;
    }
    guidesData->setGuideLines( horizontalLines, verticalLines );

    delete result;
}

#include "GuidesTool.moc"
