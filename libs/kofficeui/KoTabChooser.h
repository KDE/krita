/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef koTabChooser_h
#define koTabChooser_h

#include <q3frame.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <Q3PopupMenu>
#include <koffice_export.h>
class QMouseEvent;
class QPainter;
class Q3PopupMenu;

/**
 *  class KoTabChooser
 */

class KoTabChooserPrivate;

class KOFFICEUI_EXPORT KoTabChooser : public Q3Frame
{
    Q_OBJECT

public:
    enum { TAB_LEFT = 1,
           TAB_CENTER = 2,
           TAB_RIGHT = 4,
           TAB_DEC_PNT = 8,
           TAB_ALL = TAB_LEFT | TAB_CENTER | TAB_RIGHT | TAB_DEC_PNT };

    KoTabChooser( QWidget *parent, int _flags );
    ~KoTabChooser();

    int getCurrTabType() { return currType; }

    /**
     * put m_bReadWrite to true as default
     * and used setReadWrite(false) to make in readOnly mode
     */
    void setReadWrite(bool _readWrite);

protected:
    void mousePressEvent( QMouseEvent *e );
    void drawContents( QPainter *painter );
    void setupMenu();

    int flags;
    int currType;
    Q3PopupMenu *rb_menu;
    int mLeft;
    int mRight;
    int mCenter;
    int mDecPoint;

    KoTabChooserPrivate *d;

protected slots:
    void rbLeft() { currType = TAB_LEFT; update(); }
    void rbCenter() { currType = TAB_CENTER; update(); }
    void rbRight() { currType = TAB_RIGHT; update(); }
    void rbDecPoint() { currType = TAB_DEC_PNT; update(); }

};

#endif
