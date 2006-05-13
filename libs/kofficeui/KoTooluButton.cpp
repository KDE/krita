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

#include <QApplication>
#include <QToolTip>
#include <QPainter>
#include <qdrawutil.h>
#include <QPixmap>
#include <QStyle>
#include <q3popupmenu.h>
//Added by qt3to4:
#include <QFocusEvent>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>

#include <kglobalsettings.h>
#include <ktoolbar.h>
#include <KoTooluButton.h>
#include <k3colordrag.h>
#include <klocale.h>
#include <kcolordialog.h>
#include <kdebug.h>

namespace {
    // For the KoColorPanel
    const int COLS = 15;
    const int TILESIZE = 16;
    // For the KoToolButton
    int ARROW_WIDTH = 12;
}

KoColorPanel::KoColorPanel( QWidget* parent, const char* name ) :
    QWidget( parent, name, Qt::WStaticContents | Qt::WNoAutoErase | Qt::WResizeNoErase )
{
    setMouseTracking( true );
    setAcceptDrops( true );
    init();
}

KoColorPanel::~KoColorPanel()
{
}

QSize KoColorPanel::sizeHint() const
{
    return minimumSizeHint();
}

QSize KoColorPanel::minimumSizeHint() const
{
    return QSize( COLS << 4, lines() << 4 );
}

Q3PopupMenu* KoColorPanel::createColorPopup( KoColorPanel::MenuStyle style, const QColor& defaultColor,
                                            const QObject* receiver, const char* slot,
                                            QWidget* parent, const char* name )
{
    Q3PopupMenu* menu = new Q3PopupMenu( parent, name );
    KoColorPopupProxy* proxy = 0;

    if ( defaultColor.isValid() ) {
        QPixmap pixmap( 12, 12 );
        QPainter p( &pixmap );
        p.fillRect( 0, 0, 12, 12, defaultColor );
        p.end();
        proxy = new KoColorPopupProxy( defaultColor, 0, menu, "color proxy" );
        connect( proxy, SIGNAL( colorSelected( const QColor& ) ), receiver, slot );
        menu->insertItem( QIcon( pixmap ), i18n( "Default Color" ), proxy, SLOT( slotDefaultColor() ) );
        menu->insertSeparator();
    }

    KoColorPanel* panel = new KoColorPanel( menu, "default colors" );
    panel->insertDefaultColors();
    connect( panel, SIGNAL( colorSelected( const QColor& ) ), receiver, slot );
    menu->insertItem( panel );

    if ( style == CustomColors ) {
        menu->insertSeparator();
        panel = new KoColorPanel( menu, "custom panel" );
        connect( panel, SIGNAL( colorSelected( const QColor& ) ), receiver, slot );
        menu->insertItem( panel );
        if ( !proxy ) {
            proxy = new KoColorPopupProxy( QColor(), panel, menu, "color proxy" );
            connect( proxy, SIGNAL( colorSelected( const QColor& ) ), receiver, slot );
        }
        else
            proxy->setRecentColorPanel( panel );
        menu->insertSeparator();
        menu->insertItem( i18n( "More Colors..." ), proxy, SLOT( slotMoreColors() ) );
    }

    return menu;
}

void KoColorPanel::clear()
{
    if ( m_colorMap.isEmpty() )
        return;

    QSize area( minimumSizeHint() );
    m_colorMap.clear();
    init();
    updateGeometry();
    erase( 0, 0, area.width(), area.height() );
}

void KoColorPanel::insertColor( const QColor& color )
{
    Position pos = m_nextPosition;
    if ( insertColor( color, true ) ) // we want checking for external users
        finalizeInsertion( pos );
}

void KoColorPanel::insertColor( const QColor& color, const QString& toolTip )
{
    Position pos = m_nextPosition;
    if ( insertColor( color, toolTip, true ) ) // we want checking for external users
        finalizeInsertion( pos );
}

