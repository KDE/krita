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
#include "kis_min_heap.h"

#ifndef _MSC_EXTENSIONS
const int KisFavoriteResourceManager::MAX_FAVORITE_PRESETS;
//const int KisFavoriteResourceManager::MAX_RECENT_COLORS;
#endif

class KisFavoriteResourceManager::ColorDataList
{
public:
    static const int MAX_RECENT_COLOR = 12;

    ColorDataList() {
        m_key = 0;
    }

    ~ColorDataList() {
    }

    int size() {
        return m_guiList.size();
    }

    int leastUsedGuiPos() {
        return findPos(m_priorityList.valueAt(0));
    }

    const KoColor& guiColor(int pos) {
        Q_ASSERT_X(pos < size(), "ColorDataList::guiColor", "index out of bound");
        Q_ASSERT_X(pos >= 0, "ColorDataList::guiColor", "negative index");

        return m_guiList.at(pos)->data;
    }

    void append(const KoColor& data) {
        int pos = findPos(data);
        if (pos > -1) updateKey(pos);
        else appendNew(data);
    }

    void appendNew(const KoColor& data) {
        if (size() >= ColorDataList::MAX_RECENT_COLOR) removeLeastUsed();

        PriorityNode<KoColor> * node;
        node = new PriorityNode <KoColor>();
        node->data = data;
        node->key = m_key++;
        m_priorityList.append(node);

        int pos = guiInsertPos(data);
        pos >= m_guiList.size() ? m_guiList.append(node)
        : m_guiList.insert(pos, node);
        node = 0;
    }

    void removeLeastUsed() {
        Q_ASSERT_X(size() >= 0, "ColorDataList::removeLeastUsed", "index out of bound");
        if (size() <= 0) return;

        int pos = findPos(m_priorityList.valueAt(0));
        m_guiList.removeAt(pos);
        m_priorityList.remove(0);
    }

    void updateKey(int guiPos) {
        if (m_guiList.at(guiPos)->key == m_key - 1) return;
        m_priorityList.changeKey(m_guiList.at(guiPos)->pos, m_key++);
    }

    /*find position of the color on the gui list*/
    int findPos(const KoColor& color) {

        int low = 0, high = size(), mid = 0;
        while (low < high) {
            mid = (low + high) / 2;
            if (hsvComparison(color, m_guiList.at(mid)->data) == 0) return mid;
            else if (hsvComparison(color, m_guiList.at(mid)->data) < 0) high = mid;
            else low = mid + 1;
        }

        return -1;
    }


private:

    int m_key;

    int guiInsertPos(const KoColor& color) {
        int low = 0, high = size() - 1, mid = (low + high) / 2;
        while (low < high) {

            hsvComparison(color, m_guiList[mid]->data) == -1 ? high = mid
                    : low = mid + 1;
            mid = (low + high) / 2;
        }

        if (m_guiList.size() > 0) {
            if (hsvComparison(color, m_guiList[mid]->data) == 1) ++mid;
        }
        return mid;
    }

    /*compares c1 and c2 based on HSV.
      c1 < c2, returns -1
      c1 = c2, returns 0
      c1 > c2, returns 1 */
    int hsvComparison(const KoColor& c1, const KoColor& c2) {
        QColor qc1 = c1.toQColor();
        QColor qc2 = c2.toQColor();

        if (qc1.hue() < qc2.hue()) return -1;
        if (qc1.hue() > qc2.hue()) return 1;

        // hue is the same, ok let's compare saturation
        if (qc1.saturation() < qc2.saturation()) return -1;
        if (qc1.saturation() > qc2.saturation()) return 1;

        // oh, also saturation is same?
        if (qc1.value() < qc2.value()) return -1;
        if (qc1.value() > qc2.value()) return 1;

        // user selected two similar colors
        return 0;
    }

