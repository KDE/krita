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
#include "kis_config.h"
#include "kis_config_notifier.h"
#include <kis_paintop_preset.h>
#include <KisSortedHistoryList.h>

const int KisFavoriteResourceManager::MAX_RECENT_COLOR = 12;


KisFavoriteResourceManager::KisFavoriteResourceManager(KisPaintopBox *paintopBox)
    : m_paintopBox(paintopBox)
    , m_colorHistoryList(new KisSortedHistoryList<KoColor>(MAX_RECENT_COLOR))

{
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(configChanged()));
    configChanged();

    KisPaintOpPresetResourceServer * rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    rServer->addObserver(this);
}

KisFavoriteResourceManager::~KisFavoriteResourceManager()
{
    KisPaintOpPresetResourceServer *rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    rServer->removeObserver(this);
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
    //ENTER_FUNCTION() << ppVar(pos) << ppVar(numFavoritePresets());
    if (pos < 0 || pos >= numFavoritePresets()) return;

    QModelIndex index = m_resourcesProxyModel->index(pos, 0);
    KoResourceSP resource = m_resourcesProxyModel->resourceForIndex(index);

    m_paintopBox->resourceSelected(resource);

    Q_EMIT hidePalettes();
}

int KisFavoriteResourceManager::numFavoritePresets()
{
    init();
    return favoritePresetNamesList().size();
}

//Recent Colors
void KisFavoriteResourceManager::slotUpdateRecentColor(int pos)
{
    Q_EMIT setSelectedColor(pos);
    Q_EMIT sigSetFGColor(m_colorHistoryList->at(pos));
    Q_EMIT hidePalettes();
}

void KisFavoriteResourceManager::slotAddRecentColor(const KoColor& color)
{
    int pos = m_colorHistoryList->append(color);
    Q_EMIT setSelectedColor(pos);
}

void KisFavoriteResourceManager::slotChangeFGColorSelector(KoColor c)
{
    Q_EMIT sigChangeFGColorSelector(c);
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
    return m_colorHistoryList->size();
}

void KisFavoriteResourceManager::slotClearHistory()
{
    m_colorHistoryList->clear();
}

KoColor KisFavoriteResourceManager::recentColorAt(int pos)
{
    return m_colorHistoryList->at(pos);
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
    Q_EMIT updatePalettes();
}

void KisFavoriteResourceManager::configChanged()
{
    KisConfig cfg(true);
    m_maxPresets = cfg.favoritePresets();
    updateFavoritePresets();

    using compare_less = KisSortedHistoryList<KoColor>::compare_less;

    compare_less sortingFunc;
    const QString sortingType = cfg.readEntry("popuppalette/colorHistorySorting", QString("hsv"));
    if (sortingType == "hsv") {
        sortingFunc = [] (const KoColor &lhs, const KoColor &rhs) {
            auto makeHsvTuple = [] (const KoColor &color) {
                int h, s, v;
                color.toQColor().getHsv(&h, &s, &v);
                return std::make_tuple(h, s, v);
            };
            return makeHsvTuple(lhs) < makeHsvTuple(rhs);
        };
    }

    m_colorHistoryList->setCompareLess(sortingFunc);
}

void KisFavoriteResourceManager::presetsChanged()
{
    Q_EMIT updatePalettes();
}

void KisFavoriteResourceManager::init()
{
    if (!m_initialized) {
        m_initialized = true;

        m_tagModel = new KisTagModel(ResourceType::PaintOpPresets, this);
        m_resourcesProxyModel = new KisTagFilterResourceProxyModel(ResourceType::PaintOpPresets, this);

        connect(m_resourcesProxyModel, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(presetsChanged()));
        connect(m_resourcesProxyModel, SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(presetsChanged()));
        
        m_resourceModel = new KisResourceModel(ResourceType::PaintOpPresets, this);

        KisResourceServerProvider::instance()->paintOpPresetServer();
        QString currentTag = KisConfig(true).readEntry<QString>("favoritePresetsTag", "★ My Favorites");

        KisTagModel tagModel(ResourceType::PaintOpPresets);
        KisTagSP currentTagSP = tagModel.tagForUrl(currentTag);
        if (!currentTagSP.isNull()) {
            m_currentTag = currentTagSP;
        }
        if (m_currentTag.isNull() && tagModel.rowCount() > 0) {
            // safety measure to have at least *some* tag chosen
            QModelIndex idx = tagModel.index(0, 0);
            currentTagSP = tagModel.tagForIndex(idx);
            if (currentTagSP && !m_currentTag) {
                m_currentTag = currentTagSP;
            }
        }
        m_resourcesProxyModel->setTagFilter(m_currentTag);
        m_resourcesProxyModel->sort(KisAbstractResourceModel::Name);

        updateFavoritePresets();
    }
}


