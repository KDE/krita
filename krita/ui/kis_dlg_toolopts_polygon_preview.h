/*
 *  kis_dlg_toolopts_polygon_preview.h - part of Krayon
 *
 *  Base code from Kontour.
 *  Copyright (C) 1998 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)
 *
 *  Copyright (c) 2001 Toshitaka Fujioka <fujioka@kde.org>
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


#ifndef kis_dlg_toolopts_polygon_preview_h_
#define kis_dlg_toolopts_polygon_preview_h_

#include <qframe.h>

class PolygonPreview : public QFrame {

    Q_OBJECT

public:
    PolygonPreview( QWidget* parent = 0L, const char* name = 0L, int _nCorners = 3, int _sharpness = 0, 
                    bool _isConcave = false, int _lineThickness = 4 );

    virtual QSize sizeHint() const { return QSize( 150, 150 ); }
    virtual QSize minimumSizeHint() const { return QSize( 70, 70 ); }
    virtual QSizePolicy sizePolicy() const { return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ); }

public slots:
    void slotConvexPolygon();
    void slotConcavePolygon();
    void slotConersValue( int value );
    void slotSharpnessValue( int value );
    void slotThicknessValue( int value );

protected:
    virtual void paintEvent( QPaintEvent *e );

private:
    int nCorners;
    int sharpness;
    bool isConcave;
    int lineThickness;
};

#endif
