/* This file is part of the KDE project
   Copyright (C) 2003 Ariya Hidayat <ariya@kde.org>
   Copyright (C) 2003 Norbert Andres <nandres@web.de>
   Copyright (C) 2002 Laurent Montel <montel@kde.org>
   Copyright (C) 1999 David Faure <faure@kde.org>
   Copyright (C) 1999 Boris Wedl <boris.wedl@kfunigraz.ac.at>
   Copyright (C) 1998-2000 Torben Weis <weis@kde.org>

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

#include "KoTabBar.h"

#include <qdrawutil.h>
#include <QPainter>
#include <QString>
#include <QStringList>
#include <QStyle>
#include <QTimer>
#include <QToolButton>
#include <q3valuevector.h>
#include <QWidget>
//Added by qt3to4:
#include <QWheelEvent>
#include <QPixmap>
#include <QPaintEvent>
#include <Q3PointArray>
#include <QResizeEvent>
#include <QMouseEvent>

// TODO
// improvement possibilities
// - use offscreen buffer to reduce flicker even more
// - keep track of tabs, only (re)layout when necessary
// - paint all tabs to buffer, show only by shifting
// - customizable button pixmaps
// - use QStyle to paint the tabs & buttons (is it good/possible?)


class KoTabBarPrivate
{
public:
    KoTabBar* tabbar;

    // scroll buttons
    QToolButton* scrollFirstButton;
    QToolButton* scrollLastButton;
    QToolButton* scrollBackButton;
    QToolButton* scrollForwardButton;

    // read-only: no mouse drag, double-click, right-click
    bool readOnly;
    
    // if true, layout is from right to left
    bool reverseLayout;

    // list of all tabs, in order of appearance
    QStringList tabs;

    // array of QRect for each visible tabs
    Q3ValueVector<QRect> tabRects;

    // leftmost tab (or rightmost if reverseLayout)
    int firstTab;
    
    // rightmost tab (or leftmost if reverseLayout)
    int lastTab;

    // the active tab in the range form 1..n.
    // if this value is 0, that means that no tab is active.
    int activeTab;

    // unusable space on the left, taken by the scroll buttons
    int offset;

    // when the user drag the tab (in order to move it)
    // this is the target position, it's 0 if no tab is dragged
    int targetTab;

    // wheel movement since selected tab was last changed by the
    // mouse wheel
    int wheelDelta;

    // true if autoscroll is active
    bool autoScroll;

    // calculate the bounding rectangle for each visible tab
    void layoutTabs();

    // reposition scroll buttons
    void layoutButtons();

    // find a tab whose bounding rectangle contains the pos
    // return -1 if no such tab is found
    int tabAt( const QPoint& pos );

    // draw a single tab
    void drawTab( QPainter& painter, QRect& rect, const QString& text, bool active );

    // draw a marker to indicate tab moving
    void drawMoveMarker( QPainter& painter, int x, int y );

    // update the enable/disable status of scroll buttons
    void updateButtons();

};

// built-in pixmap for scroll-first button
static const char * arrow_leftmost_xpm[] = {
"10 10 2 1",
" 	c None",
".	c #000000",
"          ",
"  .    .  ",
"  .   ..  ",
"  .  ...  ",
"  . ....  ",
"  .  ...  ",
"  .   ..  ",
"  .    .  ",
"          ",
"          "};

// built-in pixmap for scroll-last button
static const char * arrow_rightmost_xpm[] = {
"10 10 2 1",
" 	c None",
".	c #000000",
"          ",
"  .    .  ",
"  ..   .  ",
"  ...  .  ",
"  .... .  ",
"  ...  .  ",
"  ..   .  ",
"  .    .  ",
"          ",
"          "};

// built-in pixmap for scroll-left button
static const char * arrow_left_xpm[] = {
"10 10 2 1",
" 	c None",
".	c #000000",
"          ",
"      .   ",
"     ..   ",
"    ...   ",
"   ....   ",
"    ...   ",
"     ..   ",
"      .   ",
"          ",
"          "};

// built-in pixmap for scroll-right button
static const char * arrow_right_xpm[] = {
"10 10 2 1",
" 	c None",
".	c #000000",
"          ",
"   .      ",
"   ..     ",
"   ...    ",
"   ....   ",
"   ...    ",
"   ..     ",
"   .      ",
"          ",
"          "};


void KoTabBarPrivate::layoutTabs()
{
    tabRects.clear();

    QPainter painter( tabbar );

    QFont f = painter.font();
    f.setBold( true );
    painter.setFont( f );
    QFontMetrics fm = painter.fontMetrics();
    
    if( !reverseLayout )
    {
        // left to right
        int x = 0;
        for( int c = 0; c < tabs.count(); c++ )
        {
            QRect rect;
            if( c >= firstTab-1 )
            {
                QString text = tabs[ c ];
                int tw = fm.width( text ) + 4;
                rect = QRect( x, 0, tw + 20, tabbar->height() );
                x = x + tw + 20;
            }
            tabRects.append( rect );
        }

        lastTab = tabRects.count();
        for( int i = 0; i < tabRects.count(); i++ )
            if( tabRects[i].right()-10+offset > tabbar->width() )
            {
                lastTab = i;
                break;
            }
    }
    else
    {
        // right to left
        int x = tabbar->width() - offset;
        for( int c = 0; c < tabs.count(); c++ )
        {
            QRect rect;
            if( c >= firstTab-1 )
            {
                QString text = tabs[ c ];
                int tw = fm.width( text ) + 4;
                rect = QRect( x - tw - 20, 0, tw + 20, tabbar->height() );
                x = x - tw - 20;
            }
            tabRects.append( rect );
        }

        lastTab = tabRects.count();
        for( int i = tabRects.count()-1; i>0; i-- )
            if( tabRects[i].left() > 0 )
            {
                lastTab = i+1;
                break;
            }
    }    
}

int KoTabBarPrivate::tabAt( const QPoint& pos )
{
    for( int i = 0; i < tabRects.count(); i++ )
    {
      QRect rect = tabRects[ i ];
      if( rect.isNull() ) continue;
      if( rect.contains( pos ) ) return i;
    }

    return -1; // not found
}

void KoTabBarPrivate::drawTab( QPainter& painter, QRect& rect, const QString& text, bool active )
{
    Q3PointArray polygon;
    
    if( !reverseLayout )
        polygon.setPoints( 6, rect.x(), rect.y(),
            rect.x(), rect.bottom()-3,
            rect.x()+2, rect.bottom(),
            rect.right()-4, rect.bottom(),
            rect.right()-2, rect.bottom()-2,
            rect.right()+5, rect.top() );
    else      
        polygon.setPoints( 6, rect.right(), rect.top(),
            rect.right(), rect.bottom()-3,
            rect.right()-2, rect.bottom(),
            rect.x()+4, rect.bottom(),
            rect.x()+2, rect.bottom()-2,
            rect.x()-5, rect.top() );

    painter.save();

    // fill it first  
    QBrush bg = tabbar->palette().background();
    if( active )
       bg = tabbar->palette().base();
    painter.setBrush( bg );
    painter.setPen( QPen( Qt::NoPen ) );
    painter.drawPolygon( polygon );

    // draw the lines
    painter.setPen( tabbar->palette().color( QPalette::Dark) );
    if( !active )
      painter.drawLine( rect.x()-25, rect.y(), rect.right()+25, rect.top() );
    // Qt4: painter.setRenderHint( QPainter::Antialiasing );
    painter.drawPolyline( polygon );

    painter.setPen( tabbar->palette().color( QPalette::ButtonText ) );
    QFont f = painter.font();
    if( active ) f.setBold( true );
    painter.setFont( f );
    QFontMetrics fm = painter.fontMetrics();
    int tx =  rect.x() + ( rect.width() - fm.width( text ) ) / 2;
    int ty =  rect.y() + ( rect.height() - fm.height() ) / 2 + fm.ascent();
    painter.drawText( tx, ty, text );

    painter.restore();
}

void KoTabBarPrivate::drawMoveMarker( QPainter& painter, int x, int y )
{
    Q3PointArray movmark;
    movmark.setPoints( 3, x, y, x + 7, y, x + 4, y + 6);
    QBrush oldBrush = painter.brush();
    painter.setBrush( Qt::black );
    painter.drawPolygon(movmark);
    painter.setBrush( oldBrush );
}

void KoTabBarPrivate::layoutButtons()
{
    int bw = tabbar->height();
    int w = tabbar->width();
    offset = bw * 4;
    
    if( !reverseLayout )
    {
        scrollFirstButton->setGeometry( 0, 0, bw, bw );
        scrollFirstButton->setIcon( QIcon( arrow_leftmost_xpm ) );
        scrollBackButton->setGeometry( bw, 0, bw, bw );
        scrollBackButton->setIcon( QIcon( arrow_left_xpm ) );
        scrollForwardButton->setGeometry( bw*2, 0, bw, bw );
        scrollForwardButton->setIcon( QIcon( arrow_right_xpm ) );
        scrollLastButton->setGeometry( bw*3, 0, bw, bw );
        scrollLastButton->setIcon( QIcon( arrow_rightmost_xpm ) );
    }
    else
    {
        scrollFirstButton->setGeometry( w-bw, 0, bw, bw );
        scrollFirstButton->setIcon( QIcon( arrow_rightmost_xpm ) );
        scrollBackButton->setGeometry( w-2*bw, 0, bw, bw );
        scrollBackButton->setIcon( QIcon( arrow_right_xpm ) );
        scrollForwardButton->setGeometry( w-3*bw, 0, bw, bw );
        scrollForwardButton->setIcon( QIcon( arrow_left_xpm ) );
        scrollLastButton->setGeometry( w-4*bw, 0, bw, bw );
        scrollLastButton->setIcon( QIcon( arrow_leftmost_xpm ) );
    }
 }

void KoTabBarPrivate::updateButtons()
{
    scrollFirstButton->setEnabled( tabbar->canScrollBack() );
    scrollBackButton->setEnabled( tabbar->canScrollBack() );
    scrollForwardButton->setEnabled( tabbar->canScrollForward() );
    scrollLastButton->setEnabled( tabbar->canScrollForward() );
}

// creates a new tabbar
KoTabBar::KoTabBar( QWidget* parent, const char* /*name*/ )
    : QWidget( parent, Qt::WResizeNoErase | Qt::WNoAutoErase )
{
    d = new KoTabBarPrivate;
    d->tabbar = this;
    d->readOnly = false;
    d->reverseLayout = false;
    d->firstTab = 1;
    d->lastTab = 0;
    d->activeTab = 0;
    d->targetTab = 0;
    d->wheelDelta = 0;
    d->autoScroll = false;
    d->offset = 64;

    // initialize the scroll buttons
    d->scrollFirstButton = new QToolButton( this );
    connect( d->scrollFirstButton, SIGNAL( clicked() ),
      this, SLOT( scrollFirst() ) );
    d->scrollLastButton = new QToolButton( this );
    connect( d->scrollLastButton, SIGNAL( clicked() ),
      this, SLOT( scrollLast() ) );
    d->scrollBackButton = new QToolButton( this );
    connect( d->scrollBackButton, SIGNAL( clicked() ),
      this, SLOT( scrollBack() ) );
    d->scrollForwardButton = new QToolButton( this );
    connect( d->scrollForwardButton, SIGNAL( clicked() ),
      this, SLOT( scrollForward() ) );
    d->layoutButtons();
    d->updateButtons();
}

