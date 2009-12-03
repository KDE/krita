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

#ifndef KO_FAVORITE_RESOURCE_MANAGER_H
#define KO_FAVORITE_RESOURCE_MANAGER_H

#include <QObject>
#include <kis_types.h>
#include <QQueue>
#include <QList>
#include <QPixmap>
#include "kis_recent_color_data.h"

class QString;
class QColor;
class QStringList;
class QToolButton;
class QPoint;
class KoID;
class KisPopupPalette;
class KisPaintopBox;
class KisPaletteManager;
class KisView2;

class KoFavoriteResourceManager : public QObject
{
    Q_OBJECT

public:

    KoFavoriteResourceManager(KisPaintopBox*, QWidget* = 0);
    ~KoFavoriteResourceManager();

    static const int MAX_FAVORITE_BRUSHES = 9;
    static const int MAX_RECENT_COLORS = 10;

    /************************************Popup Palette************************************/

    void showPaletteManager();
    void resetPopupPaletteParent(QWidget * = 0);
    QList<QPixmap> favoriteBrushPixmaps();
    QPixmap favoriteBrushPixmap(int);
    bool isPopupPaletteVisible();

    /**********************************Favorite Brushes***********************************/

    /**Checks if newBrush is saved as a favorite brush.
    Returns -1 if the newBrush is not yet saved, then newBrush will be appended
    Returns the position of the brush on the list otherwise**/
    int addFavoriteBrush (KisPaintOpPresetSP);
    void removeFavoriteBrush(int);
    int favoriteBrushesTotal();

    QStringList favoriteBrushesStringList();


    /***********************************Recent Colors************************************/

    /**Checks if newColor is a recently used color.
    Returns -1 if the newColor is not used recently.
    Returns the position of the newColor on the list otherwise**/
    void addRecentColor(KisRecentColorData*);
    QQueue<KisRecentColorData*>* recentColorsList();
    
public slots:
    void slotChangePaintopLabel();
    void slotShowPopupPalette(const QPoint& = QPoint(0,0));
    void slotChangeActivePaintop(int);

private:
    KisPaletteManager *m_favoriteBrushManager;
    KisPopupPalette* m_popupPalette;
    KisPaintopBox* m_paintopBox;

    QList<KisPaintOpPresetSP> m_favoriteBrushesList;

    /**The list of recently used colors**/
    QQueue<KisRecentColorData*> m_recentColorsData;

    bool isFavoriteBrushesFull();
    int isInRecentColor(QColor&);
    void saveFavoriteBrushes();
};

#endif // KIS_FAVORITE_BRUSH_DATA_H
