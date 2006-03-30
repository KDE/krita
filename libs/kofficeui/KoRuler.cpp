/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

// Description: Ruler (header)

/******************************************************************/

#include "KoRuler.h"
#include <klocale.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <qcursor.h>
#include <qpainter.h>
#include <q3popupmenu.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3Frame>
#include <QResizeEvent>
#include <QMouseEvent>
#include <KoPageLayout.h>
#include <q3tl.h>

class KoRulerPrivate {
public:
    KoRulerPrivate() {
    }
    ~KoRulerPrivate() {}

    QWidget *canvas;
    int flags;
    int oldMx, oldMy;
    bool whileMovingBorderLeft, whileMovingBorderRight;
    bool whileMovingBorderTop, whileMovingBorderBottom;
    QPixmap pmFirst, pmLeft;
    KoPageLayout layout;
    KoTabChooser *tabChooser;
    KoTabulatorList tabList;
    // Do we have to remove a certain tab in the DC Event?
    KoTabulator removeTab;
    // The tab we're moving / clicking on - basically only valid between press and release time
    KoTabulator currTab;
    // The action we're currently doing - basically only valid between press and release time
    KoRuler::Action action;
    Q3PopupMenu *rb_menu;
    int mRemoveTab, mPageLayout; // menu item ids
    int frameEnd;
    double i_right;
    bool m_bReadWrite;
    bool doubleClickedIndent;
    bool rtl;
    bool mousePressed;
};

// Equality test for tab positions in particular
static inline bool equals( double a, double b )  {
    return qAbs( a - b ) < 1E-4;
}


/******************************************************************/
/* Class: KoRuler                                                 */
/******************************************************************/

const int KoRuler::F_TABS = 1;
const int KoRuler::F_INDENTS = 2;
const int KoRuler::F_HELPLINES = 4;
const int KoRuler::F_NORESIZE = 8;

/*================================================================*/
KoRuler::KoRuler( QWidget *_parent, QWidget *_canvas, Qt::Orientation _orientation,
                 const KoPageLayout& _layout, int _flags, KoUnit::Unit _unit, KoTabChooser *_tabChooser )
    : Q3Frame( _parent ), buffer( width(), height() ), m_zoom(1.0), m_1_zoom(1.0),
      m_unit( _unit )
{
    setWFlags( Qt::WResizeNoErase | Qt::WNoAutoErase );
    setFrameStyle( Q3Frame::StyledPanel );

    d=new KoRulerPrivate();

    d->tabChooser = _tabChooser;

    d->canvas = _canvas;
    orientation = _orientation;
    d->layout = _layout;
    d->flags = _flags;

    d->m_bReadWrite=true;
    d->doubleClickedIndent=false;
    diffx = 0;
    diffy = 0;
    i_left=0.0;
    i_first=0.0;
    d->i_right=0.0;

    setMouseTracking( true );
    d->mousePressed = false;
    d->action = A_NONE;

    d->oldMx = 0;
    d->oldMy = 0;
    d->rtl = false;

    showMPos = false;
    mposX = 0;
    mposY = 0;
    gridSize=0.0;
    hasToDelete = false;
    d->whileMovingBorderLeft = d->whileMovingBorderRight = d->whileMovingBorderTop = d->whileMovingBorderBottom = false;

    d->pmFirst = UserIcon( "koRulerFirst" );
    d->pmLeft = UserIcon( "koRulerLeft" );
    d->currTab.type = T_INVALID;

    d->removeTab.type = T_INVALID;
    if ( orientation == Qt::Horizontal ) {
        frameStart = qRound( zoomIt(d->layout.ptLeft) );
        d->frameEnd = qRound( zoomIt(d->layout.ptWidth - d->layout.ptRight) );
    } else {
        frameStart = qRound( zoomIt(d->layout.ptTop) );
        d->frameEnd = qRound( zoomIt(d->layout.ptHeight - d->layout.ptBottom) );
    }
    m_bFrameStartSet = false;

    setupMenu();

    // For compatibility, emitting doubleClicked shall emit openPageLayoutDia
    connect( this, SIGNAL( doubleClicked() ), this, SIGNAL( openPageLayoutDia() ) );
}

/*================================================================*/
KoRuler::~KoRuler()
{
    delete d->rb_menu;
    delete d;
}

void KoRuler::setPageLayoutMenuItemEnabled(bool b)
{
    d->rb_menu->setItemEnabled(d->mPageLayout, b);
}

/*================================================================*/
void KoRuler::setMousePos( int mx, int my )
{
    if ( !showMPos || ( mx == mposX && my == mposY ) ) return;

    QPainter p( this );
    p.setCompositionMode( QPainter::CompositionMode_DestinationOut );

    if ( orientation == Qt::Horizontal ) {
        if ( hasToDelete )
            p.drawLine( mposX, 1, mposX, height() - 1 );
        p.drawLine( mx, 1, mx, height() - 1 );
        hasToDelete = true;
    }
    else {
        if ( hasToDelete )
            p.drawLine( 1, mposY, width() - 1, mposY );
        p.drawLine( 1, my, width() - 1, my );
        hasToDelete = true;
    }
    p.end();

    mposX = mx;
    mposY = my;
}