// destroys the tabbar
KoTabBar::~KoTabBar()
{
    delete d;
}

// adds a new visible tab
void KoTabBar::addTab( const QString& text )
{
    d->tabs.append( text );

    update();
}

// removes a tab
void KoTabBar::removeTab( const QString& text )
{
    int i = d->tabs.indexOf( text );
    if ( i == -1 ) return;

    if ( d->activeTab == i + 1 )
        d->activeTab = 0;

    d->tabs.removeAll( text );

    update();
}

// removes all tabs
void KoTabBar::clear()
{
    d->tabs.clear();
    d->activeTab = 0;
    d->firstTab = 1;

    update();
}

bool KoTabBar::readOnly() const
{
    return d->readOnly;
}

void KoTabBar::setReadOnly( bool ro )
{
    d->readOnly = ro;
}

bool KoTabBar::reverseLayout() const
{
    return d->reverseLayout;
}

void KoTabBar::setReverseLayout( bool reverse )
{
    if( reverse != d->reverseLayout )
    {
        d->reverseLayout = reverse;
        d->layoutTabs();
        d->layoutButtons();
        d->updateButtons();
        update();
    }
}

void KoTabBar::setTabs( const QStringList& list )
{
    QString left, active;

    if( d->activeTab > 0 )
        active = d->tabs[ d->activeTab-1 ];
    if( d->firstTab > 0 )
        left = d->tabs[ d->firstTab-1 ];

    d->tabs = list;

    if( !left.isNull() )
    {
        d->firstTab = d->tabs.indexOf( left ) + 1;
        if( d->firstTab > (int)d->tabs.count() )
            d->firstTab = 1;
        if( d->firstTab <= 0 )
            d->firstTab = 1;
    }

    d->activeTab = 0;
    if( !active.isNull() )
        setActiveTab( active );

    update();
}