void KoColorPanel::insertDefaultColors()
{
    if ( m_defaultsAdded )
        return;
    m_defaultsAdded = true;

    int currentRow = m_nextPosition.y; // we have to repaint this row below

    // Note: No checking for duplicates, so take care when you modify that list
    insertColor(QColor( 255 ,     0 ,     0 ), i18n( "color", "Red" ), false);
    insertColor(QColor( 255 ,   165 ,     0 ), i18n( "color", "Orange" ), false);
    insertColor(QColor( 255 ,     0 ,   255 ), i18n( "color", "Magenta" ), false);
    insertColor(QColor(   0 ,     0 ,   255 ), i18n( "color", "Blue" ), false);
    insertColor(QColor(   0 ,   255 ,   255 ), i18n( "color", "Cyan" ), false);
    insertColor(QColor(   0 ,   255 ,     0 ), i18n( "color", "Green" ), false);
    insertColor(QColor( 255 ,   255 ,     0 ), i18n( "color", "Yellow" ), false);
    insertColor(QColor( 165 ,    42 ,    42 ), i18n( "color", "Brown" ), false);
    insertColor(QColor( 139 ,     0 ,     0 ), i18n( "color", "DarkRed" ), false);
    insertColor(QColor( 255 ,   140 ,     0 ), i18n( "color", "DarkOrange" ), false);
    insertColor(QColor( 139 ,     0 ,   139 ), i18n( "color", "DarkMagenta" ), false);
    insertColor(QColor(   0 ,     0 ,   139 ), i18n( "color", "DarkBlue" ), false);
    insertColor(QColor(   0 ,   139 ,   139 ), i18n( "color", "DarkCyan" ), false);
    insertColor(QColor(   0 ,   100 ,     0 ), i18n( "color", "DarkGreen" ), false);
    insertColor(QColor( 130 ,   127 ,     0 ), i18n( "color", "DarkYellow" ), false);
    insertColor(QColor( 255 ,   255 ,   255 ), i18n( "color", "White" ), false);
    // xgettext:no-c-format
    insertColor(QColor( 229 ,   229 ,   229 ), i18n( "color", "Gray 90%" ), false);
    // xgettext:no-c-format
    insertColor(QColor( 204 ,   204 ,   204 ), i18n( "color", "Gray 80%" ), false);
    // xgettext:no-c-format
    insertColor(QColor( 178 ,   178 ,   178 ), i18n( "color", "Gray 70%" ), false);
    // xgettext:no-c-format
    insertColor(QColor( 153 ,   153 ,   153 ), i18n( "color", "Gray 60%" ), false);
    // xgettext:no-c-format
    insertColor(QColor( 127 ,   127 ,   127 ), i18n( "color", "Gray 50%" ), false);
    // xgettext:no-c-format
    insertColor(QColor( 102 ,   102 ,   102 ), i18n( "color", "Gray 40%" ), false);
    // xgettext:no-c-format
    insertColor(QColor(  76 ,    76 ,    76 ), i18n( "color", "Gray 30%" ), false);
    // xgettext:no-c-format
    insertColor(QColor(  51 ,    51 ,    51 ), i18n( "color", "Gray 20%" ), false);
    // xgettext:no-c-format
    insertColor(QColor(  25 ,    25 ,    25 ), i18n( "color", "Gray 10%" ), false);
    insertColor(QColor(   0 ,     0 ,     0 ), i18n( "color", "Black" ), false);
    insertColor(QColor( 255 ,   255 ,   240 ), i18n( "color", "Ivory" ), false);
    insertColor(QColor( 255 ,   250 ,   250 ), i18n( "color", "Snow" ), false);
    insertColor(QColor( 245 ,   255 ,   250 ), i18n( "color", "MintCream" ), false);
    insertColor(QColor( 255 ,   250 ,   240 ), i18n( "color", "FloralWhite" ), false);
    insertColor(QColor( 255 ,   255 ,   224 ), i18n( "color", "LightYellow" ), false);
    insertColor(QColor( 240 ,   255 ,   255 ), i18n( "color", "Azure" ), false);
    insertColor(QColor( 248 ,   248 ,   255 ), i18n( "color", "GhostWhite" ), false);
    insertColor(QColor( 240 ,   255 ,   240 ), i18n( "color", "Honeydew" ), false);
    insertColor(QColor( 255 ,   245 ,   238 ), i18n( "color", "Seashell" ), false);
    insertColor(QColor( 240 ,   248 ,   255 ), i18n( "color", "AliceBlue" ), false);
    insertColor(QColor( 255 ,   248 ,   220 ), i18n( "color", "Cornsilk" ), false);
    insertColor(QColor( 255 ,   240 ,   245 ), i18n( "color", "LavenderBlush" ), false);
    insertColor(QColor( 253 ,   245 ,   230 ), i18n( "color", "OldLace" ), false);
    insertColor(QColor( 245 ,   245 ,   245 ), i18n( "color", "WhiteSmoke" ), false);
    insertColor(QColor( 255 ,   250 ,   205 ), i18n( "color", "LemonChiffon" ), false);
    insertColor(QColor( 224 ,   255 ,   255 ), i18n( "color", "LightCyan" ), false);
    insertColor(QColor( 250 ,   250 ,   210 ), i18n( "color", "LightGoldenrodYellow" ), false);
    insertColor(QColor( 250 ,   240 ,   230 ), i18n( "color", "Linen" ), false);
    insertColor(QColor( 245 ,   245 ,   220 ), i18n( "color", "Beige" ), false);
    insertColor(QColor( 255 ,   239 ,   213 ), i18n( "color", "PapayaWhip" ), false);
    insertColor(QColor( 255 ,   235 ,   205 ), i18n( "color", "BlanchedAlmond" ), false);
    insertColor(QColor( 250 ,   235 ,   215 ), i18n( "color", "AntiqueWhite" ), false);
    insertColor(QColor( 255 ,   228 ,   225 ), i18n( "color", "MistyRose" ), false);
    insertColor(QColor( 230 ,   230 ,   250 ), i18n( "color", "Lavender" ), false);
    insertColor(QColor( 255 ,   228 ,   196 ), i18n( "color", "Bisque" ), false);
    insertColor(QColor( 255 ,   228 ,   181 ), i18n( "color", "Moccasin" ), false);
    insertColor(QColor( 255 ,   222 ,   173 ), i18n( "color", "NavajoWhite" ), false);
    insertColor(QColor( 255 ,   218 ,   185 ), i18n( "color", "PeachPuff" ), false);
    insertColor(QColor( 238 ,   232 ,   170 ), i18n( "color", "PaleGoldenrod" ), false);
    insertColor(QColor( 245 ,   222 ,   179 ), i18n( "color", "Wheat" ), false);
    insertColor(QColor( 220 ,   220 ,   220 ), i18n( "color", "Gainsboro" ), false);
    insertColor(QColor( 240 ,   230 ,   140 ), i18n( "color", "Khaki" ), false);
    insertColor(QColor( 175 ,   238 ,   238 ), i18n( "color", "PaleTurquoise" ), false);
    insertColor(QColor( 255 ,   192 ,   203 ), i18n( "color", "Pink" ), false);
    insertColor(QColor( 238 ,   221 ,   130 ), i18n( "color", "LightGoldenrod" ), false);
    insertColor(QColor( 211 ,   211 ,   211 ), i18n( "color", "LightGray" ), false);
    insertColor(QColor( 255 ,   182 ,   193 ), i18n( "color", "LightPink" ), false);
    insertColor(QColor( 176 ,   224 ,   230 ), i18n( "color", "PowderBlue" ), false);
    insertColor(QColor( 127 ,   255 ,   212 ), i18n( "color", "Aquamarine" ), false);
    insertColor(QColor( 216 ,   191 ,   216 ), i18n( "color", "Thistle" ), false);
    insertColor(QColor( 173 ,   216 ,   230 ), i18n( "color", "LightBlue" ), false);
    insertColor(QColor( 152 ,   251 ,   152 ), i18n( "color", "PaleGreen" ), false);
    insertColor(QColor( 255 ,   215 ,     0 ), i18n( "color", "Gold" ), false);
    insertColor(QColor( 173 ,   255 ,    47 ), i18n( "color", "GreenYellow" ), false);
    insertColor(QColor( 176 ,   196 ,   222 ), i18n( "color", "LightSteelBlue" ), false);
    insertColor(QColor( 144 ,   238 ,   144 ), i18n( "color", "LightGreen" ), false);
    insertColor(QColor( 221 ,   160 ,   221 ), i18n( "color", "Plum" ), false);
    insertColor(QColor( 190 ,   190 ,   190 ), i18n( "color", "Gray" ), false);
    insertColor(QColor( 222 ,   184 ,   135 ), i18n( "color", "BurlyWood" ), false);
    insertColor(QColor( 135 ,   206 ,   250 ), i18n( "color", "LightSkyBlue" ), false);
    insertColor(QColor( 255 ,   160 ,   122 ), i18n( "color", "LightSalmon" ), false);
    insertColor(QColor( 135 ,   206 ,   235 ), i18n( "color", "SkyBlue" ), false);
    insertColor(QColor( 210 ,   180 ,   140 ), i18n( "color", "Tan" ), false);
    insertColor(QColor( 238 ,   130 ,   238 ), i18n( "color", "Violet" ), false);
    insertColor(QColor( 244 ,   164 ,    96 ), i18n( "color", "SandyBrown" ), false);
    insertColor(QColor( 233 ,   150 ,   122 ), i18n( "color", "DarkSalmon" ), false);
    insertColor(QColor( 189 ,   183 ,   107 ), i18n( "color", "DarkKhaki" ), false);
    insertColor(QColor( 127 ,   255 ,     0 ), i18n( "color", "Chartreuse" ), false);
    insertColor(QColor( 169 ,   169 ,   169 ), i18n( "color", "DarkGray" ), false);
    insertColor(QColor( 124 ,   252 ,     0 ), i18n( "color", "LawnGreen" ), false);
    insertColor(QColor( 255 ,   105 ,   180 ), i18n( "color", "HotPink" ), false);
    insertColor(QColor( 250 ,   128 ,   114 ), i18n( "color", "Salmon" ), false);
    insertColor(QColor( 240 ,   128 ,   128 ), i18n( "color", "LightCoral" ), false);
    insertColor(QColor(  64 ,   224 ,   208 ), i18n( "color", "Turquoise" ), false);
    insertColor(QColor( 143 ,   188 ,   143 ), i18n( "color", "DarkSeaGreen" ), false);
    insertColor(QColor( 218 ,   112 ,   214 ), i18n( "color", "Orchid" ), false);
    insertColor(QColor( 102 ,   205 ,   170 ), i18n( "color", "MediumAquamarine" ), false);
    insertColor(QColor( 255 ,   127 ,    80 ), i18n( "color", "Coral" ), false);
    insertColor(QColor( 154 ,   205 ,    50 ), i18n( "color", "YellowGreen" ), false);
    insertColor(QColor( 218 ,   165 ,    32 ), i18n( "color", "Goldenrod" ), false);
    insertColor(QColor(  72 ,   209 ,   204 ), i18n( "color", "MediumTurquoise" ), false);
    insertColor(QColor( 188 ,   143 ,   143 ), i18n( "color", "RosyBrown" ), false);
    insertColor(QColor( 219 ,   112 ,   147 ), i18n( "color", "PaleVioletRed" ), false);
    insertColor(QColor(   0 ,   250 ,   154 ), i18n( "color", "MediumSpringGreen" ), false);
    insertColor(QColor( 255 ,    99 ,    71 ), i18n( "color", "Tomato" ), false);
    insertColor(QColor( 0   ,   255 ,   127 ), i18n( "color", "SpringGreen" ), false);
    insertColor(QColor( 205 ,   133 ,    63 ), i18n( "color", "Peru" ), false);
    insertColor(QColor( 100 ,   149 ,   237 ), i18n( "color", "CornflowerBlue" ), false);
    insertColor(QColor( 132 ,   112 ,   255 ), i18n( "color", "LightSlateBlue" ), false);
    insertColor(QColor( 147 ,   112 ,   219 ), i18n( "color", "MediumPurple" ), false);
    insertColor(QColor( 186 ,    85 ,   211 ), i18n( "color", "MediumOrchid" ), false);
    insertColor(QColor(  95 ,   158 ,   160 ), i18n( "color", "CadetBlue" ), false);
    insertColor(QColor(   0 ,   206 ,   209 ), i18n( "color", "DarkTurquoise" ), false);
    insertColor(QColor(   0 ,   191 ,   255 ), i18n( "color", "DeepSkyBlue" ), false);
    insertColor(QColor( 119 ,   136 ,   153 ), i18n( "color", "LightSlateGray" ), false);
    insertColor(QColor( 184 ,   134 ,    11 ), i18n( "color", "DarkGoldenrod" ), false);
    insertColor(QColor( 123 ,   104 ,   238 ), i18n( "color", "MediumSlateBlue" ), false);
    insertColor(QColor( 205 ,    92 ,    92 ), i18n( "color", "IndianRed" ), false);
    insertColor(QColor( 210 ,   105 ,    30 ), i18n( "color", "Chocolate" ), false);
    insertColor(QColor(  60 ,   179 ,   113 ), i18n( "color", "MediumSeaGreen" ), false);
    insertColor(QColor(  50 ,   205 ,    50 ), i18n( "color", "LimeGreen" ), false);
    insertColor(QColor(  32 ,   178 ,   170 ), i18n( "color", "LightSeaGreen" ), false);
    insertColor(QColor( 112 ,   128 ,   144 ), i18n( "color", "SlateGray" ), false);
    insertColor(QColor(  30 ,   144 ,   255 ), i18n( "color", "DodgerBlue" ), false);
    insertColor(QColor( 255 ,    69 ,     0 ), i18n( "color", "OrangeRed" ), false);
    insertColor(QColor( 255 ,    20 ,   147 ), i18n( "color", "DeepPink" ), false);
    insertColor(QColor(  70 ,   130 ,   180 ), i18n( "color", "SteelBlue" ), false);
    insertColor(QColor( 106 ,    90 ,   205 ), i18n( "color", "SlateBlue" ), false);
    insertColor(QColor( 107 ,   142 ,    35 ), i18n( "color", "OliveDrab" ), false);
    insertColor(QColor(  65 ,   105 ,   225 ), i18n( "color", "RoyalBlue" ), false);
    insertColor(QColor( 208 ,    32 ,   144 ), i18n( "color", "VioletRed" ), false);
    insertColor(QColor( 153 ,    50 ,   204 ), i18n( "color", "DarkOrchid" ), false);
    insertColor(QColor( 160 ,    32 ,   240 ), i18n( "color", "Purple" ), false);
    insertColor(QColor( 105 ,   105 ,   105 ), i18n( "color", "DimGray" ), false);
    insertColor(QColor( 138 ,    43 ,   226 ), i18n( "color", "BlueViolet" ), false);
    insertColor(QColor( 160 ,    82 ,    45 ), i18n( "color", "Sienna" ), false);
    insertColor(QColor( 199 ,    21 ,   133 ), i18n( "color", "MediumVioletRed" ), false);
    insertColor(QColor( 176 ,    48 ,    96 ), i18n( "color", "Maroon" ), false);
    insertColor(QColor(  46 ,   139 ,    87 ), i18n( "color", "SeaGreen" ), false);
    insertColor(QColor(  85 ,   107 ,    47 ), i18n( "color", "DarkOliveGreen" ), false);
    insertColor(QColor(  34 ,   139 ,    34 ), i18n( "color", "ForestGreen" ), false);
    insertColor(QColor( 139 ,    69 ,    19 ), i18n( "color", "SaddleBrown" ), false);
    insertColor(QColor( 148 ,     0 ,   211 ), i18n( "color", "DarkViolet" ), false);
    insertColor(QColor( 178 ,    34 ,    34 ), i18n( "color", "FireBrick" ), false);
    insertColor(QColor(  72 ,    61 ,   139 ), i18n( "color", "DarkSlateBlue" ), false);
    insertColor(QColor(  47 ,    79 ,    79 ), i18n( "color", "DarkSlateGray" ), false);
    insertColor(QColor(  25 ,    25 ,   112 ), i18n( "color", "MidnightBlue" ), false);
    insertColor(QColor(   0 ,     0 ,   205 ), i18n( "color", "MediumBlue" ), false);
    insertColor(QColor(   0 ,     0 ,   128 ), i18n( "color", "Navy" ), false);

    finalizeInsertion( m_nextPosition );  // with a no-op paint() call as we repaint anyway
    updateGeometry();
    // we have to repaint the "old" current row explicitly due
    // to WStaticContents
    update( 0, currentRow << 4, COLS << 4, 16 );
}