// distance between the main lines (those with a number)
double KoRuler::lineDistance() const
{
    switch( m_unit ) {
    case KoUnit::U_INCH:
        return INCH_TO_POINT( m_zoom ); // every inch
    case KoUnit::U_PT:
        return 100.0 * m_zoom; // every 100 pt
    case KoUnit::U_MM:
    case KoUnit::U_CM:
    case KoUnit::U_DM:
        return CM_TO_POINT ( m_zoom ); // every cm
    case KoUnit::U_PI:
        return PI_TO_POINT ( 10.0 * m_zoom ); // every 10 pica
    case KoUnit::U_DD:
        return DD_TO_POINT( m_zoom ); // every diderot
    case KoUnit::U_CC:
        return CC_TO_POINT( 10.0 * m_zoom ); // every 10 cicero
    }
    // should never end up here
    return 100.0 * m_zoom;
}

/*================================================================*/
void KoRuler::drawHorizontal( QPainter *_painter )
{
    QFont font = KGlobalSettings::toolBarFont();
    QFontMetrics fm( font );
    resize( width(), qMax( fm.height() + 4, 20 ) );

    // Use a double-buffer pixmap
    QPainter p( &buffer );
    p.fillRect( 0, 0, width(), height(), QBrush( colorGroup().brush( QColorGroup::Background ) ) );

    int totalw = qRound( zoomIt(d->layout.ptWidth) );
    QString str;

    p.setBrush( colorGroup().brush( QColorGroup::Base ) );

    // Draw white rect
    QRect r;
    if ( !d->whileMovingBorderLeft )
        r.setLeft( -diffx + frameStart );
    else
        r.setLeft( d->oldMx );
    r.setTop( 0 );
    if ( !d->whileMovingBorderRight )
        r.setWidth(d->frameEnd-frameStart);
    else
        r.setRight( d->oldMx );
    r.setBottom( height() );

    p.drawRect( r );
    p.setFont( font );

    // Draw the numbers
    double dist = lineDistance();
    int maxwidth = 0;

    for ( double i = 0.0;i <= (double)totalw;i += dist ) {
        str = QString::number( KoUnit::toUserValue( i / m_zoom, m_unit ) );
        int textwidth = fm.width( str );
        maxwidth = qMax( maxwidth, textwidth );
    }

    // Make sure that the ruler stays readable at lower zoom levels
    while( dist <= maxwidth ) {
        dist += lineDistance();
    }

    for ( double i = 0.0;i <= (double)totalw;i += dist ) {
        str = QString::number( KoUnit::toUserValue( i / m_zoom, m_unit ) );
        int textwidth = fm.width( str );
        maxwidth = qMax( maxwidth, textwidth );
        p.drawText( qRound(i) - diffx - qRound(textwidth * 0.5),
                    qRound(( height() - fm.height() ) * 0.5),
                    textwidth, height(), Qt::AlignLeft | Qt::AlignTop, str );
    }

    // Draw the medium-sized lines
    // Only if we have enough space (i.e. not at 33%)
    if ( dist > maxwidth + 2 )
    {
        for ( double i = dist * 0.5;i <= (double)totalw;i += dist ) {
            int ii=qRound(i);
            p.drawLine( ii - diffx, 7, ii - diffx, height() - 7 );
        }
    }

    // Draw the small lines
    // Only if we have enough space (i.e. not at 33%)
    if ( dist * 0.5 > maxwidth + 2 )
    {
        for ( double i = dist * 0.25;i <= (double)totalw;i += dist * 0.5 ) {
            int ii=qRound(i);
            p.drawLine( ii - diffx, 9, ii - diffx, height() - 9 );
        }
    }

    // Draw ending bar (at page width)
    //int constant=zoomIt(1);
    //p.drawLine( totalw - diffx + constant, 1, totalw - diffx + constant, height() - 1 );
    //p.setPen( colorGroup().color( QColorGroup::Base ) );
    //p.drawLine( totalw - diffx, 1, totalw - diffx, height() - 1 );

    // Draw starting bar (at 0)
    //p.setPen( colorGroup().color( QColorGroup::Text ) );
    //p.drawLine( -diffx, 1, -diffx, height() - 1 );
    //p.setPen( colorGroup().color( QColorGroup::Base ) );
    //p.drawLine( -diffx - constant, 1, -diffx - constant, height() - 1 );

    // Draw the indents triangles
    if ( d->flags & F_INDENTS ) {
        int top = 1;
        double halfPixmapWidth = d->pmFirst.width() * 0.5;
        // Cumulate i_first with correct indent
        double firstLineIdent = i_first + ( d->rtl ? d->i_right : i_left );
        p.drawPixmap( qRound( static_cast<double>(r.left()) + applyRtlAndZoom( firstLineIdent ) - halfPixmapWidth ),
                      top, d->pmFirst );

        int bottom = height() - d->pmLeft.height() - 1;
        halfPixmapWidth = d->pmLeft.width() * 0.5;
        p.drawPixmap( qRound( static_cast<double>(r.left()) + zoomIt(i_left) - halfPixmapWidth ),
                      bottom, d->pmLeft );
        p.drawPixmap( qRound( static_cast<double>(r.right()) - zoomIt(d->i_right) - halfPixmapWidth ),
                      bottom, d->pmLeft );
    }

    // Show the mouse position
    if ( d->action == A_NONE && showMPos ) {
        p.setPen( colorGroup().color( QColorGroup::Text ) );
        p.drawLine( mposX, 1, mposX, height() - 1 );
    }
    hasToDelete = false;

    // Draw the tabs
    if ( d->tabChooser && ( d->flags & F_TABS ) && !d->tabList.isEmpty() )
        drawTabs( p );

    p.end();
    _painter->drawPixmap( 0, 0, buffer );
}

