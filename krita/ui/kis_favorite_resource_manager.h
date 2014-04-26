/* This file is part of the KDE project
   Copyright 2009 Vera Lukman <shicmap@gmail.com>

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

#ifndef KIS_FAVORITE_RESOURCE_MANAGER_H
#define KIS_FAVORITE_RESOURCE_MANAGER_H

#include <QObject>
#include <kis_types.h>
#include <QQueue>
#include <QList>
#include "KoResourceServerObserver.h"

#include <KoColor.h>

class QString;
class QColor;
class QStringList;
class QToolButton;
class QPoint;
class KoID;
class KisPaintopBox;
class KisPaletteManager;
class KisView2;

class KisFavoriteResourceManager : public QObject, public KoResourceServerObserver<KisPaintOpPreset>
{
    Q_OBJECT

public:

    KisFavoriteResourceManager(KisPaintopBox *paintopBox);
    ~KisFavoriteResourceManager();

    void setPopupPalette(QWidget *palette);

    virtual void unsetResourceServer();

    static const int MAX_FAVORITE_PRESETS = 10;

    void showPaletteManager();
    QList<QImage> favoritePresetImages();

    /**
     * Checks if newBrush is saved as a favorite brush.
     * @returns -1 if the newBrush is not yet saved, then newBrush will be appended
     *         or the position of the brush on the list otherwise
     */
    int addFavoritePreset(const QString& name);
    void removeFavoritePreset(int);
    void removeFavoritePreset(const QString& name);

    /// @return -1 if paintop is not in the list, returns the paintop position otherwise
    int isFavoriteBrushSaved(const QString& name);
    int numFavoritePresets();

    QStringList favoritePresetList();

    int recentColorsTotal();
    const KoColor& recentColorAt(int pos);

    // Reimplemented from KoResourceServerObserver
    virtual void removingResource(KisPaintOpPreset* resource);
    virtual void resourceAdded(KisPaintOpPreset* resource);
    virtual void resourceChanged(KisPaintOpPreset* resource);
    virtual void syncTaggedResourceView();
    virtual void syncTagAddition(const QString& tag);
    virtual void syncTagRemoval(const QString& tag);

    /**
     * Set palette to block updates, paintops won't be deleted when they are deleted from server
     * Used when overwriting a resource
     */
    void setBlockUpdates(bool block);

signals:

    void sigSetFGColor(const KoColor& c);

    // This is a flag to handle a bug:
    // If pop up palette is visible and a new colour is selected, the new colour
    // will be added when the user clicks on the canvas to hide the palette
    // In general, we want to be able to store recent colours if the pop up palette
    // is not visible
    void sigEnableChangeColor(bool b);

    void sigChangeFGColorSelector(const QColor&);

    void setSelectedColor(int);

    void updatePalettes();

    void hidePalettes();

public slots:

    void slotChangeActivePaintop(int);

    /*update the priority of a colour in m_colorList, used only by m_popupPalette*/
    void slotUpdateRecentColor(int);

    /*add a colour to m_colorList, used by KisCanvasResourceProvider*/
    void slotAddRecentColor(const KoColor&);

    void slotChangeFGColorSelector(KoColor c);

private:

    KisPaletteManager *m_favoriteBrushManager;
    KisPaintopBox *m_paintopBox;

    QStringList m_favoritePresetsList;

    class ColorDataList;
    ColorDataList *m_colorList;

    bool m_blockUpdates;

    void saveFavoritePresets();

};

#endif