void KoColorPanel::mousePressEvent( QMouseEvent* e )
{
    if ( e->button() == Qt::LeftButton )
        m_pressedPos = e->pos();
}

void KoColorPanel::mouseReleaseEvent( QMouseEvent* )
{
    if ( isVisible() && parentWidget() && parentWidget()->inherits( "QPopupMenu" ) )
        parentWidget()->close();
    emit colorSelected( mapToColor( m_pressedPos ) );
}

void KoColorPanel::mouseMoveEvent( QMouseEvent* e )
{
    if ( e->state() & Qt::LeftButton ) {
        QPoint p = m_pressedPos - e->pos();
        if ( p.manhattanLength() > QApplication::startDragDistance() ) {
            QColor color( mapToColor( m_pressedPos ) );
            if ( color.isValid() ) {
                K3ColorDrag *drag = new K3ColorDrag( color, this, name() );
                drag->dragCopy();
            }
        }
    }
    else
        updateFocusPosition( mapToPosition( e->pos() ) );
}

void KoColorPanel::paintEvent( QPaintEvent* e )
{
    int lns = lines();
    int startRow, endRow, startCol, endCol;
    paintArea( e->rect(), startRow, endRow, startCol, endCol );

    QPainter p( this );

    // First clear all the areas we won't paint on later (only if the widget isn't erased)
    if ( !e->erased() ) {
        // vertical rects
        int tmp = TILESIZE * lns;
        if ( startCol == 0 )
            erase( 0, 0, 2, tmp );
        if ( endCol == COLS )
            erase( width() - 2, 0, 2, tmp );
        else
            erase( ( endCol << 4 ) - 2, 0, 2, tmp );
        int i = startCol == 0 ? 1 : startCol;
        for ( ; i < endCol; ++i )
            erase( ( i << 4 ) - 2, 0, 4, tmp );

        // horizontal rects
        tmp = TILESIZE * COLS;
        if ( startRow == 0 )
            erase( 0, 0, tmp, 2 );
        if ( endRow == lns )
            erase( 0, height() - 2, tmp, 2 );
        else
            erase( 0, ( endRow << 4 ) - 2, tmp, 2 );
        i = startRow == 0 ? 1 : startRow;
        for ( ; i < endRow; ++i )
            erase( 0, ( i << 4 ) - 2, tmp, 4 );
    }

    // The "active" element (if there is one)
    if ( hasFocus() && m_focusPosition.x != -1 && m_focusPosition.y != -1 &&
         mapFromPosition( m_focusPosition ).intersects( e->rect() ) )
        style().drawPrimitive( QStyle::PE_Panel, &p, QRect( m_focusPosition.x << 4, m_focusPosition.y << 4, TILESIZE, TILESIZE ),
                               colorGroup(), QStyle::State_Sunken | QStyle::State_Enabled );

    --lns;  // Attention: We just avoid some lns - 1 statements

    // ...all color tiles
    if ( !m_colorMap.isEmpty() ) {
        int currentRow = startRow, currentCol = startCol;
        while ( currentRow < endRow && currentCol < endCol ) {
            QMap<Position, QColor>::ConstIterator it = m_colorMap.find( Position( currentCol, currentRow ) );
            if( it != m_colorMap.end() )
                p.fillRect( ( currentCol << 4 ) + 2, ( currentRow << 4 ) + 2, 12, 12, it.data() );

            // position of the next cell
            ++currentCol;
            if ( currentCol == endCol ) {
                ++currentRow;
                currentCol = startCol;
            }
        }
    }

    // clean up the last line (it's most likely that it's not totally filled)
    if ( !e->erased() && endRow > lns ) {
        int fields = m_colorMap.count() % COLS;
        erase( fields << 4, lns * TILESIZE, ( COLS - fields ) << 4, 16 );
    }
}

