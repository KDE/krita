/*
 *  kis_tool_select_contiguous - part of Krayon^WKrita
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <qpainter.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_cursor.h"
#include "kis_tool_select_contiguous.h"
#include "kis_view.h"

KisToolSelectContiguous::KisToolSelectContiguous() : super()
{
	setName("tool_select_contiguous");
	m_subject = 0;

	m_dragging = false;
	m_drawn = false;
	m_init  = true;
	m_dragStart = QPoint(-1,-1);
	m_dragEnd = QPoint(-1,-1);
	setCursor(KisCursor::brushCursor());
}

KisToolSelectContiguous::~KisToolSelectContiguous()
{
}

void KisToolSelectContiguous::clearOld()
{
// //   if (m_doc->isEmpty()) return;
        
// 	KisView *view = getCurrentView();

// 	if(m_dragStart.x() != -1)
// 		drawRect( m_dragStart, m_dragEnd ); 

// 	QRect updateRect(0, 0, m_doc->currentImg()->width(), 
// 			m_doc->currentImg()->height());
// 	view->updateCanvas(updateRect);

// 	m_dragStart = QPoint(-1,-1);
// 	m_dragEnd =   QPoint(-1,-1);
}

void KisToolSelectContiguous::buttonPress(KisButtonPressEvent *event)
{
//  //   if ( m_doc->isEmpty() )
// //        return;

//     if( event->button() == LeftButton )
//     {
//         // erase old rectangle    
//         if(m_drawn) 
//         {
//             m_drawn = false;
           
//             if(m_dragStart.x() != -1)
//                 drawRect( m_dragStart, m_dragEnd ); 
//         }
                
//         m_init = false;
//         m_dragging = true;
//         m_dragStart = event->pos();
//         m_dragEnd = event->pos();
//     }
}


void KisToolSelectContiguous::move(KisMoveEvent *event)
{
// //    if ( m_doc->isEmpty() )
// //        return;

//     if( m_dragging )
//     {
//         drawRect( m_dragStart, m_dragEnd );
//         m_dragEnd = event->pos();
//         drawRect( m_dragStart, m_dragEnd );
//     }
}


void KisToolSelectContiguous::buttonRelease(KisButtonReleaseEvent *event)
{
// //    if ( m_doc->isEmpty() )
// //        return;

//     if( ( m_dragging ) && ( event->button() == LeftButton ) )
//     {
//         m_dragging = false;
//         m_drawn = true;
        
//         QPoint zStart = zoomed(m_dragStart);
//         QPoint zEnd   = zoomed(m_dragEnd);
                
//         if(zStart.x() <= zEnd.x())
//         {
//             m_selectRect.setLeft(zStart.x());
//             m_selectRect.setRight(zEnd.x());
//         }    
//         else 
//         {
//             m_selectRect.setLeft(zEnd.x());                   
//             m_selectRect.setRight(zStart.x());
//         }
        
//         if(zStart.y() <= zEnd.y())
//         {
//             m_selectRect.setTop(zStart.y());
//             m_selectRect.setBottom(zEnd.y());            
//         }    
//         else
//         {
//             m_selectRect.setTop(zEnd.y());
//             m_selectRect.setBottom(zStart.y());            
//         }
                    
//         m_doc->getSelection()->setBounds(m_selectRect);

//         kdDebug(0) << "selectRect" 
//             << " left: "   << m_selectRect.left() 
//             << " top: "    << m_selectRect.top()
//             << " right: "  << m_selectRect.right() 
//             << " bottom: " << m_selectRect.bottom()
//             << endl;
//     }
}


void KisToolSelectContiguous::drawRect( const QPoint& start, const QPoint& end )
{
// 	KisView *view = getCurrentView();
// 	QPainter p, pCanvas;

// 	p.begin( m_canvas );
// 	p.setRasterOp( Qt::NotROP );
// 	p.setPen( QPen( Qt::DotLine ) );

// 	float zF = view->zoomFactor();

// 	p.drawRect( QRect(start.x() + view->xPaintOffset() 
// 				- (int)(zF * view->xScrollOffset()),
// 				start.y() + view->yPaintOffset() 
// 				- (int)(zF * view->yScrollOffset()), 
// 				end.x() - start.x(), 
// 				end.y() - start.y()) );
// 	p.end();
}

void KisToolSelectContiguous::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("Tool &Contiguous Select"), 
					    "contiguous" , 
					    0, 
					    this, 
					    SLOT(activate()), 
					    collection, 
					    name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}

bool KisToolSelectContiguous::willModify() const
{
// 	return false;
}


#include "kis_tool_select_contiguous.moc"