/*================================================================*/
void KoRuler::drawTabs( QPainter &_painter )
{
    int ptPos = 0;

    _painter.setPen( QPen( colorGroup().color( QColorGroup::Text ), 2, Qt::SolidLine ) );
    // Check if we're in a mousemove event, removing a tab.
    // In that case, we'll have to skip drawing that one.
    bool willRemove = d->mousePressed && willRemoveTab( d->oldMy ) && d->currTab.type != T_INVALID;

    KoTabulatorList::ConstIterator it = d->tabList.begin();
    for ( ; it != d->tabList.end() ; it++ ) {
        if ( willRemove && equals( d->currTab.ptPos, (*it).ptPos ) )
            continue;
        ptPos = qRound(applyRtlAndZoom((*it).ptPos)) - diffx + frameStart;
        switch ( (*it).type ) {
        case T_LEFT: {
            ptPos -= 4;
            _painter.drawLine( ptPos + 4, height() - 4, ptPos + 20 - 4, height() - 4 );
            _painter.drawLine( ptPos + 5, 4, ptPos + 5, height() - 4 );
        } break;
        case T_CENTER: {
            ptPos -= 10;
            _painter.drawLine( ptPos + 4, height() - 4, ptPos + 20 - 4, height() - 4 );
            _painter.drawLine( ptPos + 20 / 2, 4, ptPos + 20 / 2, height() - 4 );
        } break;
        case T_RIGHT: {
            ptPos -= 16;
            _painter.drawLine( ptPos + 4, height() - 4, ptPos + 20 - 4, height() - 4 );
            _painter.drawLine( ptPos + 20 - 5, 4, ptPos + 20 - 5, height() - 4 );
        } break;
        case T_DEC_PNT: {
            ptPos -= 10;
            _painter.drawLine( ptPos + 4, height() - 4, ptPos + 20 - 4, height() - 4 );
            _painter.drawLine( ptPos + 20 / 2, 4, ptPos + 20 / 2, height() - 4 );
            _painter.fillRect( ptPos + 20 / 2 + 2, height() - 9, 3, 3,
                               colorGroup().color( QColorGroup::Text ) );
        } break;
        default: break;
        }
    }
}

/*================================================================*/
void KoRuler::drawVertical( QPainter *_painter )
{
    QFont font = KGlobalSettings::toolBarFont();
    QFontMetrics fm( font );
    resize( qMax( fm.height() + 4, 20 ), height() );

    QPainter p( &buffer );
    p.fillRect( 0, 0, width(), height(), QBrush( colorGroup().brush( QColorGroup::Background ) ) );

    int totalh = qRound( zoomIt(d->layout.ptHeight) );
    // Clip rect - this gives basically always a rect like (2,2,width-2,height-2)
    QRect paintRect = _painter->clipRegion().boundingRect();
    // Ruler rect
    QRect rulerRect( 0, -diffy, width(), totalh );

    if ( paintRect.intersects( rulerRect ) )  {
        QString str;

        p.setBrush( colorGroup().brush( QColorGroup::Base ) );

        // Draw white rect
        QRect r;
        if ( !d->whileMovingBorderTop )
            r.setTop( -diffy + frameStart );
        else
            r.setTop( d->oldMy );
        r.setLeft( 0 );
        if ( !d->whileMovingBorderBottom )
            r.setHeight(d->frameEnd-frameStart);
        else
            r.setBottom( d->oldMy );
        r.setRight( width() );

        p.drawRect( r );
        p.setFont( font );

        // Draw the numbers
        double dist = lineDistance();
        int maxheight = 0;

        for ( double i = 0.0;i <= (double)totalh;i += dist ) {
            str = QString::number( KoUnit::toUserValue( i / m_zoom, m_unit ) );
            int textwidth = fm.width( str );
            maxheight = qMax( maxheight, textwidth );
        }

        // Make sure that the ruler stays readable at lower zoom levels
        while( dist <= maxheight ) {
            dist += lineDistance();
        }

        for ( double i = 0.0;i <= (double)totalh;i += dist ) {
            str = QString::number( KoUnit::toUserValue( i / m_zoom, m_unit ) );
            int textheight = fm.height();
            int textwidth = fm.width( str );
            maxheight = qMax( maxheight, textwidth );
            p.save();
            p.translate( qRound(( width() - textheight ) * 0.5),
                         qRound(i) - diffy + qRound(textwidth * 0.5) );
            p.rotate( -90 );
            p.drawText( 0, 0, textwidth + 1, textheight, Qt::AlignLeft | Qt::AlignTop, str );
            p.restore();
        }

        // Draw the medium-sized lines
        if ( dist > maxheight + 2 )
        {
            for ( double i = dist * 0.5;i <= (double)totalh;i += dist ) {
                int ii=qRound(i);
                p.drawLine( 7, ii - diffy, width() - 7, ii - diffy );
            }
        }

        // Draw the small lines
        if ( dist * 0.5 > maxheight + 2 )
        {
            for ( double i = dist * 0.25;i <=(double)totalh;i += dist *0.5 ) {
                int ii=qRound(i);
                p.drawLine( 9, ii - diffy, width() - 9, ii - diffy );
            }
        }

        // Draw ending bar (at page height)
        //p.drawLine( 1, totalh - diffy + 1, width() - 1, totalh - diffy + 1 );
        //p.setPen( colorGroup().color( QColorGroup::Base ) );
        //p.drawLine( 1, totalh - diffy, width() - 1, totalh - diffy );

        // Draw starting bar (at 0)
        //p.setPen( colorGroup().color( QColorGroup::Text ) );
        //p.drawLine( 1, -diffy, width() - 1, -diffy );
        //p.setPen( colorGroup().color( QColorGroup::Base ) );
        //p.drawLine( 1, -diffy - 1, width() - 1, -diffy - 1 );
    }

    // Show the mouse position
    if ( d->action == A_NONE && showMPos ) {
        p.setPen( colorGroup().color( QColorGroup::Text ) );
        p.drawLine( 1, mposY, width() - 1, mposY );
    }
    hasToDelete = false;

    p.end();
    _painter->drawPixmap( 0, 0, buffer );
}

