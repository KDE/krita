/*
 *  tool_select_elliptical.cc - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qpainter.h>
#include <qregion.h>

#include <kaction.h>
#include <kdebug.h>

#include "kis_doc.h"
#include "kis_canvas.h"
#include "kis_cursor.h"
#include "kis_tool_select_elliptical.h"
#include "kis_view.h"
#include "kis_vec.h"

EllipticalSelectTool::EllipticalSelectTool(KisDoc *doc, KisCanvas *canvas) : KisTool(doc)
{
	m_dragging = false;
	m_canvas = canvas;
	m_drawn = false;
	m_init  = true;
	m_dragStart = QPoint(-1,-1);
	m_dragEnd =   QPoint(-1,-1);
	m_cursor = KisCursor::selectCursor();
	moveSelectArea = false;
}

EllipticalSelectTool::~EllipticalSelectTool()
{
}

void EllipticalSelectTool::clearOld()
{
    if(m_dragStart.x() != -1)
        drawEllipse( m_dragStart, m_dragEnd ); 

    m_dragStart = QPoint(-1,-1);
    m_dragEnd =   QPoint(-1,-1);

    QRect updateRect(0, 0, m_doc->current()->width(), 
        m_doc->current()->height());
    m_view->updateCanvas(updateRect);
    m_selectRegion = QRegion();

}

void EllipticalSelectTool::mousePress( QMouseEvent* event )
{
    if( event->button() == LeftButton && !moveSelectArea)
    {
        clearOld();
        if(m_drawn) // erase old rectangle
        {
            m_drawn = false;
           
            if(m_dragStart.x() != -1)
                drawEllipse( m_dragStart, m_dragEnd ); 
        }
                
        m_init = false;
        m_dragging = true;
        m_dragStart = event->pos();
        m_dragEnd = event->pos();

        dragSelectArea = false;
    }
    else if( event->button() == LeftButton && moveSelectArea ) {
        dragSelectArea = true;
        dragFirst = true;
        m_dragStart = event->pos();
        m_dragdist = 0;

        m_hotSpot = event->pos();
        int x = zoomed( m_hotSpot.x() );
        int y = zoomed( m_hotSpot.y() );

        m_hotSpot = QPoint( x - m_imageRect.topLeft().x(), y - m_imageRect.topLeft().y() );

        oldDragPoint = event->pos();
        setClipImage();
    }
}


void EllipticalSelectTool::mouseMove( QMouseEvent* event )
{
    if( m_dragging && !dragSelectArea )
    {
        drawEllipse( m_dragStart, m_dragEnd );
        m_dragEnd = event->pos();
        drawEllipse( m_dragStart, m_dragEnd );
    }
    else if ( !m_dragging && !dragSelectArea ) {
        if ( !m_selectRegion.isNull() && m_selectRegion.contains( event->pos() ) ) {
            setMoveCursor();
            moveSelectArea = true;
        }
        else {
            setSelectCursor();
            moveSelectArea = false;
        }
    }
    else if ( dragSelectArea ) {
        if ( dragFirst ) {
            // remove select image
            m_doc->getSelection()->erase();

            // refresh canvas
            clearOld();
            m_view->slotUpdateImage();
            dragFirst = false;
        }

        int spacing = 10;
        float zF = m_view->zoomFactor();
        QPoint pos = event->pos();
        int mouseX = pos.x();
        int mouseY = pos.y();

        KisVector end( mouseX, mouseY );
        KisVector start( m_dragStart.x(), m_dragStart.y() );

        KisVector dragVec = end - start;
        float saved_dist = m_dragdist;
        float new_dist = dragVec.length();
        float dist = saved_dist + new_dist;

        if ( (int)dist < spacing ) {
            m_dragdist += new_dist;
            m_dragStart = pos;
            return;
        }
        else
            m_dragdist = 0;

        dragVec.normalize();
        KisVector step = start;

        while ( dist >= spacing ) {
            if ( saved_dist > 0 ) {
                step += dragVec * ( spacing - saved_dist );
                saved_dist -= spacing;
            }
            else
                step += dragVec * spacing;

            QPoint p( qRound( step.x() ), qRound( step.y() ) );

            QRect ur( zoomed( oldDragPoint.x() ) - m_hotSpot.x() - m_view->xScrollOffset(),
                      zoomed( oldDragPoint.y() ) - m_hotSpot.y() - m_view->yScrollOffset(),
                      (int)( m_clipPixmap.width() * ( zF > 1.0 ? zF : 1.0 ) ),
                      (int)( m_clipPixmap.height() * ( zF > 1.0 ? zF : 1.0 ) ) );

            m_view->updateCanvas( ur );

            dragSelectImage( p, m_hotSpot );

            oldDragPoint = p;
            dist -= spacing;
        }

        if ( dist > 0 ) 
            m_dragdist = dist;
        m_dragStart = pos;
    }
}


void EllipticalSelectTool::mouseRelease( QMouseEvent* event )
{
    if( ( m_dragging ) && ( event->button() == LeftButton ) && ( !moveSelectArea ) )
    {
        m_dragging = false;
        m_drawn = true;
        
        QPoint zStart = zoomed(m_dragStart);
        QPoint zEnd   = zoomed(m_dragEnd);
                
        /* jwc - leave selection rectange boundary on screen
        it is only drawn to canvas, not to retained imagePixmap,
        and therefore will disappear when another tool action is used */
        // drawRect( m_dragStart, m_dragEnd ); 
        
        /* get selection rectangle after mouse is released
        there always is one, even if width and height are 0 
        left and right, top and bottom are sometimes reversed! */
        
        if(zStart.x() <= zEnd.x())
        {
            m_selectRect.setLeft(zStart.x());
            m_selectRect.setRight(zEnd.x());
        }    
        else 
        {
            m_selectRect.setLeft(zEnd.x());                   
            m_selectRect.setRight(zStart.x());
        }
        
        if(zStart.y() <= zEnd.y())
        {
            m_selectRect.setTop(zStart.y());
            m_selectRect.setBottom(zEnd.y());            
        }    
        else
        {
            m_selectRect.setTop(zEnd.y());
            m_selectRect.setBottom(zStart.y());            
        }

        m_imageRect = m_selectRect;
        if ( m_selectRect.left() != m_selectRect.right() 
             && m_selectRect.top() != m_selectRect.bottom()  )
            m_selectRegion = QRegion( m_selectRect, QRegion::Ellipse );
        else
           m_selectRegion = QRegion();
                    
        m_doc->getSelection()->setEllipticalSelection( m_selectRect,
        m_doc->current()->getCurrentLayer());

        kdDebug(0) << "selectRect" 
            << " left: "   << m_selectRect.left() 
            << " top: "    << m_selectRect.top()
            << " right: "  << m_selectRect.right() 
            << " bottom: " << m_selectRect.bottom()
            << endl;
    }
    else {
        // Initialize
        dragSelectArea = false;
        m_selectRegion = QRegion();
        setSelectCursor();
        moveSelectArea = false;

        QPoint pos = event->pos();

        KisImage *img = m_doc->current();
        if ( !img )
            return;
        if( !img->getCurrentLayer()->visible() )
            return;
        if( pasteClipImage( zoomed( pos ) - m_hotSpot ) ) {
            img->markDirty( QRect( zoomed( pos ) - m_hotSpot, m_clipPixmap.size() ) );
	    m_doc -> setModified(true);
	}
    }
}


