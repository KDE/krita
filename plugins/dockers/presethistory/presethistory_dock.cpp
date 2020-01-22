/*
 *  Copyright (c) 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "presethistory_dock.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QImage>

#include <klocalizedstring.h>

#include <KoCanvasResourceProvider.h>
#include <KoCanvasBase.h>

#include "kis_config.h"
#include "kis_canvas2.h"
#include "KisViewManager.h"
#include "kis_paintop_box.h"
#include "kis_paintop_presets_chooser_popup.h"
#include "kis_canvas_resource_provider.h"
#include "KisResourceServerProvider.h"
#include <KisKineticScroller.h>
#include <brushengine/kis_paintop_preset.h>
#include <kis_types.h>

#define ICON_SIZE 48

PresetHistoryDock::PresetHistoryDock( )
    : QDockWidget(i18n("Brush Preset History"))
    , m_canvas(0)
    , m_block(false)
    , m_initialized(false)
{
    m_presetHistory = new QListWidget(this);
    m_presetHistory->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_presetHistory->setDragEnabled(false);
    m_presetHistory->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_presetHistory->setSelectionMode(QAbstractItemView::SingleSelection);
    m_presetHistory->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setWidget(m_presetHistory);

    QScroller* scroller = KisKineticScroller::createPreconfiguredScroller(m_presetHistory);
    if( scroller ) {
        connect(scroller, SIGNAL(stateChanged(QScroller::State)), this, SLOT(slotScrollerStateChanged(QScroller::State)));
    }

    connect(m_presetHistory, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(presetSelected(QListWidgetItem*)));
}

void PresetHistoryDock::setCanvas(KoCanvasBase * canvas)
{
    setEnabled(canvas != 0);

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        disconnect(m_canvas->resourceManager());
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);

    if (!m_canvas || !m_canvas->viewManager() || !m_canvas->resourceManager()) return;

    connect(m_canvas->resourceManager(), SIGNAL(canvasResourceChanged(int,QVariant)), SLOT(canvasResourceChanged(int,QVariant)));

    if (!m_initialized) {
        KisConfig cfg(true);
        QStringList presetHistory = cfg.readEntry<QString>("presethistory", "").split(",", QString::SkipEmptyParts);
        KisPaintOpPresetResourceServer * rserver = KisResourceServerProvider::instance()->paintOpPresetServer();
        Q_FOREACH (const QString &p, presetHistory) {
            KisPaintOpPresetSP preset = rserver->resourceByName(p);
            addPreset(preset);
        }
        m_initialized = true;
    }
}

void PresetHistoryDock::unsetCanvas()
{
    m_canvas = 0;
    setEnabled(false);
    QStringList presetHistory;
    for(int i = m_presetHistory->count() -1; i >=0; --i) {
        QListWidgetItem *item = m_presetHistory->item(i);
        QVariant v = item->data(BrushPresetRole);
        KisPaintOpPresetSP preset = v.value<KisPaintOpPresetSP>();
        presetHistory << preset->name();
    }
    KisConfig cfg(false);
    cfg.writeEntry("presethistory", presetHistory.join(","));
}

void PresetHistoryDock::presetSelected(QListWidgetItem *item)
{
    if (item) {
        int oldPosition = m_presetHistory->currentRow();
        int newPosition = bubblePreset(oldPosition);
        QVariant v = item->data(BrushPresetRole);
        KisPaintOpPresetSP preset = v.value<KisPaintOpPresetSP>();
        m_block = true;
        m_canvas->viewManager()->paintOpBox()->resourceSelected(preset);
        m_block = false;
        if (oldPosition != newPosition) {
            m_presetHistory->setCurrentRow(newPosition);
        }
    }
}

void PresetHistoryDock::canvasResourceChanged(int key, const QVariant& v)
{
    if (m_block) return;

    if (m_canvas && key == KoCanvasResource::CurrentPaintOpPreset) {
        KisPaintOpPresetSP preset = v.value<KisPaintOpPresetSP>();
        if (preset) {
            for (int i = 0; i < m_presetHistory->count(); ++i) {
                if (preset->name() == m_presetHistory->item(i)->text()) {
                    int newPosition = bubblePreset(i);
                    m_presetHistory->setCurrentRow(newPosition);
                    return;
                }
            }
            addPreset(preset);
        }
    }
}

int PresetHistoryDock::bubblePreset(int position)
{
    QListWidgetItem *item = m_presetHistory->item(position);
    if (position == 0) {
        // topmost item cannot bubble, its bubble state stays until
        // below item tries to bubble, so state can be set unconditionally
        item->setData(BubbleMarkerRole, QVariant(true));
        return position;
    }

    if (!item->data(BubbleMarkerRole).toBool()) {
        // first activation effectively makes the entry rival the list position above
        // (unless that one is already marked to bubble too) but it won't raise in position
        // until it gets activated again so its position is clearly defined as above
        item->setData(BubbleMarkerRole, QVariant(true));
        return position;
    }
    else {
        item->setData(BubbleMarkerRole, QVariant(false));
        int topPosition = position - 1;
        for (; topPosition >= 0; --topPosition) {
            QListWidgetItem *topItem = m_presetHistory->item(topPosition);
            if (topItem->data(BubbleMarkerRole).toBool()) {
                topItem->setData(BubbleMarkerRole, QVariant(false));
            }
            else {
                break;
            }
        }
        // if all above items want to bubble too, nothing happens besides resetting bubble state
        if (topPosition >= 0) {
            // since a group of items may bubble together, the net effect is
            // that the item above this range moves below that group
            QListWidgetItem *topItem = m_presetHistory->takeItem(topPosition);
            m_presetHistory->insertItem(position, topItem);
            return position - 1;
        }
    }
    return position;
}

void PresetHistoryDock::addPreset(KisPaintOpPresetSP preset)
{
    if (preset) {
        QListWidgetItem *item = new QListWidgetItem(QPixmap::fromImage(preset->image()), preset->name());
        QVariant v = QVariant::fromValue<KisPaintOpPresetSP>(preset);
        item->setData(BrushPresetRole, v);
        item->setData(BubbleMarkerRole, QVariant(false));
        m_presetHistory->insertItem(0, item);
        m_presetHistory->setCurrentRow(0);
        if (m_presetHistory->count() > 10) {
            delete m_presetHistory->takeItem(10);
        }
    }

}