void KoColorPanel::keyPressEvent( QKeyEvent* e )
{
    Position newPos( validPosition( m_focusPosition ) );
    if ( e->key() == Qt::Key_Up ) {
        if ( newPos.y == 0 )
            e->ignore();
        else
            --newPos.y;
    }
    else if ( e->key() == Qt::Key_Down ) {
        if ( newPos < Position( m_colorMap.count() % COLS, lines() - 2 ) )
            ++newPos.y;
        else
            e->ignore();
    }
    else if ( e->key() == Qt::Key_Left ) {
        if ( newPos.x == 0 )
            e->ignore();
        else
            --newPos.x;
    }
    else if ( e->key() == Qt::Key_Right ) {
        if ( newPos.x < COLS - 1 && newPos < Position( m_colorMap.count() % COLS - 1, lines() - 1 ) )
            ++newPos.x;
        else
            e->ignore();
    }
    else if ( e->key() == Qt::Key_Return ) {
        if ( isVisible() && parentWidget() && parentWidget()->inherits( "QPopupMenu" ) )
            parentWidget()->close();
        emit colorSelected( mapToColor( m_focusPosition ) );
    }
    updateFocusPosition( newPos );
}

void KoColorPanel::focusInEvent( QFocusEvent* e )
{
    if ( !m_colorMap.isEmpty() && m_focusPosition.x == -1 && m_focusPosition.y == -1 ) {
        m_focusPosition.x = 0;
        m_focusPosition.y = 0;
    }
    QWidget::focusInEvent( e );
}

