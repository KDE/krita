/*
 *  kis_tabbar.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  based on kpread code Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
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

#include <stdio.h>
#include <stdlib.h>

#include <qpointarray.h>
#include <qstring.h>
#include <qtimer.h>
#include <qpopupmenu.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <klineeditdlg.h>
#include <kdebug.h>

#include <kis_view.h>
#include <kis_doc.h>


#include "kis_tabbar.h"

KisTabBar::KisTabBar( KisView *_view, KisDoc *_doc )
  : QWidget ( (QWidget*)_view )
  , m_pView ( _view )
  , m_pDoc  ( _doc )
{
    m_pPopupMenu = 0L;

    m_pAutoScrollTimer = new QTimer(this);
    connect( m_pAutoScrollTimer, SIGNAL(timeout()), SLOT(slotAutoScroll()));

    leftTab = 1;
    m_rightTab = 0;
    activeTab = 0;
    m_autoScroll = 0;
}

void KisTabBar::addTab( const QString& _text )
{
    tabsList.append( _text );
    repaint();
}

void KisTabBar::removeTab( const QString& _text )
{
    int i = tabsList.findIndex( _text );
    if ( i == -1 )
        return;

    if ( activeTab == i + 1 )
        activeTab = i;

    if ( activeTab == 0 )
        leftTab = 1;
    else if ( leftTab > activeTab )
        leftTab = activeTab;

    tabsList.remove( _text );
    repaint();
}


void KisTabBar::removeAllTabs()
{
    tabsList.clear();
    activeTab = 0;
    leftTab = 1;
    repaint();
}


void KisTabBar::slotScrollLeft()
{
  if ( tabsList.count() == 0 )
    return;

  if ( leftTab == 1 )
    return;

  leftTab--;
  repaint( false );
}

void KisTabBar::slotScrollRight()
{
  if ( tabsList.count() == 0 )
    return;

  if ( m_rightTab == (int)tabsList.count() )
    return;

  if ( (unsigned int )leftTab == tabsList.count() )
    return;

  leftTab++;
  repaint( false );
}

void KisTabBar::slotScrollFirst()
{
  if ( tabsList.count() == 0 )
    return;

  if ( leftTab == 1 )
    return;

  leftTab = 1;
  repaint( false );
}

void KisTabBar::slotScrollLast()
{
  if ( tabsList.count() == 0 )
    return;

  QPainter painter;
  painter.begin( this );

  int i = tabsList.count();
  int x = 0;

  if ( m_rightTab == i )
    return;

  QStringList::Iterator it;
  it = tabsList.end();
  do
    {
      --it;
      QFontMetrics fm = painter.fontMetrics();

      x += 10 + fm.width( *it );
      if ( x > width() )
	{
	  leftTab = i + 1;
	  break;
	}
      --i;
    } while ( it != tabsList.begin() );
  painter.end();
  repaint( false );
}

void KisTabBar::setActiveTab( const QString& _text )
{
    int i = tabsList.findIndex( _text );
    if ( i == -1 )
        return;

    if ( i + 1 == activeTab )
        return;

    activeTab = i + 1;
    repaint( false );
}

/*
    remove a tab from the tabbar itself - tabs can also be remove
    from menu with "remove currentImg image tab"
*/
void KisTabBar::slotRemove( )
{
    int ret = KMessageBox::warningContinueCancel( this,
				  i18n("You are going to remove the active image.\nDo you want to continue?"),
				  i18n("Remove Image"), i18n("&Remove"));

    if ( ret == KMessageBox::Continue )
    {
        int i = 1;
        QStringList::Iterator it;

        for ( it = tabsList.begin(); it != tabsList.end(); ++it )
	    {
	        if (i == activeTab)
	        {
	            QString img = *it;
	            kdDebug()<<"Removing: "<<img.latin1()<<endl;
	            m_pDoc->slotRemoveImage(img);

                    m_pDoc->setModified( true );
	            return;
	        }
	        i++;
	    }
    }
}


/*
    response to imagelistupdated signal sent from document
*/
void KisTabBar::slotImageListUpdated()
{
    kdDebug() << "KisTabBar::slotImageListUpdated\n";

    // clear list - this also repaints
    removeAllTabs();

    // populate list
    QStringList lst = m_pDoc->images();
    if (!lst.isEmpty())
    {
        QStringList::Iterator it;

        for ( it = lst.begin(); it != lst.end(); ++it )
	        addTab(*it);
    }

    // set active if there is a currentImg image
    // note that currentImg() is the image pointer
    // and currentImage() is the name of that image

    if(m_pDoc->currentImg())
        setActiveTab(m_pDoc->currentImgName());
}


void KisTabBar::slotRename()
{
    renameTab();
}


void KisTabBar::slotAdd()
{
    m_pDoc->slotNewImage();

    m_pDoc->setModified( true );
}