void KoRuler::mousePressEvent( QMouseEvent *e )
{
    if( !d->m_bReadWrite)
        return;

    d->oldMx = e->x();
    d->oldMy = e->y();
    d->mousePressed = true;
    d->removeTab.type = T_INVALID;

    switch ( e->button() ) {
    case Qt::RightButton:
        if(d->currTab.type == T_INVALID || !(d->flags & F_TABS))
            d->rb_menu->setItemEnabled(d->mRemoveTab, false);
        else
            d->rb_menu->setItemEnabled(d->mRemoveTab, true);
        d->rb_menu->popup( QCursor::pos() );
        d->action = A_NONE;
        d->mousePressed = false;
        return;
    case Qt::MidButton:
        // MMB shall do like double-click (it opens a dialog).
        handleDoubleClick();
        return;
    case Qt::LeftButton:
        if ( d->action == A_BR_RIGHT || d->action == A_BR_LEFT ) {
            if ( d->action == A_BR_RIGHT )
                d->whileMovingBorderRight = true;
            else
                d->whileMovingBorderLeft = true;

            if ( d->canvas )
                drawLine(d->oldMx, -1);
            update();
        } else if ( d->action == A_BR_TOP || d->action == A_BR_BOTTOM ) {
            if ( d->action == A_BR_TOP )
                d->whileMovingBorderTop = true;
            else
                d->whileMovingBorderBottom = true;

            if ( d->canvas ) {
                QPainter p( d->canvas );
                p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
                p.drawLine( 0, d->oldMy, d->canvas->width(), d->oldMy );
                p.end();
            }
            update();
        } else if ( d->action == A_FIRST_INDENT || d->action == A_LEFT_INDENT || d->action == A_RIGHT_INDENT ) {
            if ( d->canvas )
                drawLine(d->oldMx, -1);
        } else if ( d->action == A_TAB ) {
            if ( d->canvas && d->currTab.type != T_INVALID ) {
                drawLine( qRound( applyRtlAndZoom(d->currTab.ptPos) ) + frameStart - diffx, -1 );
            }
        } else if ( d->tabChooser && ( d->flags & F_TABS ) && d->tabChooser->getCurrTabType() != 0 ) {
            int left = frameStart - diffx;
            int right = d->frameEnd - diffx;

            if( e->x()-left < 0 || right-e->x() < 0 )
                return;
            KoTabulator tab;
            tab.filling = TF_BLANK;
            tab.ptWidth = 0.5;
            switch ( d->tabChooser->getCurrTabType() ) {
            case KoTabChooser::TAB_LEFT:
                tab.type = T_LEFT;
                break;
            case KoTabChooser::TAB_CENTER:
                tab.type = T_CENTER;
                break;
            case KoTabChooser::TAB_RIGHT:
                tab.type = T_RIGHT;
                break;
            case KoTabChooser::TAB_DEC_PNT:
                tab.type = T_DEC_PNT;
                tab.alignChar = KGlobal::locale()->decimalSymbol()[0];
                break;
            default: break;
            }
            tab.ptPos = unZoomItRtl( e->x() + diffx - frameStart );

            KoTabulatorList::Iterator it=d->tabList.begin();
            while ( it!=d->tabList.end() && tab > (*it) )
		++it;

            d->tabList.insert(it, tab);

            d->action = A_TAB;
            d->removeTab = tab;
            d->currTab = tab;

            emit tabListChanged( d->tabList );
            update();
        }
        else if ( d->flags & F_HELPLINES )
        {
	    setCursor( orientation == Qt::Horizontal ?
		       Qt::SizeHorCursor : Qt::SizeHorCursor );
            d->action = A_HELPLINES;
        }
    default:
        break;
    }
}