void KoColorPanel::dragEnterEvent( QDragEnterEvent* e )
{
    e->accept( K3ColorDrag::canDecode( e ) );
}

void KoColorPanel::dropEvent( QDropEvent* e )
{
    QColor color;
    if ( K3ColorDrag::decode( e, color ) )
        insertColor( color );
}

void KoColorPanel::finalizeInsertion( const Position& pos )
{
    paint( pos );

    if ( !isFocusEnabled() )
        setFocusPolicy( Qt::StrongFocus );
    // Did we start a new row?
    if ( m_nextPosition.x == 1 )
        updateGeometry();
}

bool KoColorPanel::insertColor( const QColor& color, bool checking )
{
    if ( checking && isAvailable( color ) )
        return false;

    m_colorMap.insert( m_nextPosition, color );

    ++m_nextPosition.x;
    if ( m_nextPosition.x == COLS ) {
        m_nextPosition.x = 0;
        ++m_nextPosition.y;
    }
    return true;
}

bool KoColorPanel::insertColor( const QColor& color, const QString& toolTip, bool checking )
{
    if ( checking && isAvailable( color ) )
        return false;

    // Remember the "old" m_nextPosition -- this is the place where the newly
    // inserted color will be located
    QRect rect( mapFromPosition( m_nextPosition ) );
    insertColor( color, false ); // check only once ;)
    this->setToolTip( rect, toolTip );
    return true;
}

