/*
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _PRESETHISTORY_DOCK_H_
#define _PRESETHISTORY_DOCK_H_

#include <QDockWidget>
#include <QPointer>
#include <KisKineticScroller.h>

#include <KoCanvasObserverBase.h>

#include <kis_canvas2.h>
#include <kis_types.h>

class QListWidget;
class QListWidgetItem;
class QActionGroup;

class PresetHistoryDock : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    enum HistoryDataRole {
        BrushPresetRole = Qt::UserRole,
        BubbleMarkerRole = Qt::UserRole + 1
    };

    enum DisplayOrder {
        Static = 0,
        MostRecent = 1,
        Bubbling = 2
    };

    PresetHistoryDock();
    QString observerName() override { return "PresetHistoryDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
public Q_SLOTS:
    void slotScrollerStateChanged(QScroller::State state){ KisKineticScroller::updateCursor(this, state); }
private Q_SLOTS:
    void presetSelected(QListWidgetItem* item);
    void canvasResourceChanged(int key, const QVariant& v);
    void slotSortingModeChanged(QAction *action);
    void slotContextMenuRequest(const QPoint &pos);
private:
    void updatePresetState(int position);
    int bubblePreset(int position);
    void addPreset(KisPaintOpPresetSP preset);
private:
    QPointer<KisCanvas2> m_canvas;
    QListWidget *m_presetHistory;
    QAction *m_actionSortStatic;
    QAction *m_actionSortMostRecent;
    QAction *m_actionSortBubble;
    QActionGroup *m_sortingModes;
    DisplayOrder m_sorting {Static};
    bool m_block {false};
    bool m_initialized {false};
};


#endif