void KoRuler::mouseReleaseEvent( QMouseEvent *e )
{
    d->mousePressed = false;

    // Hacky, but necessary to prevent multiple tabs with the same coordinates (Werner)
    bool fakeMovement=false;
    if(d->removeTab.type != T_INVALID) {
        mouseMoveEvent(e);
        fakeMovement=true;
    }

    if ( d->action == A_BR_RIGHT || d->action == A_BR_LEFT ) {
        d->whileMovingBorderRight = false;
        d->whileMovingBorderLeft = false;

        if ( d->canvas )
            drawLine(d->oldMx, -1);
        update();
        emit newPageLayout( d->layout );
    } else if ( d->action == A_BR_TOP || d->action == A_BR_BOTTOM ) {
        d->whileMovingBorderTop = false;
        d->whileMovingBorderBottom = false;

        if ( d->canvas ) {
            QPainter p( d->canvas );
            p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
            p.drawLine( 0, d->oldMy, d->canvas->width(), d->oldMy );
            p.end();
        }
        update();
        emit newPageLayout( d->layout );
    } else if ( d->action == A_FIRST_INDENT ) {
        if ( d->canvas )
            drawLine(d->oldMx, -1);
        update();
        emit newFirstIndent( i_first );
    } else if ( d->action == A_LEFT_INDENT ) {
        if ( d->canvas )
            drawLine(d->oldMx, -1);
        update();
        emit newLeftIndent( i_left );
    } else if ( d->action == A_RIGHT_INDENT ) {
        if ( d->canvas )
            drawLine(d->oldMx, -1);
        update();
        emit newRightIndent( d->i_right );
    } else if ( d->action == A_TAB ) {
        if ( d->canvas && !fakeMovement ) {
            drawLine( qRound( applyRtlAndZoom( d->currTab.ptPos ) ) + frameStart - diffx, -1);
        }
        if ( willRemoveTab( e->y() ) )
        {
            d->tabList.remove(d->currTab);
        }
        qHeapSort( d->tabList );

        // Delete the new tabulator if it is placed on top of another.
        KoTabulatorList::ConstIterator tmpTab=d->tabList.begin();
        int count=0;
        while(tmpTab!=d->tabList.end()) {
            if( equals( (*tmpTab).ptPos, d->currTab.ptPos ) ) {
                count++;
                if(count > 1) {
                    d->tabList.remove(d->currTab);
                    break;
                }
            }
            tmpTab++;
        }
        //searchTab( e->x() ); // DF: why set currTab here?
        emit tabListChanged( d->tabList );
        update();
    }
    else if( d->action == A_HELPLINES )
    {
        emit addGuide( e->pos(), orientation == Qt::Horizontal, orientation == Qt::Horizontal ? size().height() : size().width() );
        emit addHelpline( e->pos(), orientation == Qt::Horizontal);
        setCursor( Qt::ArrowCursor );
    }
    d->currTab.type = T_INVALID; // added (DF)
}

