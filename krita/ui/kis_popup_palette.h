/* This file is part of the KDE project
   Copyright 2009 Vera Lukman <vla24@sfu.ca>

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
#include <QToolBox>
#include <QQueue>

class KisFavoriteBrushData;
class KisRecentColorData;
class KoFavoriteResourceManager;
class FlowLayout;
class QWidget;
class QToolButton;
class QSize;


class KisPopupPalette : public QToolBox
{
    Q_OBJECT;

public:
    KisPopupPalette(KoFavoriteResourceManager* , QWidget *parent=0);
    QSize sizeHint() const;
    ~KisPopupPalette();
    static const int BUTTON_SIZE = 25;

    // note: pass pointers here instead of references, since we
    // keep pointers to KisFavoriteBrushData everywhere.
    void addFavoriteBrushButton(KisFavoriteBrushData*);
    void removeFavoriteBrushButton(KisFavoriteBrushData*);
    void addRecentColorButton(KisRecentColorData*);
    void removeRecentColorButton(KisRecentColorData*);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    KoFavoriteResourceManager* m_resourceManager;

    int colorFoo;

    QPoint dragPosition;

    void updatePalette();

    /**The Layout for favorite brushes on the palette**/
    FlowLayout *m_brushButtonLayout;

    /**The Layout for recently used colors on the palette**/
    FlowLayout *m_colorLayout;


private slots:
    void slotPickNewColor();

};

#endif // KIS_POPUP_PALETTE_H