    KisMinHeap <KoColor, MAX_RECENT_COLOR> m_priorityList;
    QList <PriorityNode <KoColor>*> m_guiList;
};



KisFavoriteResourceManager::KisFavoriteResourceManager(KisPaintopBox *paintopBox)
    : m_favoriteBrushManager(0)
    , m_paintopBox(paintopBox)
    , m_colorList(0)
    , m_blockUpdates(false)
{
    //take favorite brushes from a file then append to QList
    KConfigGroup group(KGlobal::config(), "favoriteList");
    m_favoritePresetsList = (group.readEntry("favoritePresets")).split(',', QString::SkipEmptyParts);

    m_colorList = new ColorDataList();

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

void KisFavoriteResourceManager::unsetResourceServer()
{
    // ...
}

QStringList KisFavoriteResourceManager::favoritePresetList()
{
    return m_favoritePresetsList;
}

QList<QImage> KisFavoriteResourceManager::favoritePresetImages()
{
    QList<QImage> images;
    KoResourceServer<KisPaintOpPreset>* rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    foreach(const QString & name, m_favoritePresetsList) {
        KoResource* resource = rServer->resourceByName(name);
        if (!resource) {
            removeFavoritePreset(name);
        } else {
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
    emit hidePalettes();
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

    if (pos > -1) { //brush is saved
        return pos;
    }

    else { //brush hasn't been saved yet

        if (m_favoritePresetsList.size() == KisFavoriteResourceManager::MAX_FAVORITE_PRESETS)  {
            return -2; //list is full!
        } else {
            m_favoritePresetsList.append(name);
            saveFavoritePresets();
            emit updatePalettes();
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
    if (pos < 0 || pos > m_favoritePresetsList.size()) {
        return;
    } else {
        m_favoritePresetsList.removeAt(pos);
        saveFavoritePresets();
        emit updatePalettes();
    }
}

void KisFavoriteResourceManager::removeFavoritePreset(const QString& name)
{
    int pos = isFavoriteBrushSaved(name);
    if (pos > -1) removeFavoritePreset(pos);
}

int KisFavoriteResourceManager::numFavoritePresets()
{
    return m_favoritePresetsList.size();
}

void KisFavoriteResourceManager::saveFavoritePresets()
{

    QString favoriteList;

    for (int pos = 0; pos < m_favoritePresetsList.size(); pos++)  {
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
    // Do not update the key, the colour might be selected but it is not used yet. So we are not supposed
    // to update the colour priority when we select it.
    m_colorList->updateKey(pos);

    emit setSelectedColor(pos);
    emit sigSetFGColor(m_colorList->guiColor(pos));
    emit hidePalettes();
}

void KisFavoriteResourceManager::slotAddRecentColor(const KoColor& color)
{
    m_colorList->append(color);
    int pos = m_colorList->findPos(color);
    emit setSelectedColor(pos);
}

void KisFavoriteResourceManager::slotChangeFGColorSelector(KoColor c)
{
    QColor color;
    color = c.toQColor();
    emit sigChangeFGColorSelector(color);
}

void KisFavoriteResourceManager::removingResource(KisPaintOpPreset* resource)
{
    if (m_blockUpdates) {
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

void KisFavoriteResourceManager::syncTaggedResourceView() {}

void KisFavoriteResourceManager::syncTagAddition(const QString& /*tag*/) {}

void KisFavoriteResourceManager::syncTagRemoval(const QString& /*tag*/) {}

int KisFavoriteResourceManager::recentColorsTotal()
{
    return m_colorList->size();
}

const KoColor& KisFavoriteResourceManager::recentColorAt(int pos)
{
    return m_colorList->guiColor(pos);
}

void KisFavoriteResourceManager::slotSetBGColor(const KoColor c)
{
    m_bgColor = c.toQColor();
}

QColor KisFavoriteResourceManager::bgColor()
{
    return m_bgColor;
}

#include "kis_favorite_resource_manager.moc"