void KoRuler::mouseMoveEvent( QMouseEvent *e )
{
    hasToDelete = false;

    int pw = d->frameEnd - frameStart;
    int ph = qRound(zoomIt(d->layout.ptHeight));
    int left = frameStart - diffx;
    int top = qRound(zoomIt(d->layout.ptTop));
    top -= diffy;
    int right = d->frameEnd - diffx;
    int bottom = qRound(zoomIt(d->layout.ptBottom));
    bottom = ph - bottom - diffy;
    // Cumulate first-line-indent
    int ip_first = qRound( zoomIt( i_first + ( d->rtl ? d->i_right : i_left) ) );
    int ip_left = qRound(zoomIt(i_left));
    int ip_right = qRound(zoomIt(d->i_right));

    int mx = e->x();
    mx = mx+diffx < 0 ? 0 : mx;
    int my = e->y();
    my = my+diffy < 0 ? 0 : my;

    QToolTip::remove( this);
    switch ( orientation ) {
        case Qt::Horizontal: {
            if ( !d->mousePressed ) {
                setCursor( Qt::ArrowCursor );
                d->action = A_NONE;
                /////// ######
                // At the moment, moving the left and right border indicators
                // is disabled when setFrameStartEnd has been called (i.e. in KWord)
                // Changing the layout margins directly from it would be utterly wrong
                // (just try the 2-columns modes...). What needs to be done is:
                // emitting a signal frameResized in mouseReleaseEvent, when a left/right
                // border has been moved, and in kword we need to update the margins from
                // there, if the left border of the 1st column or the right border of the
                // last column was moved... and find what to do with the other borders.
                // And for normal frames, resize the frame without touching the page layout.
                // All that is too much for now -> disabling.
                if ( !m_bFrameStartSet )
                {
                    if ( mx > left - 5 && mx < left + 5 ) {
                        setCursor( Qt::SizeHorCursor );
                        d->action = A_BR_LEFT;
                    } else if ( mx > right - 5 && mx < right + 5 ) {
                        setCursor( Qt::SizeHorCursor );
                        d->action = A_BR_RIGHT;
                    }
                }
                if ( d->flags & F_INDENTS ) {
                    int firstX = d->rtl ? right - ip_first : left + ip_first;
                    if ( mx > firstX - 5 && mx < firstX + 5 &&
                         my >= 2 && my <= d->pmFirst.size().height() + 2 ) {
                        this->setToolTip( i18n("First line indent") );
                        setCursor( Qt::ArrowCursor );
                        d->action = A_FIRST_INDENT;
                    } else if ( mx > left + ip_left - 5 && mx < left + ip_left + 5 &&
                                my >= height() - d->pmLeft.size().height() - 2 && my <= height() - 2 ) {
                        this->setToolTip( i18n("Left indent") );
                        setCursor( Qt::ArrowCursor );
                        d->action = A_LEFT_INDENT;
                    } else if ( mx > right - ip_right - 5 && mx < right - ip_right + 5 &&
                                my >= height() - d->pmLeft.size().height() - 2 && my <= height() - 2 ) {
                        this->setToolTip( i18n("Right indent") );
                        setCursor( Qt::ArrowCursor );
                        d->action = A_RIGHT_INDENT;
                    }
                }
                if ( d->flags & F_TABS )
                    searchTab(mx);
            } else {
                // Calculate the new value.
                int newPos=mx;
                if( newPos!=right && gridSize!=0.0 && (e->state() & Qt::ShiftModifier)==0) { // apply grid.
                    double grid=zoomIt(gridSize * 16);
                    newPos=qRound( ((newPos * 16 / grid) * grid) / 16 );
                }
                if(newPos-left < 0) newPos=left;
                else if (right-newPos < 0) newPos=right;
                double newValue = unZoomIt(static_cast<double>(newPos) - frameStart + diffx);

                switch ( d->action ) {
                    case A_BR_LEFT: {
                        if ( d->canvas && mx < right-10 && mx+diffx-2 > 0) {
                            drawLine( d->oldMx, mx );
                            d->layout.ptLeft = unZoomIt(static_cast<double>(mx + diffx));
                            if( ip_left > right-left-15 ) {
                                ip_left=right-left-15;
                                ip_left=ip_left<0 ? 0 : ip_left;
                                i_left=unZoomIt( ip_left );
                                emit newLeftIndent( i_left );
                            }
                            if ( ip_right > right-left-15 ) {
                                ip_right=right-left-15;
                                ip_right=ip_right<0? 0 : ip_right;
                                d->i_right=unZoomIt( ip_right );
                                emit newRightIndent( d->i_right );
                            }
                            d->oldMx = mx;
                            d->oldMy = my;
                            update();
                        }
                        else
                            return;
                    } break;
                    case A_BR_RIGHT: {
                        if ( d->canvas && mx > left+10 && mx+diffx <= pw-2) {
                            drawLine( d->oldMx, mx );
                            d->layout.ptRight = unZoomIt(static_cast<double>(pw - ( mx + diffx )));
                            if( ip_left > right-left-15 ) {
                                ip_left=right-left-15;
                                ip_left=ip_left<0 ? 0 : ip_left;
                                i_left=unZoomIt( ip_left );
                                emit newLeftIndent( i_left );
                            }
                            if ( ip_right > right-left-15 ) {
                                ip_right=right-left-15;
                                ip_right=ip_right<0? 0 : ip_right;
                                d->i_right=unZoomIt( ip_right );
                                emit newRightIndent( d->i_right );
                            }
                            d->oldMx = mx;
                            d->oldMy = my;
                            update();
                        }
                        else
                            return;
                    } break;
                    case A_FIRST_INDENT: {
                        if ( d->canvas ) {
                            if (d->rtl)
                                newValue = unZoomIt(pw) - newValue - d->i_right;
                            else
                                newValue -= i_left;
                            if(newValue == i_first) break;
                            drawLine( d->oldMx, newPos);
                            d->oldMx=newPos;
                            i_first = newValue;
                            update();
                        }
                    } break;
                    case A_LEFT_INDENT: {
                        if ( d->canvas ) {
                            //if (d->rtl) newValue = unZoomIt(pw) - newValue;
                            if(newValue == i_left) break;

                            drawLine( d->oldMx, newPos);
                            i_left = newValue;
                            d->oldMx = newPos;
                            update();
                        }
                    } break;
                    case A_RIGHT_INDENT: {
                        if ( d->canvas ) {
                            double rightValue = unZoomIt(right - newPos);
                            //if (d->rtl) rightValue = unZoomIt(pw) - rightValue;
                            if(rightValue == d->i_right) break;

                            drawLine( d->oldMx, newPos);
                            d->i_right=rightValue;
                            d->oldMx = newPos;
                            update();
                        }
                    } break;
                    case A_TAB: {
                        if ( d->canvas) {
                            if (d->rtl) newValue = unZoomIt(pw) - newValue;
                            if(newValue == d->currTab.ptPos) break; // no change
                            QPainter p( d->canvas );
                            p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
                            // prevent 1st drawLine when we just created a new tab
                            // (it's a NOT line)
                            double pt;
                            int pt_fr;
                            if( d->currTab != d->removeTab )
                            {
                                pt = applyRtlAndZoom(d->currTab.ptPos);
                                pt_fr = qRound(pt) + frameStart - diffx;
                                p.drawLine( pt_fr, 0, pt_fr, d->canvas->height() );
                            }

                            KoTabulatorList::Iterator it = d->tabList.find( d->currTab );
                            Q_ASSERT( it != d->tabList.end() );
                            if ( it != d->tabList.end() )
                                (*it).ptPos = newValue;
                            d->currTab.ptPos = newValue;

                            pt = applyRtlAndZoom( newValue );
                            pt_fr = qRound(pt) + frameStart - diffx;
                            p.drawLine( pt_fr, 0, pt_fr, d->canvas->height() );

                            p.end();
                            d->oldMx = mx;
                            d->oldMy = my;
                            d->removeTab.type = T_INVALID;
                            update();
                        }
                    } break;
                    default: break;
                }
            }
            if( d->action == A_HELPLINES )
            {
                emit moveGuide( e->pos(), true, size().height() );
                emit moveHelpLines( e->pos(), true );
            }

            return;
        } break;
        case Qt::Vertical: {
            if ( !d->mousePressed ) {
                setCursor( Qt::ArrowCursor );
                d->action = A_NONE;
                if ( d->flags & F_NORESIZE )
                    break;
                if ( my > top - 5 && my < top + 5 ) {
                    this->setToolTip( i18n("Top margin") );
                    setCursor( Qt::SizeHorCursor );
                    d->action = A_BR_TOP;
                } else if ( my > bottom - 5 && my < bottom + 5 ) {
                    this->setToolTip( i18n("Bottom margin") );
                    setCursor( Qt::SizeHorCursor );
                    d->action = A_BR_BOTTOM;
                }
            } else {
                switch ( d->action ) {
                    case A_BR_TOP: {
                        if ( d->canvas && my < bottom-20 && my+diffy-2 > 0) {
                            QPainter p( d->canvas );
                            p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
                            p.drawLine( 0, d->oldMy, d->canvas->width(), d->oldMy );
                            p.drawLine( 0, my, d->canvas->width(), my );
                            p.end();
                            d->layout.ptTop = unZoomIt(static_cast<double>(my + diffy));
                            d->oldMx = mx;
                            d->oldMy = my;
                            update();
                        }
                        else
                            return;
                    } break;
                    case A_BR_BOTTOM: {
                        if ( d->canvas && my > top+20 && my+diffy < ph-2) {
                            QPainter p( d->canvas );
                            p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
                            p.drawLine( 0, d->oldMy, d->canvas->width(), d->oldMy );
                            p.drawLine( 0, my, d->canvas->width(), my );
                            p.end();
                            d->layout.ptBottom = unZoomIt(static_cast<double>(ph - ( my + diffy )));
                            d->oldMx = mx;
                            d->oldMy = my;
                            update();
                        }
                        else
                            return;
                    } break;
                    default: break;
                }
            }

            if( d->action == A_HELPLINES )
            {
                emit moveGuide( e->pos(), false, size().width() );
                emit moveHelpLines( e->pos(), false );
            }
        } break;
    }

    d->oldMx = mx;
    d->oldMy = my;
}

