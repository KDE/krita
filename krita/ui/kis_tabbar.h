/*
 *  kis_tabbar.h - part of KImageShop
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

#ifndef __kimageshop_tabbar_h__
#define __kimageshop_tabbar_h__

#include <qwidget.h>
#include <qpainter.h>
#include <qstrlist.h>

class KisView;
class KisDoc;

class KisTabBar : public QWidget
{
    Q_OBJECT

public:
    KisTabBar( KisView *_view, KisDoc *_doc );

signals:
    void tabSelected( const QString& _text );

public slots:
    void slotImageListUpdated();
    void slotScrollLeft();
    void slotScrollRight();    
    void slotScrollFirst();
    void slotScrollLast();

protected slots:
    void slotRename( );
    void slotRemove( );
    void slotAdd();
    void slotAutoScroll( ); 

protected:
    /**
     * Adds a tab to the bar and paints it. The tab does not become active.
     * call @ref #setActiveTab to do so.
     */
    void addTab( const QString& _text );
    /**
     * Removes the tab from the bar. If the tab was the active one then the one
     * left of it ( or if not available ) the one right of it will become active.
     * It is recommended to call @ref #setActiveTab after a call to this function.
     */
    void removeTab( const QString& _text );
    
    /**
     * Removes all tabs from the bar and repaints the widget.
     */
    void removeAllTabs();
    
    /**
     * Highlights this tab.
     */
    void setActiveTab( const QString& _text );

    /**
     * Opens a dialog to rename active tab.
     */
    void renameTab();
    
    /**
     * Open a context menu.
     */
    void openPopupMenu( const QPoint &_global );

    virtual void paintEvent ( QPaintEvent* _ev );
    virtual void mousePressEvent ( QMouseEvent* _ev );
    virtual void mouseDoubleClickEvent ( QMouseEvent* _ev );

    void paintTab( QPainter & painter, int x, const QString& text,
		   int text_width, int text_y, bool isactive);
    
    void openPopupMenu( QPoint &_global );
    
    KisView *m_pView;
    KisDoc  *m_pDoc;
    
    enum { autoScrollNo = 0, autoScrollLeft, autoScrollRight };
    
    /**
     * Pointer to the last popup menu.
     * If this pointer is not 0L delete before usage.
     */
    QPopupMenu *m_pPopupMenu;
    
    /**
     * List with the names of all tabs. The order in this list determines the
     * order of appearance.
     */
    QStringList tabsList;
     
    /**
     * Timer that causes the tabbar to scroll when the user drag a tab.
     */
    QTimer* m_pAutoScrollTimer;

    /**
     * This is the first visible tab on the left of the bar.
     */
    int leftTab;
    
    /**
     * This is the last fully visible tab on the right of the bar.
     */
    int m_rightTab;
    
    /**
     * The active tab in the range form 1..n.
     * If this value is 0, that means that no tab is active.
     */
    int activeTab;

    /**
     * Indicates whether a tab may be scrolled while moving a table.
     * Used to provide a timeout.
     */
    bool m_mayScroll;
    
    /**
     * Indicates the direction the tabs are scrolled to.
     */
    int m_autoScroll;
};

#endif
