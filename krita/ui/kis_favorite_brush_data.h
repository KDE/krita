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

#ifndef KIS_FAVORITE_BRUSH_DATA_H
#define KIS_FAVORITE_BRUSH_DATA_H

#include <QObject>
#include <kis_types.h>

class QToolButton;
class QIcon;
class KoFavoriteResourceManager;

class KisFavoriteBrushData : public QObject
{
    Q_OBJECT

public:
    KisFavoriteBrushData(KoFavoriteResourceManager*, KisPaintOpPresetSP, QIcon * = 0);
    ~KisFavoriteBrushData();
    KisPaintOpPresetSP paintopPreset();
    void setIcon (QIcon*);
    QToolButton* paintopButton();

signals:
    void signalPaintOpChanged(KisPaintOpPresetSP paintop);

private:
    KoFavoriteResourceManager* m_favoriteResourceManager;
    QToolButton* m_button;
    KisPaintOpPresetSP  m_data;
    
private slots:
    void slotBrushButtonClicked();
};

#endif // KIS_FAVORITE_BRUSH_DATA_H