bool KoColorPanel::isAvailable( const QColor& color )
{
    // O(n) checking on insert, but this is better than O(n) checking
    // on every mouse move...
    QMap<Position, QColor>::ConstIterator it = m_colorMap.begin();
    QMap<Position, QColor>::ConstIterator end = m_colorMap.end();
    for ( ; it != end; ++it )
        if ( it.data() == color )
            return true;
    return false;
}

KoColorPanel::Position KoColorPanel::mapToPosition( const QPoint& point ) const
{
    return Position( point.x() >> 4, point.y() >> 4 );
}

QColor KoColorPanel::mapToColor( const QPoint& point ) const
{
    return mapToColor( mapToPosition( point ) );
}

QColor KoColorPanel::mapToColor( const KoColorPanel::Position& position ) const
{
    QMap<Position, QColor>::ConstIterator it = m_colorMap.find( position );
    if ( it != m_colorMap.end() )
        return it.data();
    return QColor();
}

QRect KoColorPanel::mapFromPosition( const KoColorPanel::Position& position ) const
{
    return QRect( position.x << 4, position.y << 4, TILESIZE, TILESIZE );
}

KoColorPanel::Position KoColorPanel::validPosition( const Position& position )
{
    Position pos( position );
    int lns = lines() - 1;
    int lastLineLen = m_colorMap.count() % COLS - 1;

    // ensure the position is within the valid grid area
    // note: special handling of the last line
    if ( pos.x < 0 )
        pos.x = 0;
    else if ( pos.y == lns && pos.x > lastLineLen )
        pos.x = lastLineLen;
    else if ( pos.x >= COLS )
        pos.x = COLS - 1;

    if ( pos.y < 0 )
        pos.y = 0;
    else if ( pos.x > lastLineLen && pos.y > lns - 1 )
        pos.y = lns - 1;
    else if ( pos.y > lns )
        pos.y = lns;
    return pos;
}

int KoColorPanel::lines() const
{
    if ( m_colorMap.isEmpty() )
        return 1;
    return ( m_colorMap.count() - 1 ) / COLS + 1;
}

void KoColorPanel::paintArea( const QRect& rect, int& startRow, int& endRow, int& startCol, int& endCol ) const
{
    startRow = rect.top() >> 4;
    endRow = ( rect.bottom() >> 4 ) + 1;
    startCol = rect.left() >> 4;
    endCol = ( rect.right() >> 4 ) + 1;
}

void KoColorPanel::updateFocusPosition( const Position& newPosition )
{
    QPainter p( this );

    // restore the old tile where we had the focus before
    if ( m_focusPosition.x != -1 && m_focusPosition.y != -1 )
        paint( m_focusPosition );

    m_focusPosition = newPosition;

    QMap<Position, QColor>::ConstIterator it = m_colorMap.find( m_focusPosition );
    if ( it != m_colorMap.end() ) {
        // draw at the new focus position
        style().drawPrimitive( QStyle::PE_Panel, &p, QRect( m_focusPosition.x << 4, m_focusPosition.y << 4, TILESIZE, TILESIZE ),
                               colorGroup(), QStyle::State_Sunken | QStyle::State_Enabled );
        p.fillRect( ( m_focusPosition.x << 4 ) + 2, ( m_focusPosition.y << 4 ) + 2, 12, 12, it.data() );
    }

}

void KoColorPanel::paint( const Position& position )
{
    QMap<Position, QColor>::ConstIterator it = m_colorMap.find( position );
    if ( it == m_colorMap.end() )
        return;

    erase( mapFromPosition( position ) );
    QPainter p( this );
    p.fillRect( ( position.x << 4 ) + 2, ( position.y << 4 ) + 2, 12, 12, it.data() );
}

void KoColorPanel::init()
{
    setFocusPolicy( Qt::NoFocus ); // it's empty
    m_nextPosition.x = 0;
    m_nextPosition.y = 0;
    m_focusPosition.x = -1;
    m_focusPosition.y = -1;
    m_defaultsAdded = false;
}