QStringList KoTabBar::tabs() const
{
    return d->tabs;
}

unsigned KoTabBar::count() const
{
    return d->tabs.count();
}

bool KoTabBar::canScrollBack() const
{
    if ( d->tabs.count() == 0 )
        return false;

    return d->firstTab > 1;
}

bool KoTabBar::canScrollForward() const
{
    if ( d->tabs.count() == 0 )
        return false;
        
    return d->lastTab < (int)d->tabs.count();
}

void KoTabBar::scrollBack()
{
    if ( !canScrollBack() )
        return;

    d->firstTab--;
    if( d->firstTab < 1 ) d->firstTab = 1;

    d->layoutTabs();
    d->updateButtons();
    update();
}

void KoTabBar::scrollForward()
{
    if ( !canScrollForward() )
        return;

    d->firstTab ++;
    if( d->firstTab > (int)d->tabs.count() )
        d->firstTab = d->tabs.count();

    d->layoutTabs();
    d->updateButtons();
    update();
}

void KoTabBar::scrollFirst()
{
    if ( !canScrollBack() )
        return;

    d->firstTab = 1;
    d->layoutTabs();
    d->updateButtons();
    update();
}

void KoTabBar::scrollLast()
{
    if ( !canScrollForward() )
        return;

    d->layoutTabs();

    if( !d->reverseLayout )
    {
        int fullWidth = d->tabRects[ d->tabRects.count()-1 ].right();
        int delta = fullWidth - width() + d->offset;
        for( int i = 0; i < d->tabRects.count(); i++ )
            if( d->tabRects[i].x() > delta )
            {
                d->firstTab = i+1;
                break;
            }
    }
    else
    {
        // FIXME optimize this, perhaps without loop
        for( ; d->firstTab <= (int)d->tabRects.count();)
        {
            int x = d->tabRects[ d->tabRects.count()-1 ].x();
            if( x > 0 ) break;
            d->firstTab++;
            d->layoutTabs();
        }
    }

    d->layoutTabs();
    d->updateButtons();
    update();
}