void KoRuler::resizeEvent( QResizeEvent *e )
{
    Q3Frame::resizeEvent( e );
    buffer.resize( size() );
}

void KoRuler::mouseDoubleClickEvent( QMouseEvent* )
{
    handleDoubleClick();
}

void KoRuler::handleDoubleClick()
{
    if ( !d->m_bReadWrite )
        return;

    d->doubleClickedIndent = false;
    if ( d->tabChooser && ( d->flags & F_TABS ) ) {
        // Double-click and mousePressed inserted a tab -> need to remove it
        if ( d->tabChooser->getCurrTabType() != 0 && d->removeTab.type != T_INVALID && !d->tabList.isEmpty()) {
            uint c = d->tabList.count();
            d->tabList.remove( d->removeTab );
            Q_ASSERT( d->tabList.count() < c );

            d->removeTab.type = T_INVALID;
            d->currTab.type = T_INVALID;
            emit tabListChanged( d->tabList );
            setCursor( Qt::ArrowCursor );
            update();
            // --- we didn't click on a tab, fall out to indents test ---
        } else if ( d->action == A_TAB ) {
            // Double-click on a tab
            emit doubleClicked( d->currTab.ptPos ); // usually paragraph dialog
            return;
        }
    }

    // When Binary Compatibility is broken this will hopefully emit a
    // doubleClicked(int) to differentiate between double-clicking an
    // indent and double-clicking the ruler
    if ( d->flags & F_INDENTS ) {
        if ( d->action == A_LEFT_INDENT || d->action == A_RIGHT_INDENT || d->action == A_FIRST_INDENT ) {
            d->doubleClickedIndent = true;
            emit doubleClicked(); // usually paragraph dialog
            return;
        }
    }

    // Double-clicked nothing
    d->action = A_NONE;
    emit doubleClicked(); // usually page layout dialog
}

void KoRuler::setTabList( const KoTabulatorList & _tabList )
{
    d->tabList = _tabList;
    qHeapSort(d->tabList);   // "Trust no one." as opposed to "In David we trust."

    // Note that d->currTab and d->removeTab could now point to
    // tabs which don't exist in d->tabList

    update();
}

double KoRuler::makeIntern( double _v )
{
    return KoUnit::fromUserValue( _v, m_unit );
}

