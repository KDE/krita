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

class KisFavoriteBrushData;
class KisRecentColorData;
class KoFavoriteResourceManager;
class QWidget;


class KisPopupPalette : public QWidget
{
    Q_OBJECT

public:
    KisPopupPalette(KoFavoriteResourceManager* , QWidget *parent=0);
    ~KisPopupPalette();
    QSize sizeHint() const;

    void showPopupPalette (const QPoint&);

    static int const BUTTON_SIZE = 25;

protected:
    void paintEvent (QPaintEvent*);
    void resizeEvent (QResizeEvent*);
    void mouseReleaseEvent (QMouseEvent*);

    //functions to calculate favorite brush position in array
    int calculateRound(float);
    int calculateFavoriteBrush(QPointF);
    int max(int x,int y);

private:
    KoFavoriteResourceManager* m_resourceManager;

    QPainterPath drawColorDonutPath(int, int);
    QPainterPath drawBrushDonutPath(int, int);

private slots:
    void slotPickNewColor();

signals:
    void changeActivePaintop(int);
};

#endif // KIS_POPUP_PALETTE_H