void KoTabBar::ensureVisible( const QString& tab )
{
    int i = d->tabs.indexOf( tab );
    if ( i == -1 )
        return;
    i++;

    // already visible, then do nothing
    if( ( i >= d->firstTab ) && ( i <= d->lastTab ) )
      return;

    if( i < d->firstTab )
        while( i < d->firstTab )
            scrollBack();

    if( i > d->lastTab )
        while( i > d->lastTab )
            scrollForward();
}

void KoTabBar::moveTab( int tab, int target )
{
    QString tabName = d->tabs.takeAt( tab );

    if( target > tab )
	target--;

    if( target >= d->tabs.count() )
	d->tabs.append( tabName );
    else
        d->tabs.insert( target, tabName );
	
    if( d->activeTab == tab+1 )
        d->activeTab = target+1;

    update();
}

void KoTabBar::setActiveTab( const QString& text )
{
    int i = d->tabs.indexOf( text );
    if ( i == -1 )
        return;

    if ( i + 1 == d->activeTab )
        return;

    d->activeTab = i + 1;
    d->updateButtons();
    update();

    emit tabChanged( text );
}

void KoTabBar::autoScrollBack()
{
    if( !d->autoScroll ) return;

    scrollBack();

    if( !canScrollBack() )
        d->autoScroll = false;
    else
        QTimer::singleShot( 400, this, SLOT( autoScrollBack() ) );
}

