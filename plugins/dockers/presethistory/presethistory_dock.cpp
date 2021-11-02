/*
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "presethistory_dock.h"

#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QHBoxLayout>
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

PresetHistoryList::PresetHistoryList(QWidget *parent)
    : QListWidget(parent)
{}

void PresetHistoryList::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        emit mouseReleased(currentItem());
    }
    QListWidget::mouseReleaseEvent(e); // forward the event to the QListWidget
}

PresetHistoryDock::PresetHistoryDock( )
    : QDockWidget(i18n("Brush Preset History"))
{
    m_presetHistory = new PresetHistoryList(this);
    m_presetHistory->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_presetHistory->setDragEnabled(false);
    m_presetHistory->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_presetHistory->setSelectionMode(QAbstractItemView::SingleSelection);
    m_presetHistory->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_presetHistory->setContextMenuPolicy(Qt::CustomContextMenu);
    setWidget(m_presetHistory);

    m_sortingModes = new QActionGroup(this);
    m_actionSortStatic = new QAction(i18n("Static Positions"), m_sortingModes);
    m_actionSortStatic->setCheckable(true);
    m_actionSortMostRecent = new QAction(i18n("Move to Top on Use"), m_sortingModes);
    m_actionSortMostRecent->setCheckable(true);
    m_actionSortBubble = new QAction(i18n("Bubble Up on Repeated Use"), m_sortingModes);
    m_actionSortBubble->setCheckable(true);

    QScroller* scroller = KisKineticScroller::createPreconfiguredScroller(m_presetHistory);
    if( scroller ) {
        connect(scroller, SIGNAL(stateChanged(QScroller::State)), this, SLOT(slotScrollerStateChanged(QScroller::State)));
    }

    connect(m_presetHistory, SIGNAL(mouseReleased(QListWidgetItem*)), SLOT(presetSelected(QListWidgetItem*)));
    connect(m_sortingModes, SIGNAL(triggered(QAction*)), SLOT(slotSortingModeChanged(QAction*)));
    connect(m_presetHistory, SIGNAL(customContextMenuRequested(QPoint)), SLOT(slotContextMenuRequest(QPoint)));
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
            KisPaintOpPresetSP preset = rserver->resource("", "", p);
            addPreset(preset);
        }
        int ordering = cfg.readEntry("presethistorySorting", int(DisplayOrder::Static));
        m_sorting = qBound(DisplayOrder::Static, static_cast<DisplayOrder>(ordering), DisplayOrder::Bubbling);

        switch (m_sorting) {
        case DisplayOrder::Static:
            m_actionSortStatic->setChecked(true);
            break;
        case DisplayOrder::MostRecent:
            m_actionSortMostRecent->setChecked(true);
            break;
        case DisplayOrder::Bubbling:
            m_actionSortBubble->setChecked(true);
        }

        QVariant currentPreset = m_canvas->resourceManager()->resource(KoCanvasResource::CurrentPaintOpPreset);
        canvasResourceChanged(KoCanvasResource::CurrentPaintOpPreset, currentPreset);
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
        updatePresetState(oldPosition);
        QVariant v = item->data(BrushPresetRole);
        KisPaintOpPresetSP preset = v.value<KisPaintOpPresetSP>();
        m_block = true;
        m_canvas->viewManager()->paintOpBox()->resourceSelected(preset);
        m_block = false;
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
                    updatePresetState(i);
                    return;
                }
            }
            addPreset(preset);
        }
    }
}

void PresetHistoryDock::slotSortingModeChanged(QAction *action)
{
    if (action == m_actionSortStatic) {
        m_sorting = DisplayOrder::Static;
    } else if (action == m_actionSortMostRecent) {
        m_sorting = DisplayOrder::MostRecent;
    } else if (action == m_actionSortBubble) {
        m_sorting = DisplayOrder::Bubbling;
    }
    KisConfig cfg(false);
    cfg.writeEntry("presethistorySorting", int(m_sorting));
}

void PresetHistoryDock::updatePresetState(int position)
{
    switch (m_sorting) {
    case Static:
        m_presetHistory->setCurrentRow(position);
        break;
    case MostRecent:
        m_presetHistory->insertItem(0, m_presetHistory->takeItem(position));
        m_presetHistory->setCurrentRow(0);
        break;
    case Bubbling:
        position = bubblePreset(position);
        m_presetHistory->setCurrentRow(position);
    };
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

void PresetHistoryDock::slotContextMenuRequest(const QPoint &pos)
{
    QMenu contextMenu;
    QListWidgetItem *presetItem = m_presetHistory->itemAt(pos);
    QAction *actionForget = 0;
    if (presetItem) {
        actionForget = new QAction(i18n("Forget \"%1\"", presetItem->text()), &contextMenu);
        contextMenu.addAction(actionForget);
    }
    contextMenu.addAction(i18n("Clear History"), m_presetHistory, SLOT(clear()));
    contextMenu.addSeparator();
    contextMenu.addSection(i18n("History Behavior:"));
    contextMenu.addAction(m_actionSortStatic);
    contextMenu.addAction(m_actionSortMostRecent);
    contextMenu.addAction(m_actionSortBubble);
    QAction *triggered = contextMenu.exec(m_presetHistory->mapToGlobal(pos));

    if (presetItem && triggered == actionForget) {
        // deleting a QListWidgetItem removes it from the QListWidget automatically
        delete presetItem;
    }
}
