/* This file is part of the KDE project
   Copyright 2009 Vera Lukman <shichan.karachu@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KIS_POPUP_PALETTE_H
#define KIS_POPUP_PALETTE_H

#include <kis_types.h>
#include <QtGui/QWidget>
#include <QQueue>
#include <KoColor.h>

class KisFavoriteBrushData;
class KoFavoriteResourceManager;
class QWidget;


class KisPopupPalette : public QWidget
{
    Q_OBJECT
    Q_PROPERTY (int hoveredBrush READ hoveredBrush WRITE setHoveredBrush);
    Q_PROPERTY (int selectedBrush READ selectedBrush WRITE setSelectedBrush);
    Q_PROPERTY (int hoveredColor READ hoveredColor WRITE setHoveredColor);
    Q_PROPERTY (int selectedColor READ selectedColor WRITE setSelectedColor);

public:
    KisPopupPalette(KoFavoriteResourceManager* , QWidget *parent=0);
    ~KisPopupPalette();
    QSize sizeHint() const;

    void showPopupPalette (const QPoint&);

    //functions to set up selectedBrush
    void setSelectedBrush( int x );
    int selectedBrush() const;
    //functions to set up selectedColor
    void setSelectedColor( int x );
    int selectedColor() const;

protected:
    void paintEvent (QPaintEvent*);
    void resizeEvent (QResizeEvent*);
    void mouseReleaseEvent (QMouseEvent*);
    void mouseMoveEvent (QMouseEvent*);
    void mousePressEvent (QMouseEvent*);

    //functions to calculate index of favorite brush or recent color in array
    //n is the total number of favorite brushes or recent colors
    int calculateIndex(QPointF, int n);

    //functions to set up hoveredBrush
    void setHoveredBrush( int x );
    int hoveredBrush() const;
    //functions to set up hoveredColor
    void setHoveredColor( int x );
    int hoveredColor() const;


private:
    QPainterPath drawDonutPathFull(int, int, int, int);
    QPainterPath drawDonutPathAngle(int, int, int);
    bool isPointInPixmap(QPointF&, int pos);

    //inline functions
    inline int brushRadius(){ return 50; }
    inline float PI(){ return 3.14159265; }
    inline float brushInnerRadius(){ return width()/2 - 60; }
    inline float brushOuterRadius(){ return width()/2 - 40; }
    inline float colorInnerRadius(){ return width()/2 - 30; }
    inline float colorOuterRadius(){ return width()/2 - 10; }

private:
    int colorFoo;//TEMPORARY
    int m_hoveredBrush;
    int m_selectedBrush;
    int m_hoveredColor;
    int m_selectedColor;
    KoFavoriteResourceManager* m_resourceManager;

signals:
    void sigSelectNewColor();
    void sigChangeActivePaintop(int);
    void sigUpdateRecentColor(int);
    void sigAddRecentColor(KoColor);

 private slots:
    void slotSelectNewColor();
};

#endif // KIS_POPUP_PALETTE_H