void EllipticalSelectTool::drawEllipse( const QPoint& start, const QPoint& end )
{
	QPainter p;
	QPen pen(Qt::DotLine);
	float zF = m_view -> zoomFactor();

	p.begin(m_canvas);
	p.setPen(pen);
	p.setRasterOp(Qt::NotROP);
    
	/* adjust for scroll ofset as this draws on the canvas, not on
	   the image itself QRect(left, top, width, height) */
    
	p.drawEllipse(QRect(start.x() + m_view -> xPaintOffset() - (int)(zF * m_view -> xScrollOffset()),
				start.y() + m_view -> yPaintOffset() - (int)(zF * m_view -> yScrollOffset()), 
				end.x() - start.x(), 
				end.y() - start.y()));
	p.end();
}

void EllipticalSelectTool::setupAction(QObject *collection)
{
	KToggleAction *toggle = new KToggleAction(i18n("&Elliptical select"), "elliptical" , 0, this, SLOT(toolSelect()),
			collection, "tool_select_elliptical" );

	toggle -> setExclusiveGroup("tools");
}

bool EllipticalSelectTool::willModify() const
{
	return false;
}

void EllipticalSelectTool::paintEvent(QPaintEvent *e)
{
	if (m_dragStart.x() != -1)
		drawEllipse(m_dragStart, m_dragEnd);
}