bool operator<( const KoColorPanel::Position& lhs, const KoColorPanel::Position& rhs )
{
    return ( lhs.y * COLS + lhs.x ) < ( rhs.y * COLS + rhs.x );
}


KoColorPopupProxy::KoColorPopupProxy( const QColor& defaultColor, KoColorPanel* recentColors, QObject* parent, const char* name ) :
    QObject( parent, name ), m_defaultColor( defaultColor ), m_recentColors( recentColors )
{
}

void KoColorPopupProxy::setRecentColorPanel( KoColorPanel* recentColors )
{
    m_recentColors = recentColors;
}

void KoColorPopupProxy::slotDefaultColor()
{
    emit colorSelected( m_defaultColor );
}

void KoColorPopupProxy::slotMoreColors()
{
    if ( !m_recentColors )
        return;

    QColor newColor;
    QWidget* p = 0;
    if ( parent() && parent()->isWidgetType() )
        p = static_cast<QWidget*>( parent() );

    if ( KColorDialog::getColor( newColor, p ) == QDialog::Accepted ) {
        m_recentColors->insertColor( newColor );
        emit colorSelected( newColor );
    }
}


KoToolButton::KoToolButton( const QString& icon, int id, QWidget* parent,
                            const char* name, const QString& txt, KInstance* _instance ) :
    KToolBarButton( icon, id, parent, name, txt, _instance ), m_arrowPressed( false )
{
    init();
}

KoToolButton::KoToolButton( const QPixmap& pixmap, int id, QWidget* parent,
                            const char* name, const QString& txt ) :
    KToolBarButton( pixmap, id, parent, name, txt ), m_arrowPressed( false )
{
    init();
}

KoToolButton::~KoToolButton()
{
}

QSize KoToolButton::sizeHint() const
{
    return minimumSizeHint();
}

QSize KoToolButton::minimumSizeHint() const
{
    QSize size = KToolBarButton::minimumSizeHint();
    size.setWidth( size.width() + ARROW_WIDTH );
    return size;
}

QSize KoToolButton::minimumSize() const
{
    return minimumSizeHint();
}

void KoToolButton::colorSelected( const QColor& color )
{
    kDebug() << "selected::: " << color.name() << endl;
}