void KoTabBar::autoScrollForward()
{
    if( !d->autoScroll ) return;

    scrollForward();

    if( !canScrollForward() )
        d->autoScroll = false;
    else
        QTimer::singleShot( 400, this, SLOT( autoScrollForward() ) );
}

void KoTabBar::paintEvent( QPaintEvent* )
{
    if ( d->tabs.count() == 0 )
    {
        update();
        return;
    }

    QPainter painter;
    QPixmap pm( size() );
    pm.fill( palette().color( QPalette::Window ) );
    painter.begin( &pm );

    painter.setPen( palette().color(QPalette::Dark) );
    painter.drawLine( 0, 0, width(), 0 );

    if( !d->reverseLayout )
        painter.translate( 5, 0 );

    d->layoutTabs();
    d->updateButtons();
    
    // draw first all non-active, visible tabs
    for( int c = d->tabRects.count()-1; c>=0; c-- )
    {
        QRect rect = d->tabRects[ c ];
        if( rect.isNull() ) continue;
        QString text = d->tabs[ c ];
        d->drawTab( painter, rect, text, false );
    }

    // draw the active tab
    if( d->activeTab > 0 )
    {
        QRect rect = d->tabRects[ d->activeTab-1 ];
        if( !rect.isNull() )
        {
            QString text = d->tabs[ d->activeTab-1 ];
            d->drawTab( painter, rect, text, true );
        }
    }

    // draw the move marker
    if( d->targetTab > 0 )
    {
        int p = qMin( d->targetTab, (int)d->tabRects.count() );
        QRect rect = d->tabRects[ p-1 ];
        if( !rect.isNull() )
        {
            int x = !d->reverseLayout ? rect.x() : rect.right()-7;
            if( d->targetTab > (int)d->tabRects.count() )
              x = !d->reverseLayout ? rect.right()-7 : rect.x()-3;
            d->drawMoveMarker( painter, x, rect.y() );
        }
    }
    painter.end();
      
    painter.begin( this );
    
    if( !d->reverseLayout )
         painter.drawPixmap( d->offset, 0, pm );
    else
         painter.drawPixmap( 0, 0, pm );
    
    painter.end();
}

void KoTabBar::resizeEvent( QResizeEvent* )
{
    d->layoutButtons();
    d->updateButtons();
    update();
}

QSize KoTabBar::sizeHint() const
{
#warning "kde4: port it !"
    //return QSize( 40, style().pixelMetric( QStyle::PM_ScrollBarExtent, this ) );
	return QSize();
}

void KoTabBar::renameTab( const QString& old_name, const QString& new_name )
{
    d->tabs.replace( d->tabs.indexOf( old_name ), new_name );

    update();
}

QString KoTabBar::activeTab() const
{
    if( d->activeTab == 0 )
        return QString::null;
    else
        return d->tabs[ d->activeTab ];
}

void KoTabBar::mousePressEvent( QMouseEvent* ev )
{
    if ( d->tabs.count() == 0 )
    {
        update();
        return;
    }

    d->layoutTabs();

    QPoint pos = ev->pos();
    if( !d->reverseLayout ) pos = pos - QPoint( d->offset,0 );

    int tab = d->tabAt( pos ) + 1;
    if( ( tab > 0 ) && ( tab != d->activeTab ) )
    {
        d->activeTab = tab;
        update();

        emit tabChanged( d->tabs[ d->activeTab-1] );

        // scroll if partially visible
        if( d->tabRects[ tab-1 ].right() > width() - d->offset )
            scrollForward();
    }

    if( ev->button() == Qt::RightButton )
    if( !d->readOnly )
        emit contextMenu( ev->globalPos() );
}

