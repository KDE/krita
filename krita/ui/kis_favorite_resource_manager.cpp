/* This file is part of the KDE project
   Copyright 2009 Vera Lukman <shicmap@gmail.com>
   Copyright 2011 Sven Langkamp <sven.langkamp@gmail.com>

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

#include <QDebug>
#include <QPoint>
#include <QStringList>
#include <QString>
#include <QColor>
#include <kis_paintop_registry.h>
#include <KoToolManager.h>
#include <kis_paintop_preset.h>
#include <KoID.h>
#include <kconfig.h>
#include <kglobalsettings.h>
#include "kis_favorite_resource_manager.h"
#include "kis_popup_palette.h"
#include "kis_paintop_box.h"
#include "kis_palette_manager.h"
#include "kis_view2.h"
#include "kis_resource_server_provider.h"

#ifndef _MSC_EXTENSIONS
const int KisFavoriteResourceManager::MAX_FAVORITE_PRESETS;
//const int KisFavoriteResourceManager::MAX_RECENT_COLORS;
#endif

KisFavoriteResourceManager::KisFavoriteResourceManager(KisPaintopBox *paintopBox)
    : m_favoriteBrushManager(0)
    , m_popupPalette(0)
    , m_paintopBox(paintopBox)
    , m_colorList(0)
    , m_blockUpdates(false)
{
    //take favorite brushes from a file then append to QList
    KConfigGroup group(KGlobal::config(), "favoriteList");
    m_favoritePresetsList = (group.readEntry("favoritePresets")).split(',', QString::SkipEmptyParts);

    m_colorList = new KisColorDataList();

    KoResourceServer<KisPaintOpPreset>* rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    rServer->addObserver(this);
}

KisFavoriteResourceManager::~KisFavoriteResourceManager()
{
    KoResourceServer<KisPaintOpPreset>* rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    rServer->removeObserver(this);
    delete m_favoriteBrushManager;
    delete m_colorList;
}

void KisFavoriteResourceManager::setPopupPalette(QWidget *palette)
{
    m_popupPalette = qobject_cast<KisPopupPalette*>(palette);
    m_popupPalette->showPopupPalette(false);
}

void KisFavoriteResourceManager::unsetResourceServer()
{
    // ...
}

QStringList KisFavoriteResourceManager::favoritePresetList()
{
    return m_favoritePresetsList;
}

//Popup Palette
void KisFavoriteResourceManager::slotShowPopupPalette(const QPoint &p)
{
    if (!m_popupPalette) {
        return;
    }
    else {
        m_popupPalette->showPopupPalette(p);
    }
}

void KisFavoriteResourceManager::resetPopupPaletteParent(QWidget* w)
{
    if (m_popupPalette) {
        m_popupPalette->setParent(w);
    }
}

QList<QImage> KisFavoriteResourceManager::favoritePresetImages()
{
    QList<QImage> images;
    KoResourceServer<KisPaintOpPreset>* rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    foreach(const QString& name, m_favoritePresetsList) {
        KoResource* resource = rServer->resourceByName(name);
        if(!resource) {
            removeFavoritePreset(name);
        }
        else {
            images.append(resource->image());
        }
    }
    return images;
}

void KisFavoriteResourceManager::slotChangeActivePaintop(int pos)
{
    if (pos < 0 || pos >= m_favoritePresetsList.size()) return;

    KoResourceServer<KisPaintOpPreset>* rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    KoResource* resource = rServer->resourceByName(m_favoritePresetsList.at(pos));
    m_paintopBox->resourceSelected(resource);

    if (m_popupPalette) {
        m_popupPalette->showPopupPalette(false); //automatically close the palette after a button is clicked.
    }
}


//Palette Manager
void KisFavoriteResourceManager::showPaletteManager()
{
    if (!m_favoriteBrushManager) {
        m_favoriteBrushManager = new KisPaletteManager(this);
    }
    m_favoriteBrushManager->show();

}

//Favorite Brushes
int KisFavoriteResourceManager::addFavoritePreset(const QString& name)
{

    int pos = isFavoriteBrushSaved(name);

    if (pos > -1) //brush is saved
    {
        return pos;
    }
    else //brush hasn't been saved yet
    {
        if (isFavoritePresetsFull())
        {
            return -2; //list is full!
        }
        else
        {
            m_favoritePresetsList.append(name);
            saveFavoritePresets();
            m_popupPalette->update();
            return -1;
        }
    }
}

int KisFavoriteResourceManager::isFavoriteBrushSaved(const QString& name)
{
    return m_favoritePresetsList.indexOf(name);
}

void KisFavoriteResourceManager::removeFavoritePreset(int pos)
{
    if (pos < 0 || pos > m_favoritePresetsList.size())
    {
        return;
    }
    else {
        m_favoritePresetsList.removeAt(pos);
        saveFavoritePresets();
        m_popupPalette->update();
    }
}

void KisFavoriteResourceManager::removeFavoritePreset(const QString& name)
{
    int pos = isFavoriteBrushSaved(name);
    if (pos > -1) removeFavoritePreset(pos);
}

bool KisFavoriteResourceManager::isFavoritePresetsFull()
{
    return m_favoritePresetsList.size() == KisFavoriteResourceManager::MAX_FAVORITE_PRESETS;
}

int KisFavoriteResourceManager::numFavoritePresets()
{
    return m_favoritePresetsList.size();
}

void KisFavoriteResourceManager::saveFavoritePresets()
{

    QString favoriteList;

    for (int pos = 0; pos < m_favoritePresetsList.size(); pos++)
    {
        favoriteList.append(m_favoritePresetsList.at(pos));
        favoriteList.append(",");
    }

    KConfigGroup group(KGlobal::config(), "favoriteList");
    group.writeEntry("favoritePresets", favoriteList);
    group.config()->sync();
}

//Recent Colors
void KisFavoriteResourceManager::slotUpdateRecentColor(int pos)
{
    //    qDebug() << "[KisFavoriteResourceManager] selected color: " << recentColorAt(pos)
    //            << "(r)" << recentColorAt(pos).red() << "(g)" << recentColorAt(pos).green()
    //            << "(b)" << recentColorAt(pos).blue();

    addRecentColorUpdate(pos);

    if (m_popupPalette)
        m_popupPalette->showPopupPalette(false); //automatically close the palette after a button is clicked.
}

void KisFavoriteResourceManager::slotAddRecentColor(const KoColor& color)
{
    addRecentColor(color);
}

void KisFavoriteResourceManager::slotChangeFGColorSelector(KoColor c)
{
    QColor color;
    color = c.toQColor();

    //qDebug() << "[KisFavoriteResourceManager] slotChangeFGColorSelector | color " << color ;
    emit sigChangeFGColorSelector(color);
}

void KisFavoriteResourceManager::addRecentColorUpdate(int guipos)
{
    // Do not update the key, the colour might be selected but it is not used yet. So we are not supposed
    // to update the colour priority when we select it.
    m_colorList->updateKey(guipos);
    if (m_popupPalette)
    {
        m_popupPalette->setSelectedColor(guipos);
        m_popupPalette->update();
    }

    emit sigSetFGColor(m_colorList->guiColor(guipos));
    printColors();
}

void KisFavoriteResourceManager::addRecentColor(const KoColor& color)
{
    m_colorList->append(color);
    int pos = m_colorList->findPos(color);
    if (m_popupPalette)
    {
        m_popupPalette->setSelectedColor(pos);
        m_popupPalette->update();
    }

    printColors();
}

void KisFavoriteResourceManager::removingResource(KisPaintOpPreset* resource)
{
    if(m_blockUpdates) {
        return;
    }
    removeFavoritePreset(resource->name());
}

void KisFavoriteResourceManager::resourceAdded(KisPaintOpPreset* /*resource*/)
{
}

void KisFavoriteResourceManager::resourceChanged(KisPaintOpPreset* /*resource*/)
{
}

void KisFavoriteResourceManager::setBlockUpdates(bool block)
{
    m_blockUpdates = block;
}

void KisFavoriteResourceManager::syncTaggedResourceView(){}

void KisFavoriteResourceManager::syncTagAddition(const QString& /*tag*/){}

void KisFavoriteResourceManager::syncTagRemoval(const QString& /*tag*/){}

#include "kis_favorite_resource_manager.moc"
