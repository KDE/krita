/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2009 Vera Lukman <shicmap@gmail.com>
   SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
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
        Q_ASSERT_X(pos < size(), "ColorDataList::guiColor", "index out of bounds");
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
        Q_ASSERT_X(size() >= 0, "ColorDataList::removeLeastUsed", "index out of bounds");
        if (size() <= 0) return;

        int pos = findPos(m_priorityList.valueAt(0));
        m_guiList.removeAt(pos);
        m_priorityList.remove(0);
    }

    void clearHistory() {
        Q_ASSERT_X(size() >= 0, "ColorDataList::clearHistory", "index out of bounds");
        if (size() <= 0 ) return;
        while (size() > 0){
            removeLeastUsed();
        }
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
QVector<QString> KisFavoriteResourceManager::favoritePresetNamesList()
{
    init();

    QVector<QString> names;
    for (int i = 0; i < m_maxPresets; i++) {
        QModelIndex index = m_resourcesProxyModel->index(i, 0);
        if (index.isValid()) {
            QString name = m_resourcesProxyModel->data(index, Qt::UserRole + KisAbstractResourceModel::Name).toString();
            names << name;
        }  else {
            break; // no more valid indices
        }
    }

    return names;
}

QList<QImage> KisFavoriteResourceManager::favoritePresetImages()
{
    init();
    QList<QImage> images;
    for (int i = 0; i < m_maxPresets; i++) {
        QModelIndex index = m_resourcesProxyModel->index(i, 0);
        if (index.isValid()) {
            QVariant tmp = m_resourcesProxyModel->data(index, Qt::UserRole + KisAbstractResourceModel::Thumbnail);
            QImage image = tmp.value<QImage>();
            images << image;
        } else {
            break; // no more valid indices
        }
    }
    return images;
}

void KisFavoriteResourceManager::setCurrentTag(const KisTagSP tag)
{
    m_currentTag = tag;
    m_resourcesProxyModel->setTagFilter(tag);
    KisConfig(false).writeEntry<QString>("favoritePresetsTag", tag->url());
    updateFavoritePresets();
}

void KisFavoriteResourceManager::slotChangeActivePaintop(int pos)
{
    ENTER_FUNCTION() << ppVar(pos) << ppVar(numFavoritePresets());
    if (pos < 0 || pos >= numFavoritePresets()) return;

    QModelIndex index = m_resourcesProxyModel->index(pos, 0);
    KoResourceSP resource = m_resourcesProxyModel->resourceForIndex(index);

    m_paintopBox->resourceSelected(resource);

    emit hidePalettes();
}

int KisFavoriteResourceManager::numFavoritePresets()
{
    init();
    return favoritePresetNamesList().size();
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

void KisFavoriteResourceManager::removingResource(QSharedPointer<KisPaintOpPreset> /*resource*/)
{
    updateFavoritePresets();
}

void KisFavoriteResourceManager::resourceAdded(QSharedPointer<KisPaintOpPreset>  /*resource*/)
{
    updateFavoritePresets();
}

void KisFavoriteResourceManager::resourceChanged(QSharedPointer<KisPaintOpPreset>  /*resource*/)
{
    updateFavoritePresets();
}

int KisFavoriteResourceManager::recentColorsTotal()
{
    return m_colorList->size();
}

void KisFavoriteResourceManager::slotClearHistory()
{
    m_colorList->clearHistory();
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

        m_tagModel = new KisTagModel(ResourceType::PaintOpPresets, this);
        m_resourcesProxyModel = new KisTagFilterResourceProxyModel(ResourceType::PaintOpPresets, this);

        m_resourceModel = new KisResourceModel(ResourceType::PaintOpPresets, this);

        KisResourceServerProvider::instance()->paintOpPresetServer();
        QString currentTag = KisConfig(true).readEntry<QString>("favoritePresetsTag", "★ My Favorites");

        // TODO: RESOURCES: tag by url?
        KisTagModel tagModel(ResourceType::PaintOpPresets);
        for (int i = 0; i < tagModel.rowCount(); i++) {
            QModelIndex index = tagModel.index(i, 0);
            KisTagSP tag = tagModel.tagForIndex(index);
            if (!tag.isNull() && tag->url() == currentTag) {
                 m_currentTag = tag;
                 break;
            }
        }
        m_resourcesProxyModel->setTagFilter(m_currentTag);

        updateFavoritePresets();
    }
}


