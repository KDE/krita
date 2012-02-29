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
#include "ko_favorite_resource_manager.h"
#include "kis_popup_palette.h"
#include "kis_paintop_box.h"
#include "kis_palette_manager.h"
#include "kis_view2.h"
#include "kis_resource_server_provider.h"

#ifndef _MSC_EXTENSIONS
const int KoFavoriteResourceManager::MAX_FAVORITE_PRESETS;
//const int KoFavoriteResourceManager::MAX_RECENT_COLORS;
#endif

KoFavoriteResourceManager::KoFavoriteResourceManager(KisPaintopBox *paintopBox, QWidget* popupParent)
        :m_favoriteBrushManager(0)
        ,m_popupPalette(0)
        ,m_paintopBox(paintopBox)
        ,m_colorList(0)
        ,m_blockUpdates(false)
{

    //connect(paintopBox, SIGNAL(signalPaintopChanged(KisPaintOpPresetSP)), this, SLOT(slotChangePaintopLabel(KisPaintOpPresetSP)));

    //take favorite brushes from a file then append to QList
    KConfigGroup group(KGlobal::config(), "favoriteList");
    m_favoritePresetsList = (group.readEntry("favoritePresets")).split(',', QString::SkipEmptyParts);

    m_popupPalette = new KisPopupPalette(this, popupParent);
    m_popupPalette->showPopupPalette(false);
    m_colorList = new KisColorDataList();

    KoResourceServer<KisPaintOpPreset>* rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    rServer->addObserver(this);
}

KoFavoriteResourceManager::~KoFavoriteResourceManager()
{
    KoResourceServer<KisPaintOpPreset>* rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    rServer->removeObserver(this);
    delete m_favoriteBrushManager;
    delete m_colorList;
}

QStringList KoFavoriteResourceManager::favoritePresetList()
{
    return m_favoritePresetsList;
}

//Popup Palette
void KoFavoriteResourceManager::slotShowPopupPalette(const QPoint &p)
{
    if (!m_popupPalette) return;
    else m_popupPalette->showPopupPalette(p);
}

void KoFavoriteResourceManager::resetPopupPaletteParent(QWidget* w)
{
    if (m_popupPalette)
    {
        m_popupPalette->setParent(w);
    }
}

QList<QImage> KoFavoriteResourceManager::favoritePresetImages()
{
    QList<QImage> images;
    KoResourceServer<KisPaintOpPreset>* rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    foreach(const QString& name, m_favoritePresetsList) {
        KoResource* resource = rServer->getResourceByName(name);
        if(!resource) {
            removeFavoritePreset(name);
        }
        else {
            images.append(resource->image());
        }
    }
    return images;
}

void KoFavoriteResourceManager::slotChangeActivePaintop(int pos)
{
    if (pos < 0 || pos >= m_favoritePresetsList.size()) return;

    KoResourceServer<KisPaintOpPreset>* rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    KoResource* resource = rServer->getResourceByName(m_favoritePresetsList.at(pos));
    m_paintopBox->resourceSelected(resource);

    if (m_popupPalette)
    {
        m_popupPalette->showPopupPalette(false); //automatically close the palette after a button is clicked.
    }
}

bool KoFavoriteResourceManager::isPopupPaletteVisible()
{
    if (!m_popupPalette) return false;
    else return m_popupPalette->isVisible();
}

//Palette Manager
void KoFavoriteResourceManager::showPaletteManager()
{

    if (!m_favoriteBrushManager)
    {
        m_favoriteBrushManager = new KisPaletteManager (this, m_paintopBox);
    }
    m_favoriteBrushManager->show();

}

//Favorite Brushes
int KoFavoriteResourceManager::addFavoritePreset(const QString& name)
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

int KoFavoriteResourceManager::isFavoriteBrushSaved(const QString& name)
{
    return m_favoritePresetsList.indexOf(name);
}

void KoFavoriteResourceManager::removeFavoritePreset(int pos)
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

void KoFavoriteResourceManager::removeFavoritePreset(const QString& name)
{
    int pos = isFavoriteBrushSaved(name);
    if (pos > -1) removeFavoritePreset(pos);
}

bool KoFavoriteResourceManager::isFavoritePresetsFull()
{
    return m_favoritePresetsList.size() == KoFavoriteResourceManager::MAX_FAVORITE_PRESETS;
}

int KoFavoriteResourceManager::favoritePresetsTotal()
{
    return m_favoritePresetsList.size();
}

void KoFavoriteResourceManager::saveFavoritePresets()
{

    QString favoriteList = "";

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
void KoFavoriteResourceManager::slotUpdateRecentColor(int pos)
{
//    qDebug() << "[KoFavoriteResourceManager] selected color: " << recentColorAt(pos)
//            << "(r)" << recentColorAt(pos).red() << "(g)" << recentColorAt(pos).green()
//            << "(b)" << recentColorAt(pos).blue();

    addRecentColorUpdate(pos);

    if (m_popupPalette)
        m_popupPalette->showPopupPalette(false); //automatically close the palette after a button is clicked.
}

void KoFavoriteResourceManager::slotAddRecentColor(const KoColor& color)
{
    addRecentColor(color);
}

void KoFavoriteResourceManager::slotChangeFGColorSelector(KoColor c)
{
    QColor color;
    color = c.toQColor();

    //qDebug() << "[KoFavoriteResourceManager] slotChangeFGColorSelector | color " << color ;
    emit sigChangeFGColorSelector(color);
}

void KoFavoriteResourceManager::addRecentColorUpdate(int guipos)
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

void KoFavoriteResourceManager::addRecentColor(const KoColor& color)
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

void KoFavoriteResourceManager::removingResource(KisPaintOpPreset* resource)
{
    if(m_blockUpdates) {
        return;
    }
    removeFavoritePreset(resource->name());
}

void KoFavoriteResourceManager::resourceAdded(KisPaintOpPreset* /*resource*/)
{
}

void KoFavoriteResourceManager::resourceChanged(KisPaintOpPreset* /*resource*/)
{
}

void KoFavoriteResourceManager::setBlockUpdates(bool block)
{
    m_blockUpdates = block;
}

#include "ko_favorite_resource_manager.moc"
