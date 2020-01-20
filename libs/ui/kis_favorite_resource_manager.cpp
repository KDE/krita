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

#include <kis_debug.h>
#include <QPoint>
#include <QStringList>
#include <QString>
#include <QColor>
#include <brushengine/kis_paintop_registry.h>
#include <brushengine/kis_paintop_preset.h>
#include <KoID.h>
#include <kconfig.h>
#include "kis_favorite_resource_manager.h"
#include "kis_popup_palette.h"
#include "kis_paintop_box.h"
#include "KisViewManager.h"
#include "KisResourceServerProvider.h"
#include "kis_min_heap.h"
#include "kis_config.h"
#include "kis_config_notifier.h"
#include <kis_paintop_preset.h>

class KisFavoriteResourceManager::ColorDataList
{
public:
    static const int MAX_RECENT_COLOR = 12;

    ColorDataList() {
        m_key = 0;
    }

    ~ColorDataList() {
        qDeleteAll(m_guiList);
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
    : m_paintopBox(paintopBox)
    , m_colorList(0)
    , m_blockUpdates(false)
    , m_initialized(false)
{
    KisConfig cfg(true);
    m_maxPresets = cfg.favoritePresets();
    m_colorList = new ColorDataList();
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(configChanged()));
    KisPaintOpPresetResourceServer * rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    rServer->addObserver(this);
}

KisFavoriteResourceManager::~KisFavoriteResourceManager()
{
    KisPaintOpPresetResourceServer *rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    rServer->removeObserver(this);
    delete m_colorList;
}

void KisFavoriteResourceManager::unsetResourceServer()
{
    // ...
}

QVector<KisPaintOpPresetSP>  KisFavoriteResourceManager::favoritePresetList()
{
    init();
    return m_favoritePresetsList;
}

QList<QImage> KisFavoriteResourceManager::favoritePresetImages()
{
    init();
    QList<QImage> images;
    Q_FOREACH (KisPaintOpPresetSP preset, m_favoritePresetsList) {
        if (preset) {
            images.append(preset->image());
        }

    }
    return images;
}

void KisFavoriteResourceManager::setCurrentTag(const KisTagSP tag)
{
    m_currentTag = tag;
    KisConfig(false).writeEntry<QString>("favoritePresetsTag", tag->url());
    updateFavoritePresets();
}

void KisFavoriteResourceManager::slotChangeActivePaintop(int pos)
{
    if (pos < 0 || pos >= m_favoritePresetsList.size()) return;

    KoResourceSP resource = m_favoritePresetsList.at(pos);;
    m_paintopBox->resourceSelected(resource);

    emit hidePalettes();
}

int KisFavoriteResourceManager::numFavoritePresets()
{
    init();
    return m_favoritePresetsList.size();
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
    emit sigChangeFGColorSelector(c);
}

void KisFavoriteResourceManager::removingResource(QSharedPointer<KisPaintOpPreset>  resource)
{
    if (m_blockUpdates) {
        return;
    }
    if (m_favoritePresetsList.contains(resource)) {
        updateFavoritePresets();
    }
}

void KisFavoriteResourceManager::resourceAdded(QSharedPointer<KisPaintOpPreset>  /*resource*/)
{
    if (m_blockUpdates) {
        return;
    }
    updateFavoritePresets();
}

void KisFavoriteResourceManager::resourceChanged(QSharedPointer<KisPaintOpPreset>  /*resource*/)
{
}

void KisFavoriteResourceManager::setBlockUpdates(bool block)
{
    m_blockUpdates = block;
    if (!block) {
        updateFavoritePresets();
    }
}

void KisFavoriteResourceManager::syncTaggedResourceView() {
    if (m_blockUpdates) {
        return;
    }
    updateFavoritePresets();
}

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
    m_bgColor = c;
}

KoColor KisFavoriteResourceManager::bgColor() const
{
    return m_bgColor;
}

bool sortPresetByName(KisPaintOpPresetSP preset1, KisPaintOpPresetSP preset2)
{
     return preset1->name() < preset2->name();
}

void KisFavoriteResourceManager::updateFavoritePresets()
{

    m_favoritePresetsList.clear();
    KisResourceModel* model = KisResourceModelProvider::resourceModel(ResourceType::PaintOpPresets);
    KisPaintOpPresetResourceServer* rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    QVector<KoResourceSP> presetFilenames = rServer->resourcesForTag(m_currentTag);
    for(int i = 0; i < qMin(m_maxPresets, presetFilenames.size()); i++) {
        KoResourceSP resource = presetFilenames[i];
        KisPaintOpPresetSP preset = KisPaintOpPresetSP(dynamic_cast<KisPaintOpPreset*>(resource.data()));
        m_favoritePresetsList.append(preset);
    }

    /*
    for(int i = 0; i < qMin(m_maxPresets, presetFilenames.size()); i++) {
        KisPaintOpPresetSP pr = rServer->resourceByFilename(presetFilenames.at(i));
        m_favoritePresetsList.append(pr);
        std::sort(m_favoritePresetsList.begin(), m_favoritePresetsList.end(), sortPresetByName);
    }
    */
    emit updatePalettes();
}

void KisFavoriteResourceManager::configChanged()
{
    KisConfig cfg(true);
    m_maxPresets = cfg.favoritePresets();
    updateFavoritePresets();
}

void KisFavoriteResourceManager::init()
{
    if (!m_initialized) {
        m_initialized = true;
        KisResourceServerProvider::instance()->paintOpPresetServer();
        QString currentTag = KisConfig(true).readEntry<QString>("favoritePresetsTag", "★ My Favorites");

        // TODO: RESOURCES: tag by url?
        KisTagModel* tagModel = KisTagModelProvider::tagModel(ResourceType::PaintOpPresets);
        for (int i = 0; i < tagModel->rowCount(); i++) {
            QModelIndex index = tagModel->index(i, 0);
            KisTagSP tag = tagModel->tagForIndex(index);
             if (!tag.isNull() && tag->url() == currentTag) {
                 m_currentTag = tag;
                 break;
             }
         }

        updateFavoritePresets();
    }
}


