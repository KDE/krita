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
#include <KisTag.h>

#include <KoColor.h>
#include <KoResource.h>

class QString;
class KisPaintopBox;
class KisPaintOpPreset;

class KisFavoriteResourceManager : public QObject, public KoResourceServerObserver<KisPaintOpPreset>
{
    Q_OBJECT

public:

    KisFavoriteResourceManager(KisPaintopBox *paintopBox);
    ~KisFavoriteResourceManager() override;

    void unsetResourceServer() override;

    QList<QImage> favoritePresetImages();

    void setCurrentTag(const KisTagSP tag);

    int numFavoritePresets();

    QVector<KisPaintOpPresetSP> favoritePresetList();

    int recentColorsTotal();
    const KoColor& recentColorAt(int pos);

    // Reimplemented from KoResourceServerObserver
    void removingResource(QSharedPointer<KisPaintOpPreset> resource) override;
    void resourceAdded(QSharedPointer<KisPaintOpPreset> resource) override;
    void resourceChanged(QSharedPointer<KisPaintOpPreset> resource) override;
    void syncTaggedResourceView() override;
    void syncTagAddition(const QString& tag) override;
    void syncTagRemoval(const QString& tag) override;

    //BgColor;
    KoColor bgColor() const;

    /**
     * Set palette to block updates, paintops won't be deleted when they are deleted from server
     * Used when overwriting a resource
     */
    void setBlockUpdates(bool block);

Q_SIGNALS:

    void sigSetFGColor(const KoColor& c);
    void sigSetBGColor(const KoColor& c);

    // This is a flag to handle a bug:
    // If pop up palette is visible and a new colour is selected, the new colour
    // will be added when the user clicks on the canvas to hide the palette
    // In general, we want to be able to store recent colours if the pop up palette
    // is not visible
    void sigEnableChangeColor(bool b);

    void sigChangeFGColorSelector(const KoColor&);

    void setSelectedColor(int);

    void updatePalettes();

    void hidePalettes();

public Q_SLOTS:

    void slotChangeActivePaintop(int);

    /*update the priority of a colour in m_colorList, used only by m_popupPalette*/
    void slotUpdateRecentColor(int);

    /*add a colour to m_colorList, used by KisCanvasResourceProvider*/
    void slotAddRecentColor(const KoColor&);

    void slotChangeFGColorSelector(KoColor c);

    void slotSetBGColor(const KoColor c);

private Q_SLOTS:
    void updateFavoritePresets();
    void configChanged();

private:

    // Loads the favorite preset list for the first time
    void init();

    KisPaintopBox *m_paintopBox;

    QVector<KisPaintOpPresetSP> m_favoritePresetsList;

    class ColorDataList;
    ColorDataList *m_colorList;

    bool m_blockUpdates;

    void saveFavoritePresets();

    KoColor m_bgColor;
    KisTagSP m_currentTag;

    bool m_initialized;

    int m_maxPresets;
};

#endif
