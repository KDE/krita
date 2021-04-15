/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2009 Vera Lukman <shicmap@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KIS_FAVORITE_RESOURCE_MANAGER_H
#define KIS_FAVORITE_RESOURCE_MANAGER_H

#include <QObject>
#include <kis_types.h>
#include <QQueue>
#include <QList>
#include "KoResourceServerObserver.h"
#include <KisTag.h>
#include "KisTagFilterResourceProxyModel.h"

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
    QVector<QString> favoritePresetNamesList();

    void setCurrentTag(const KisTagSP tag);

    int numFavoritePresets();

    void updateFavoritePresets();

    int recentColorsTotal();
    const KoColor& recentColorAt(int pos);

    // Reimplemented from KoResourceServerObserver
    void removingResource(QSharedPointer<KisPaintOpPreset> resource) override;
    void resourceAdded(QSharedPointer<KisPaintOpPreset> resource) override;
    void resourceChanged(QSharedPointer<KisPaintOpPreset> resource) override;

    //BgColor;
    KoColor bgColor() const;


Q_SIGNALS:

    void sigSetFGColor(const KoColor& c);
    void sigSetBGColor(const KoColor& c);

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

    /** Clears the color history shown in the popup palette. */
    void slotClearHistory();

private Q_SLOTS:

    void configChanged();

private:

    void init();

    KisPaintopBox *m_paintopBox;

    class ColorDataList;
    ColorDataList *m_colorList;

    void saveFavoritePresets();

    KoColor m_bgColor;
    KisTagSP m_currentTag;

    bool m_initialized;

    int m_maxPresets;

    KisTagModel* m_tagModel;
    KisTagFilterResourceProxyModel* m_resourcesProxyModel;
    KisResourceModel* m_resourceModel;

};

#endif