void KoToolButton::drawButton(QPainter *_painter)
{
    QStyle::State flags = QStyle::State_None;
    QStyle::SubControls active = QStyle::SC_None;
    QStyle::SubControls arrowActive = QStyle::SC_None;
    QStyleOption opt;
    QColorGroup cg( colorGroup() );

    if ( isEnabled() ) {
  	flags |= QStyle::State_Enabled;
        if ( KToolBarButton::isRaised() || m_arrowPressed )
            flags |= QStyle::State_Raised;
    }
    if ( isOn() )
        flags |= QStyle::State_On;

    QStyle::State arrowFlags = flags;

    if ( isDown() && !m_arrowPressed ) {
        flags  |= QStyle::State_DownArrow;
        active |= QStyle::SC_ToolButton;
    }
    if ( m_arrowPressed )
        arrowActive |= QStyle::SC_ToolButton;

    // Draw styled toolbuttons
    _painter->setClipRect( 0, 0, width() - ARROW_WIDTH, height() );
    style().drawComplexControl( QStyle::CC_ToolButton, _painter, this, QRect( 0, 0, width() - ARROW_WIDTH, height() ), cg,
                                flags, QStyle::SC_ToolButton, active, opt );
    _painter->setClipRect( width() - ARROW_WIDTH, 0, ARROW_WIDTH, height() );
    style().drawComplexControl( QStyle::CC_ToolButton, _painter, this, QRect( width(), 0, ARROW_WIDTH, height() ), cg,
                                arrowFlags, QStyle::SC_ToolButton, arrowActive, opt );
    _painter->setClipping( false );

    // ...and the arrow indicating the popup
    style().drawPrimitive( QStyle::PE_ArrowDown, _painter, QRect( width() - ARROW_WIDTH - 1, 0, ARROW_WIDTH, height() ),
                           cg, flags, opt );

    if ( KToolBarButton::isRaised() || m_arrowPressed )
        qDrawShadeLine( _painter, width() - ARROW_WIDTH - 1, 0, width() - ARROW_WIDTH - 1, height() - 1, colorGroup(), true );

    int dx, dy;
    QFont tmp_font( KGlobalSettings::toolBarFont() );
    QFontMetrics fm( tmp_font );
    QRect textRect;
    int textFlags = 0;

    if ( static_cast<KToolBar::IconText>( iconTextMode() ) == KToolBar::IconOnly ) { // icon only
        QPixmap pixmap = iconSet().pixmap( QIcon::Automatic,
                                           isEnabled() ? ( KToolBarButton::isActive() ? QIcon::Active : QIcon::Normal ) :
                                           QIcon::Disabled, isOn() ? QIcon::On : QIcon::Off );
        if ( !pixmap.isNull() ) {
            dx = ( width() - ARROW_WIDTH - pixmap.width() ) / 2;
            dy = ( height() - pixmap.height() ) / 2;
            buttonShift( dx, dy );
            _painter->drawPixmap( dx, dy, pixmap );
        }
    }
    else if ( static_cast<KToolBar::IconText>( iconTextMode() ) == KToolBar::IconTextRight ) { // icon and text (if any)
        QPixmap pixmap = iconSet().pixmap( QIcon::Automatic,
                                           isEnabled() ? ( KToolBarButton::isActive() ? QIcon::Active : QIcon::Normal ) :
                                           QIcon::Disabled, isOn() ? QIcon::On : QIcon::Off );
        if( !pixmap.isNull()) {
            dx = 4;
            dy = ( height() - pixmap.height() ) / 2;
            buttonShift( dx, dy );
            _painter->drawPixmap( dx, dy, pixmap );
        }

        if (!textLabel().isNull()) {
            textFlags = Qt::AlignVCenter | Qt::AlignLeft;
            if ( !pixmap.isNull() )
                dx = 4 + pixmap.width() + 2;
            else
                dx = 4;
            dy = 0;
            buttonShift( dx, dy );
            textRect = QRect( dx, dy, width() - dx, height() );
        }
    }
    else if ( static_cast<KToolBar::IconText>( iconTextMode() ) == KToolBar::TextOnly ) {
        if ( !textLabel().isNull() ) {
            textFlags = Qt::AlignTop | Qt::AlignLeft;
            dx = ( width() - ARROW_WIDTH - fm.width( textLabel() ) ) / 2;
            dy = ( height() - fm.lineSpacing() ) / 2;
            buttonShift( dx, dy );
            textRect = QRect( dx, dy, fm.width(textLabel()), fm.lineSpacing() );
        }
    }
    else if ( static_cast<KToolBar::IconText>( iconTextMode() ) == KToolBar::IconTextBottom ) {
        QPixmap pixmap = iconSet().pixmap( QIcon::Automatic,
                                           isEnabled() ? ( KToolBarButton::isActive() ? QIcon::Active : QIcon::Normal ) :
                                           QIcon::Disabled, isOn() ? QIcon::On : QIcon::Off );
        if( !pixmap.isNull()) {
            dx = ( width() - ARROW_WIDTH - pixmap.width() ) / 2;
            dy = ( height() - fm.lineSpacing() - pixmap.height() ) / 2;
            buttonShift( dx, dy );
            _painter->drawPixmap( dx, dy, pixmap );
        }

        if ( !textLabel().isNull() ) {
            textFlags = Qt::AlignBottom | Qt::AlignHCenter;
            dx = ( width() - ARROW_WIDTH - fm.width( textLabel() ) ) / 2;
            dy = height() - fm.lineSpacing() - 4;
            buttonShift( dx, dy );
            textRect = QRect( dx, dy, fm.width( textLabel() ), fm.lineSpacing() );
        }
    }

    // Draw the text at the position given by textRect, and using textFlags
    if (!textLabel().isNull() && !textRect.isNull()) {
        _painter->setFont( KGlobalSettings::toolBarFont() );
        if ( !isEnabled() )
            _painter->setPen( palette().disabled().dark() );
        else if( KToolBarButton::isRaised() )
            _painter->setPen( KGlobalSettings::toolBarHighlightColor() );
        else
            _painter->setPen( colorGroup().buttonText() );
        _painter->drawText( textRect, textFlags, textLabel() );
    }
}

bool KoToolButton::eventFilter( QObject* o, QEvent* e )
{
    if ( o == m_popup ) {
        if ( e->type() == QEvent::MouseButtonPress )
            if ( hitArrow( mapFromGlobal( static_cast<QMouseEvent*>( e )->globalPos() ) ) ) {
                kDebug() << "KoToolButton::eventFilter-------------->" << endl;
                m_popup->close();
                m_arrowPressed = false;
                return true;
            }
        return false;
    }

    if ( e->type() == QEvent::MouseButtonPress ) {
        m_arrowPressed = hitArrow( static_cast<QMouseEvent*>( e )->pos() );
        if ( m_arrowPressed )
            m_popup->popup( mapToGlobal( QPoint( 0, height() ) ) );
    }
    else if ( e->type() == QEvent::MouseButtonRelease )
        m_arrowPressed = false;
    return KToolBarButton::eventFilter( o, e );
}

void KoToolButton::init()
{
    m_popup = KoColorPanel::createColorPopup( KoColorPanel::CustomColors, Qt::yellow, this,
                                              SLOT( colorSelected( const QColor& ) ),
                                              this, "no-name" );
    // We are interested in the mouse clicks on the arrow button
    m_popup->installEventFilter( this );

    ARROW_WIDTH = style().pixelMetric(QStyle::PM_MenuButtonIndicator) + 4;
    kDebug() << "##################### Arrow: " << ARROW_WIDTH << endl;
}

void KoToolButton::buttonShift( int& dx, int& dy )
{
    if ( isDown() && !m_arrowPressed ) {
        dx += style().pixelMetric( QStyle::PM_ButtonShiftHorizontal );
        dy += style().pixelMetric( QStyle::PM_ButtonShiftVertical );
    }
}

bool KoToolButton::hitArrow( const QPoint& pos )
{
    return QRect( width() - ARROW_WIDTH, 0, ARROW_WIDTH, height() ).contains( pos );
}

#include <KoTooluButton.moc>
