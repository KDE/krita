/* This file is part of the KDE project
 *   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 * 
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 * 
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Library General Public License for more details.
 * 
 *   You should have received a copy of the GNU Library General Public License
 *   along with this library; see the file COPYING.LIB.  If not, write to
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoViewChild.h"

#include <QRect>

#include <KoView.h>
#include <KoFrame.h>
#include <KoDocumentChild.h>
#include <KoDocument.h>

class KoViewChild::KoViewChildPrivate
{
    public:
        KoViewChildPrivate()
        {
        }
        ~KoViewChildPrivate()
        {
        }
};

KoViewChild::KoViewChild( KoDocumentChild *child, KoView *_parentView )
{
    d = new KoViewChildPrivate;
    m_parentView = _parentView;
    m_child = child;

    m_frame = new KoFrame( parentView()->canvas() );
    KoView *view = child->document()->createView( m_frame );
    view->setXMLGUIBuildDocument( child->document()->viewBuildDocument( view ) );

    view->setPartManager( parentView()->partManager() );

    // hack? (Werner)
    view->setZoom( parentView()->zoom() * qMax(child->xScaling(), child->yScaling()) );

    m_frame->setView( view );
    m_frame->show();
    m_frame->raise();

    parentView()->canvasAddChild( this );


    /*
     *   KoViewChild has basically three geometries to keep in sync.
     *   - The KoDocumentChild geometry (i.e. the embedded object's geometry, unzoomed)
     *   - Its own geometry (used for hit-test etc.)
     *   - The KoFrame geometry (the graphical widget for moving the object when active)
     * 
     *   So we need to subtract the scrollview's offset for the frame geometry, since it's a widget.
     * 
     *   The rules are
     *   (R1) frameGeometry = viewGeometry(childGeometry) "+" m_frame->{left|right|top|bottom}Border() - scrollview offset,
     *   (R2) frameGeometry = myGeometry "+" active_frame_border - scrollview offset.
     * 
     *   So: (R3, unused) myGeometry = viewGeometry(childGeometry) "+" m_frame->{left|right|top|bottom}Border() "-" active_frame_border
     * 
     *   Notes: active_frame_border is m_frame->border() (0 when inactive, 5 when active).
     *          {left|right|top|bottom}Border are the borders used in kspread (0 when inactive, big when active).
     *          "+" border means we add a border, so it's a subtraction on x, y and an addition on width, height.
     * 
     *          viewGeometry() applies the zoom as well as any other translation the app might want to do
     */

    // Setting the frameGeometry is done in setInitialFrameGeometry, which is
    // also called right after activation.

    connect( view, SIGNAL( activated( bool ) ),
             parentView(), SLOT( slotChildActivated( bool ) ) );
}

KoViewChild::~KoViewChild()
{
    if ( m_frame )
    {
        slotFrameGeometryChanged();
        delete static_cast<KoFrame *>( m_frame );
    }
    delete d;
}

void KoViewChild::slotFrameGeometryChanged()
{
    // Set our geometry from the frame geometry (R2 reversed)
    QRect geom = m_frame->geometry();
    int b = m_frame->border();
    QRect borderRect( geom.x() + b + parentView()->canvasXOffset(),
                      geom.y() + b + parentView()->canvasYOffset(),
                      geom.width() - b * 2,
                      geom.height() - b * 2 );
                      setGeometry( borderRect );

    if(m_child)
    {
        // Set the child geometry from the frame geometry (R1 reversed)
        QRect borderLessRect( geom.x() + m_frame->leftBorder() + parentView()->canvasXOffset(),
                            geom.y() + m_frame->topBorder() + parentView()->canvasYOffset(),
                            geom.width() - m_frame->leftBorder() - m_frame->rightBorder(),
                            geom.height() - m_frame->topBorder() - m_frame->bottomBorder() );

        // We don't want to trigger slotDocGeometryChanged again
        lock();
        QRect childGeom = parentView()->reverseViewTransformations( borderLessRect );
        kDebug(30003) << "KoChild::slotFrameGeometryChanged child geometry "
        << ( geometry() == childGeom ? "already " : "set to " )
        << childGeom << endl;
        m_child->setGeometry( childGeom );
        unlock();
    }
}

void KoViewChild::slotDocGeometryChanged()
{
    if ( locked() )
        return;
    // Set frame geometry from child geometry (R1)
    // The frame's resizeEvent will call slotFrameGeometryChanged.
    QRect geom = parentView()->applyViewTransformations( m_child->geometry() );
    QRect borderRect( geom.x() - m_frame->leftBorder() - parentView()->canvasXOffset(),
                      geom.y() - m_frame->topBorder() - parentView()->canvasYOffset(),
                      geom.width() + m_frame->leftBorder() + m_frame->rightBorder(),
                      geom.height() + m_frame->topBorder() + m_frame->bottomBorder() );
    kDebug(30003) << "KoViewChild::slotDocGeometryChanged frame geometry "
    << ( m_frame->geometry() == borderRect ? "already " : "set to " )
    << borderRect << endl;

    m_frame->setGeometry( borderRect );
}

void KoViewChild::setInitialFrameGeometry()
{
    kDebug(30003) << k_funcinfo << endl;

    // Connect only now, so that the GUI building doesn't move us around.
    connect( m_frame, SIGNAL( geometryChanged() ),
             this, SLOT( slotFrameGeometryChanged() ) );
    connect( m_child, SIGNAL( changed( KoChild * ) ),
            this, SLOT( slotDocGeometryChanged() ) );

    // Set frameGeometry from childGeometry
    slotDocGeometryChanged();
    // Set myGeometry from frameGeometry
    slotFrameGeometryChanged();
}

#include "KoViewChild.moc"
