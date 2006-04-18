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

#ifndef kotabbar_h
#define kotabbar_h

#include <QWidget>
#include <QStringList>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QPaintEvent>
#include <koffice_export.h>
class KoTabBarPrivate;

/**
 * The KoTabBar class provides a tab bar, for use to switch active
 * page/sheet in a document.
 *
 * The tab bar is typically used in the main view. 
 * It is the small widget on the bottom left corner. 
 * Pages/sheets are displayed as tabs, clicking on
 * one of the tab will activate the corresponding sheet (this is actually
 * done in main view). Current active page/sheet is marked by bold text.
 *
 * The tab bar supports page/sheet reorder by dragging a certain tab
 * and move it to another location. The main view should handle the necessary
 * action to perform the actual reorder.
 *
 * There are four scroll buttons available in the tab bar. They are used
 * to shift the tabs in left and right direction, for example when there
 * are more tabs than the space available to display all tabs.
 *
 * Since a page/sheet can be hidden, the tab bar only shows the visible page/sheet.
 * When a hidden page or sheet is shown again, it will be on the same position as
 * before it was hidden.
 *
 * When the document is protected, it is necessary to set the tab bar as
 * read-only using setReadOnly (see also readOnly). If it is set to true,
 * tabs can not be moved by dragging and context menu will not be displayed.
 *
 * @short A bar with tabs and scroll buttons.
 */
class KOFFICEUI_EXPORT KoTabBar : public QWidget
{
    Q_OBJECT
    
    Q_PROPERTY( QString activeTab READ activeTab WRITE setActiveTab )
    Q_PROPERTY( bool readOnly READ readOnly WRITE setReadOnly )
    Q_PROPERTY( QStringList tabs READ tabs WRITE setTabs )
    Q_PROPERTY( unsigned count READ count )
    
public:

    /**
     * Creates a new tabbar.
     */
    KoTabBar( QWidget* parent = 0, const char *name = 0 );

    /**
     * Destroy the tabbar.
     */
    virtual ~KoTabBar();

    /**
     * Returns true if the tab bar is read only.
     */
    bool readOnly() const;
    
    /**
     * Returns true if tabs and scroll buttons will be laid out in a mirrored 
     * (right to left) fashion.
     */
    bool reverseLayout() const;
    
    /**
     * Returns all the tab as list of strings.
     */
    QStringList tabs() const;

    /**
     * Returns number of tabs.
     * This is the same as KoTabBar::tabs().count()
     */
    unsigned count() const;

    /**
     * Returns the active tab.
     */
    QString activeTab() const;

    /**
     * Returns true if it is possible to scroll one tab back.
     *
     * \sa scrollBack
     */
    bool canScrollBack() const;

    /**
     * Returns true if it is possible to scroll one tab forward.
     *
     * \sa scrollForward
     */
    bool canScrollForward() const;

    /**
     * Ensures that specified tab is visible.
     */
    void ensureVisible( const QString& tab );

public slots:

    /**
     * Replaces all tabs with the list of strings.
     */
    void setTabs( const QStringList& list );

    /**
     * Sets the tab bar to be read only.
     *
     * If the tab bar is read only, tab reordering is not allowed.
     * This means that signal tabMoved, contextMenu and doubleClicked
     * would not be emitted.
     */
    void setReadOnly( bool ro );

    /**
     * If reverse is true, dialogs and scroll buttonswidgets will be laid out in a mirrored
     * as if the sheet is in right to left languages (such as Arabic and Hebrew)
     */
    void setReverseLayout( bool reverse );

    /**
     * Adds a tab to the tab bar.
     */
    void addTab( const QString& text );

    /**
     * Removes a tab from the bar. If the tab was the active one then
     * no tab will be active.
     * It is recommended to call setActiveTab after a call to this function.
     */
    void removeTab( const QString& text );

    /**
     * Renames a tab.
     */
    void renameTab( const QString& old_name, const QString& new_name );
    
    /**
     * Moves a tab to another position and reorder other tabs.
     *
     * Example 1: if there are four tabs A - B - C - D, then
     * moveTab(2,1) will yield A - C - B - D. This is because
     * 2nd tab (i.e C) is moved to a position before 1th tab (i.e B).
     *
     * Example 2: for these tabs: X - Y - Z, moveTab(0,3) will
     * move tab X after tab Z so that the result is Y - Z - X.
     */
    void moveTab( int tab, int target );

    /**
     * Scrolls one tab back. Does nothing if the leftmost tab (rightmost tab
     * when reverseLayout is true) is already the first tab.
     *
     * \sa canScrollBack
     */
    void scrollBack();

    /**
     * Scrolls one tab forward. Does nothing if the rightmost tab (leftmost tab 
     * when reverseLayout is true) is already the last tab.
     *
     * \sa canScrollForward
     */
    void scrollForward();

    /**
     * Scrolls to the first tab. Does nothing if the leftmost tab (rightmost tab
     * when reverseLayout is true) is already the first tab.
     *
     * \sa canScrollBack
     */
    void scrollFirst();

    /**
     * Scrolls to the last tab. Does nothing if the rightmost tab (leftmost tab 
     * when reverseLayout is true) is already the last tab.
     *
     * \sa canScrollForward
     */
    void scrollLast();

    /**
     * Sets active tab.
     */
    void setActiveTab( const QString& text );

    /**
     * Removes all tabs.
     */
    void clear();
    
    QSize sizeHint() const;

signals:

    /**
     * Emitted if the active tab changed. 
     */
    void tabChanged( const QString& _text );

    /**
     * This signal is emitted whenever a tab is dragged, moved and
     * released. This means that the user wants to move a tab into
     * another position (right before target).
     *
     * When the signal is emitted, the tabs are not reordered.
     * Therefore if you just ignore this signal, than no tab reorder
     * is possible. Call moveTab (from the slot connected to this signal)
     * to perform the actual tab reorder.
     */
    void tabMoved( unsigned tab, unsigned target );

    /**
     * This signal is emitted whenever the tab bar is right-clicked.
     * Typically it is used to popup a context menu.
     */
    void contextMenu( const QPoint& pos );

    /**
     * This signal is emitted whenever the tab bar is double-clicked.
     */
    void doubleClicked();

protected slots:
    void autoScrollBack();
    void autoScrollForward();

protected:
    virtual void paintEvent ( QPaintEvent* ev );
    virtual void resizeEvent( QResizeEvent* ev );
    virtual void mousePressEvent ( QMouseEvent* ev );
    virtual void mouseReleaseEvent ( QMouseEvent* ev );
    virtual void mouseDoubleClickEvent ( QMouseEvent* ev );
    virtual void mouseMoveEvent ( QMouseEvent* ev );
    virtual void wheelEvent ( QWheelEvent * e );

private:
    KoTabBarPrivate *d;

    // don't allow copy or assignment
    KoTabBar( const KoTabBar& );
    KoTabBar& operator=( const KoTabBar& );
};


#endif // kotabbar_h