void KoTabBar::mouseReleaseEvent( QMouseEvent* ev )
{
    if ( d->readOnly ) return;

    d->autoScroll = false;

    if ( ev->button() == Qt::LeftButton && d->targetTab != 0 )
    {
        emit tabMoved( d->activeTab-1, d->targetTab-1 );
        d->targetTab = 0;
    }
}

void KoTabBar::mouseMoveEvent( QMouseEvent* ev )
{
    if ( d->readOnly ) return;

    QPoint pos = ev->pos();
    if( !d->reverseLayout) pos = pos - QPoint( d->offset,0 );
    
    // check if user drags a tab to move it
    int i = d->tabAt( pos ) + 1;
    if( ( i > 0 ) && ( i != d->targetTab ) )
    {
        if( i == d->activeTab ) i = 0;
        if( i == d->activeTab+1 ) i = 0;

        if( i != d->targetTab )
        {
           d->targetTab = i;
           d->autoScroll = false;
           update();
        }
    }

    // drag past the very latest visible tab
    // e.g move a tab to the last ordering position
    QRect r = d->tabRects[ d->tabRects.count()-1 ];
    bool moveToLast = false;
    if( r.isValid() )
    {
        if( !d->reverseLayout )
        if( pos.x() > r.right() )
        if( pos.x() < width() )
            moveToLast = true;
        if( d->reverseLayout )
        if( pos.x() < r.x() )
        if( pos.x() > 0 )
            moveToLast = true;
    }
    if( moveToLast )
    if( d->targetTab != (int)d->tabRects.count()+1 )
    {
        d->targetTab = d->tabRects.count()+1;
        d->autoScroll = false;
        update();
    }

    // outside far too left ? activate autoscroll...
    if ( pos.x() < 0 && !d->autoScroll  )
    {
        d->autoScroll = true;
        autoScrollBack();
    }

    // outside far too right ? activate autoscroll...
    int w = width() - d->offset;
    if ( pos.x() > w && !d->autoScroll )
    {
        d->autoScroll = true;
        autoScrollForward();
    }
}

void KoTabBar::mouseDoubleClickEvent( QMouseEvent* ev )
{
    int offset = d->reverseLayout ? 0 : d->offset;
    if( ev->pos().x() > offset )
    if( !d->readOnly )
        emit doubleClicked();
}

void KoTabBar::wheelEvent( QWheelEvent * e )
{
  if ( d->tabs.count() == 0 )
  {
      update();
      return;
  }

  // Currently one wheel movement is a delta of 120.
  // The 'unused' delta is stored for devices that allow
  // a higher scrolling resolution.
  // The delta required to move one tab is one wheel movement:
  const int deltaRequired = 120;

  d->wheelDelta += e->delta();
  int tabDelta = - (d->wheelDelta / deltaRequired);
  d->wheelDelta = d->wheelDelta % deltaRequired;
  int numTabs = d->tabs.size();

  if(d->activeTab + tabDelta > numTabs)
  {
    // Would take us past the last tab
    d->activeTab = numTabs;
  }
  else if (d->activeTab + tabDelta < 1)
  {
    // Would take us before the first tab
    d->activeTab = 1;
  }
  else
  {
    d->activeTab = d->activeTab + tabDelta;
  }

  // Find the left and right edge of the new tab.  If we're
  // going forward, and the right of the new tab isn't visible
  // then scroll forward.  Likewise, if going back, and the 
  // left of the new tab isn't visible, then scroll back.
  int activeTabRight = d->tabRects[ d->activeTab-1 ].right();
  int activeTabLeft  = d->tabRects[ d->activeTab-1 ].left();
  if(tabDelta > 0 && activeTabRight > width() - d->offset )
  {
    scrollForward();
  }
  else if(tabDelta < 0 && activeTabLeft < width() - d->offset )
  {
    scrollBack();
  }

  update();
  emit tabChanged( d->tabs[ d->activeTab-1] );
}


#include "KoTabBar.moc"