void KisTabBar::paintEvent( QPaintEvent* )
{
    if ( tabsList.count() == 0 )
    {
        erase();
        return;
    }

    QPainter painter;
    QPixmap pm(size());
    pm.fill( backgroundColor() );
    painter.begin( &pm, this );

    if ( leftTab > 1 )
        paintTab( painter, -10, QString(""), 0, 0, false );

    int i = 1;
    int x = 0;
    QString text;
    QString active_text;
    int active_x = -1;
    int active_width = 0;
    int active_y = 0;

    QStringList::Iterator it;
    for ( it = tabsList.begin(); it != tabsList.end(); ++it )
    {
        text = *it;
        QFontMetrics fm = painter.fontMetrics();
        int text_width = fm.width( text );
        int text_y = ( height() - fm.ascent() - fm.descent() ) / 2 + fm.ascent();

        if ( i == activeTab )
	    {
	        active_text = text;
	        active_x = x;
	        active_y = text_y;
	        active_width = text_width;

	        if ( i >= leftTab )  x += 10 + text_width;
	    }
        else if ( i >= leftTab )
	    {
	        paintTab( painter, x, text, text_width, text_y, false);
	        x += 10 + text_width;
	    }

        if ( x - 10 < width() ) m_rightTab = i;

        i++;
    }

    // if ( active_text != 0L )
    paintTab( painter, active_x, active_text, active_width, active_y, true);

    painter.end();
    bitBlt( this, 0, 0, &pm );
}


void KisTabBar::paintTab( QPainter & painter, int x, const QString& text,
    int text_width, int text_y, bool isactive )
{
    QPointArray parr;
    parr.setPoints( 4, x,0, x+10,height()-1, x+10+text_width,height()-1,
        x+20+text_width,0 );
    QRegion reg( parr );
    painter.setClipping( TRUE );
    painter.setClipRegion( reg );
    if ( isactive )
        painter.setBackgroundColor( white );
    else
        painter.setBackgroundColor( colorGroup().background() );
    painter.eraseRect( x, 0, text_width + 20, height() );
    painter.setClipping( FALSE );

    painter.drawLine( x, 0, x + 10, height() - 1 );
    painter.drawLine( x + 10, height() - 1, x + text_width + 10, height() - 1 );
    painter.drawLine( x + 10 + text_width, height() - 1, x + 20 + text_width, 0 );
    if ( !isactive )
        painter.drawLine( x, 0, x + 20 + text_width, 0 );
   painter.drawText( x + 10, text_y , text );
}


void KisTabBar::openPopupMenu( const QPoint &_global )
{
    if ( m_pPopupMenu != 0L )  delete m_pPopupMenu;
    m_pPopupMenu = new QPopupMenu();

    m_pPopupMenu->insertItem( i18n( "Rename Image..." ), this, SLOT( slotRename() ) );
    m_pPopupMenu->insertItem( i18n( "Remove Image" ), this, SLOT( slotRemove() ) );
    m_pPopupMenu->insertItem( i18n( "New Image" ), this, SLOT( slotAdd() ) );
    m_pPopupMenu->popup( _global );
}


void KisTabBar::renameTab()
{
    int i = 1;
    QString imgName;

    QStringList::Iterator it;
    for ( it = tabsList.begin(); it != tabsList.end(); ++it )
    {
        if (i == activeTab)
	        imgName = *it;
        i++;
    }

    bool ok;
    QString activeName = imgName;
    QString newName = KLineEditDlg::getText( i18n("Image Name"), i18n("Enter name:"), activeName, &ok, this );

    // Have a different name ?
    if ( ok ) // User pushed an OK button.
    {
        if ( (newName.stripWhiteSpace()).isEmpty() ) // Image name is empty.
        {
            KNotifyClient::beep();
            KMessageBox::information( this, i18n("Image name cannot be empty."), i18n("Change Image Name") );
            // Recursion
            renameTab();
            return;
        }
        else if ( newName != activeName ) // Image name changed.
        {
             for ( QStringList::Iterator it = tabsList.begin(); it != tabsList.end(); ++it )
             {
                 // Is the name already used
                 if ( (*it) == newName )
                 {
                     KNotifyClient::beep();
                     KMessageBox::information( this, i18n("This name is already used."), i18n("Change Image Name") );
                     // Recursion
                     renameTab();
                     return;
                 }
             }
             m_pDoc->renameImage(imgName, newName);
             m_pDoc->setModified( true );
        }
    }
}

void KisTabBar::mousePressEvent( QMouseEvent* _ev )
{
    int old_active = activeTab;

    if ( tabsList.count() == 0 )
    {
        erase();
        return;
    }

    QPainter painter;
    painter.begin( this );

    int i = 1;
    int x = 0;
    QString text;
    //const char *active_text = 0L; //jwc
    QString active_text = "";

    QStringList::Iterator it;
    for ( it = tabsList.begin(); it != tabsList.end(); ++it )
    {
        text = *it;
	    QFontMetrics fm = painter.fontMetrics();
	    int text_width = fm.width( text );

	    if ( i >= leftTab )
	    {
	        if ( x <= _ev->pos().x() && _ev->pos().y() <= x + 20 + text_width )
	        {
		        activeTab = i;
		        active_text = text;
	        }

	        x += 10 + text_width;
	    }
	    i++;
    }

    painter.end();

    if ( activeTab != old_active )
    {
        repaint( false );
        emit tabSelected( active_text );
    }

    if ( _ev->button() == RightButton )
    {
	    openPopupMenu( _ev->globalPos() );
    }
}


void KisTabBar::slotAutoScroll( )
{
    if ( m_autoScroll == autoScrollLeft && leftTab > 1 )
    {
	    slotScrollLeft();
    }
    else if ( m_autoScroll == autoScrollRight )
    {
        slotScrollRight();
    }

    if ( leftTab <= 1 )
    {
        m_pAutoScrollTimer->stop();
        m_autoScroll = 0;
    }
}

void KisTabBar::mouseDoubleClickEvent( QMouseEvent*  )
{
    renameTab();
}

#include "kis_tabbar.moc"
