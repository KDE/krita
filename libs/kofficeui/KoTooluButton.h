/* This file is part of the KDE project
   Copyright (C) 2002 Werner Trobin <trobin@kde.org>

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

#ifndef _kotoolbutton_h_
#define _kotoolbutton_h_


#include <qmap.h>
#include <qpoint.h>
#include <kinstance.h>
//Added by qt3to4:
#include <QPixmap>
#include <QFocusEvent>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QDropEvent>
#include <Q3PopupMenu>
#include <QDragEnterEvent>
#include <QMouseEvent>

class Q3PopupMenu;

class KoColorPanel : public QWidget
{
    Q_OBJECT
public:
    KoColorPanel( QWidget* parent = 0, const char* name = 0 );
    virtual ~KoColorPanel();

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

    enum MenuStyle { Plain, CustomColors };
    static Q3PopupMenu* createColorPopup( MenuStyle style, const QColor& defaultColor,
                                         const QObject* receiver, const char* slot,
                                         QWidget* parent, const char* name );

public slots:
    void clear();
    void insertColor( const QColor& color );
    void insertColor( const QColor& color, const QString& toolTip );
    void insertDefaultColors();

signals:
    void colorSelected( const QColor& color );

protected:
    virtual void mousePressEvent( QMouseEvent* e );
    virtual void mouseReleaseEvent( QMouseEvent* e );
    virtual void mouseMoveEvent( QMouseEvent* e );
    virtual void paintEvent( QPaintEvent* e );
    virtual void keyPressEvent( QKeyEvent* e );
    virtual void focusInEvent( QFocusEvent* e );
    virtual void dragEnterEvent( QDragEnterEvent* e );
    virtual void dropEvent( QDropEvent* e );

private:
    // The position of the 16x16 tiles in "tile steps"
    struct Position {
        Position() : x( -1 ), y( -1 ) {}
        Position( short x_, short y_ ) : x( x_ ), y( y_ ) {}

        short x;
        short y;
    };
    friend bool operator<( const KoColorPanel::Position& lhs, const KoColorPanel::Position& rhs );

    void finalizeInsertion( const Position& pos );
    bool insertColor( const QColor& color, bool checking );
    bool insertColor( const QColor& color, const QString& toolTip, bool checking );
    bool isAvailable( const QColor& color );

    Position mapToPosition( const QPoint& point ) const;
    QColor mapToColor( const QPoint& point ) const;
    QColor mapToColor( const Position& position ) const;
    QRect mapFromPosition( const Position& position ) const;
    Position validPosition( const Position& position );

    int lines() const;
    void paintArea( const QRect& rect, int& startRow, int& endRow, int& startCol, int& endCol ) const;
    void updateFocusPosition( const Position& newPosition );
    void paint( const Position& position );
    void init();

    Position m_nextPosition, m_focusPosition;
    QMap<Position, QColor> m_colorMap;
    QPoint m_pressedPos;
    bool m_defaultsAdded;
};

// Needed for the use of KoColorPanel::Position in QMap
bool operator<( const KoColorPanel::Position& lhs, const KoColorPanel::Position& rhs );


// A tiny class needed to emit the correct signal when the default
// color item in the color-panel popup is activated. Additionally
// it's used to provide the color select dialog and manages the recent
// colors... hacky
class KoColorPopupProxy : public QObject
{
    Q_OBJECT
public:
    KoColorPopupProxy( const QColor& defaultColor, KoColorPanel* recentColors, QObject* parent, const char* name );
    virtual ~KoColorPopupProxy() {}

    void setRecentColorPanel( KoColorPanel* recentColors );

public slots:
    void slotDefaultColor();
    void slotMoreColors();

signals:
    void colorSelected( const QColor& color );

private:
    QColor m_defaultColor;
    KoColorPanel* m_recentColors;
};


// Parts of the code are from KToolBarButton
class KoToolButton : public KToolBarButton
{
    Q_OBJECT
public:
    /**
     * Construct a button with an icon loaded by the button itself.
     * This will trust the button to load the correct icon with the
     * correct size.
     *
     * @param icon   Name of icon to load (may be absolute or relative)
     * @param id     Id of this button
     * @param parent This button's parent
     * @param name   This button's internal name
     * @param txt    This button's text (in a tooltip or otherwise)
     */
    KoToolButton( const QString& icon, int id, QWidget* parent,
                  const char* name = 0L, const QString& txt = QString::null,
                  KInstance* _instance = KGlobal::instance() );

    /**
     * Construct a button with an existing pixmap.  It is not
     * recommended that you use this as the internal icon loading code
     * will almost always get it "right".
     *
     * @param icon   Name of icon to load (may be absolute or relative)
     * @param id     Id of this button
     * @param parent This button's parent
     * @param name   This button's internal name
     * @param txt    This button's text (in a tooltip or otherwise)
     */
    KoToolButton( const QPixmap& pixmap, int id, QWidget* parent,
                  const char* name = 0L, const QString& txt = QString::null );

    virtual ~KoToolButton();

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    virtual QSize minimumSize() const;

public slots:
    void colorSelected( const QColor& color );

protected:
    virtual void drawButton(QPainter *p);
    virtual bool eventFilter( QObject* o, QEvent* e );

private:
    void init();
    void buttonShift( int& dx, int& dy );
    bool hitArrow( const QPoint& pos );

    Q3PopupMenu* m_popup;
    bool m_arrowPressed;
};

#endif // _kotoolbutton_h_