void KoRuler::setupMenu()
{
    d->rb_menu = new Q3PopupMenu();
    Q_CHECK_PTR( d->rb_menu );
    for ( uint i = 0 ; i <= KoUnit::U_LASTUNIT ; ++i )
    {
        KoUnit::Unit unit = static_cast<KoUnit::Unit>( i );
        d->rb_menu->insertItem( KoUnit::unitDescription( unit ), i /*as id*/ );
        if ( m_unit == unit )
            d->rb_menu->setItemChecked( i, true );
    }
    connect( d->rb_menu, SIGNAL( activated( int ) ), SLOT( slotMenuActivated( int ) ) );

    d->rb_menu->insertSeparator();
    d->mPageLayout=d->rb_menu->insertItem(i18n("Page Layout..."), this, SLOT(pageLayoutDia()));
    d->rb_menu->insertSeparator();
    d->mRemoveTab=d->rb_menu->insertItem(i18n("Remove Tabulator"), this, SLOT(rbRemoveTab()));
    d->rb_menu->setItemEnabled( d->mRemoveTab, false );
}

void KoRuler::uncheckMenu()
{
    for ( uint i = 0 ; i <= KoUnit::U_LASTUNIT ; ++i )
        d->rb_menu->setItemChecked( i, false );
}

void KoRuler::setUnit( const QString& _unit )
{
    setUnit( KoUnit::unit( _unit ) );
}

void KoRuler::setUnit( KoUnit::Unit unit )
{
    m_unit = unit;
    uncheckMenu();
    d->rb_menu->setItemChecked( m_unit, true );
    update();
}

void KoRuler::setZoom( const double& zoom )
{
    if(zoom==m_zoom)
        return;
    if(zoom < 1E-4) // Don't do 0 or negative values
        return;
    m_zoom=zoom;
    m_1_zoom=1/m_zoom;
    update();
}

bool KoRuler::willRemoveTab( int y ) const
{
    return (y < -50 || y > height() + 25) && d->currTab.type != T_INVALID;
}

void KoRuler::rbRemoveTab() {

    d->tabList.remove( d->currTab );
    d->currTab.type = T_INVALID;
    emit tabListChanged( d->tabList );
    update();
}

void KoRuler::setReadWrite(bool _readWrite)
{
    d->m_bReadWrite=_readWrite;
}

void KoRuler::searchTab(int mx) {

    int pos;
    d->currTab.type = T_INVALID;
    KoTabulatorList::ConstIterator it = d->tabList.begin();
    for ( ; it != d->tabList.end() ; ++it ) {
        pos = qRound(applyRtlAndZoom((*it).ptPos)) - diffx + frameStart;
        if ( mx > pos - 5 && mx < pos + 5 ) {
            setCursor( Qt::SizeHorCursor );
            d->action = A_TAB;
            d->currTab = *it;
            break;
        }
    }
}

void KoRuler::drawLine(int oldX, int newX) {

    QPainter p( d->canvas );
    p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
    p.drawLine( oldX, 0, oldX, d->canvas->height() );
    if(newX!=-1)
        p.drawLine( newX, 0, newX, d->canvas->height() );
    p.end();
}

void KoRuler::showMousePos( bool _showMPos )
{
    showMPos = _showMPos;
    hasToDelete = false;
    mposX = -1;
    mposY = -1;
    update();
}

void KoRuler::setOffset( int _diffx, int _diffy )
{
    //kDebug() << "KoRuler::setOffset " << _diffx << "," << _diffy << endl;
    diffx = _diffx;
    diffy = _diffy;
    update();
}

void KoRuler::setFrameStartEnd( int _frameStart, int _frameEnd )
{
    if ( _frameStart != frameStart || _frameEnd != d->frameEnd || !m_bFrameStartSet )
    {
        frameStart = _frameStart;
        d->frameEnd = _frameEnd;
        // Remember that setFrameStartEnd was called. This activates a slightly
        // different mode (when moving start and end positions).
        m_bFrameStartSet = true;
        update();
    }
}

void KoRuler::setRightIndent( double _right )
{
    d->i_right = makeIntern( _right );
    update();
}

void KoRuler::setDirection( bool rtl )
{
    d->rtl = rtl;
    update();
}

void KoRuler::changeFlags(int _flags)
{
    d->flags = _flags;
}

int KoRuler::flags() const
{
    return d->flags;
}

bool KoRuler::doubleClickedIndent() const
{
    return d->doubleClickedIndent;
}

double KoRuler::applyRtlAndZoom( double value ) const
{
    int frameWidth = d->frameEnd - frameStart;
    return d->rtl ? ( frameWidth - zoomIt( value ) ) : zoomIt( value );
}

double KoRuler::unZoomItRtl( int pixValue ) const
{
    int frameWidth = d->frameEnd - frameStart;
    return d->rtl ? ( unZoomIt( (double)(frameWidth - pixValue) ) ) : unZoomIt( (double)pixValue );
}

void KoRuler::slotMenuActivated( int i )
{
    if ( i >= 0 && i <= KoUnit::U_LASTUNIT )
    {
        KoUnit::Unit unit = static_cast<KoUnit::Unit>(i);
        setUnit( unit );
        emit unitChanged( unit );
    }
}

QSize KoRuler::minimumSizeHint() const
{
    QSize size;
    QFont font = KGlobalSettings::toolBarFont();
    QFontMetrics fm( font );

    size.setWidth( qMax( fm.height() + 4, 20 ) );
    size.setHeight( qMax( fm.height() + 4, 20 ) );

    return size;
}

QSize KoRuler::sizeHint() const
{
    return minimumSizeHint();
}

void KoRuler::setPageLayout( const KoPageLayout& _layout )
{
    d->layout = _layout;
    update();
}

#include "KoRuler.moc"
