/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __TIMELINE_FRAMES_VIEW_H
#define __TIMELINE_FRAMES_VIEW_H


#include <QScopedPointer>
#include <QTableView>
#include <QScroller>
#include "kis_action_manager.h"
#include "kritaanimationdocker_export.h"

class KisAction;
class TimelineWidget;

enum TimelineDirection : short
{
    LEFT = -1,
    BEFORE = -1,

    RIGHT = 1,
    AFTER = 1
};


class KRITAANIMATIONDOCKER_EXPORT TimelineFramesView : public QTableView
{
    Q_OBJECT

public:
    TimelineFramesView(QWidget *parent);
    ~TimelineFramesView() override;

    void setModel(QAbstractItemModel *model) override;

    void updateGeometries() override;

    void setShowInTimeline(KisAction *action);

    void setActionManager(KisActionManager *actionManager);

public Q_SLOTS:
    void slotSelectionChanged();
    void slotUpdateIcons();

private Q_SLOTS:
    void slotUpdateLayersMenu();
    void slotUpdateFrameActions();

    void slotSetStartTimeToCurrentPosition();
    void slotSetEndTimeToCurrentPosition();
    void slotUpdatePlackbackRange();

    // Layer
    void slotAddNewLayer();
    void slotAddExistingLayer(QAction *action);
    void slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void slotRemoveLayer();
    void slotLayerContextMenuRequested(const QPoint &globalPos);

    // New, Insert and Remove Frames
    void slotAddBlankFrame();
    void slotAddDuplicateFrame();

    void slotInsertKeyframeLeft() {insertKeyframes(-1, 1, TimelineDirection::LEFT, false);}
    void slotInsertKeyframeRight() {insertKeyframes(-1, 1, TimelineDirection::RIGHT, false);}

    void slotInsertKeyframeColumnLeft() {insertKeyframes(-1, 1, TimelineDirection::LEFT, true);}
    void slotInsertKeyframeColumnRight() {insertKeyframes(-1, 1, TimelineDirection::RIGHT, true);}

    void slotInsertMultipleKeyframes() {insertMultipleKeyframes(false);}
    void slotInsertMultipleKeyframeColumns() {insertMultipleKeyframes(true);}

    void slotRemoveSelectedFrames(bool entireColumn = false, bool pull = false);
    void slotRemoveSelectedFramesAndShift() {slotRemoveSelectedFrames(false, true);}

    void slotRemoveSelectedColumns() {slotRemoveSelectedFrames(true);}
    void slotRemoveSelectedColumnsAndShift() {slotRemoveSelectedFrames(true, true);}

    void slotInsertHoldFrame() {insertOrRemoveHoldFrames(1);}
    void slotRemoveHoldFrame() {insertOrRemoveHoldFrames(-1);}

    void slotInsertHoldFrameColumn() {insertOrRemoveHoldFrames(1,true);}
    void slotRemoveHoldFrameColumn() {insertOrRemoveHoldFrames(-1,true);}

    void slotInsertMultipleHoldFrames() {insertOrRemoveMultipleHoldFrames(true);}
    void slotRemoveMultipleHoldFrames() {insertOrRemoveMultipleHoldFrames(false);}

    void slotInsertMultipleHoldFrameColumns() {insertOrRemoveMultipleHoldFrames(true, true);}
    void slotRemoveMultipleHoldFrameColumns() {insertOrRemoveMultipleHoldFrames(false, true);}

    void slotMirrorFrames(bool entireColumn = false);
    void slotMirrorColumns() {slotMirrorFrames(true);}

    // Copy-paste
    void slotCopyFrames() {cutCopyImpl(false, true);}
    void slotCutFrames() {cutCopyImpl(false, false);}

    void slotCopyColumns() {cutCopyImpl(true, true);}
    void slotCutColumns() {cutCopyImpl(true, false);}

    void slotPasteFrames(bool entireColumn = false);
    void slotPasteColumns() {slotPasteFrames(true);}

    void slotReselectCurrentIndex();

    void slotUpdateInfiniteFramesCount();

    void slotHeaderDataChanged(Qt::Orientation orientation, int first, int last);

    void slotZoomButtonPressed(qreal staticPoint);
    void slotZoomButtonChanged(qreal value);

    void slotColorLabelChanged(int);
    void slotEnsureRowVisible(int row);

    // Audio
    void slotSelectAudioChannelFile();
    void slotAudioChannelMute(bool value);
    void slotAudioChannelRemove();
    void slotUpdateAudioActions();
    void slotAudioVolumeChanged(int value);

    // DragScroll
    void slotScrollerStateChanged(QScroller::State state);

private:
    void setFramesPerSecond(int fps);

    void calculateSelectionMetrics(int &minColumn, int &maxColumn, QSet<int> &rows) const;

    /*   Insert new keyframes/columns.
     *
     *   count        - Number of frames to add. If <0, use number of currently SELECTED frames.
     *   timing       - Animation timing of frames to be added (on 1s, 2s, 3s, etc.)
     *   direction    - Insert frames before (left) or after (right) selection scrubber.
     *   entireColumn - Create frames on all layers (rows) instead of just the active layer?
     */
    void insertKeyframes(int count = 1, int timing = 1,
                         TimelineDirection direction = TimelineDirection::LEFT, bool entireColumn = false);
    void insertMultipleKeyframes(bool entireColumn = false);

    void insertOrRemoveHoldFrames(int count, bool entireColumn = false);
    void insertOrRemoveMultipleHoldFrames(bool insertion, bool entireColumn = false);

    void cutCopyImpl(bool entireColumn, bool copy);

    void createFrameEditingMenuActions(QMenu *menu, bool addFrameCreationActions);

    QModelIndexList calculateSelectionSpan(bool entireColumn, bool editableOnly = true) const;

protected:
    QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &index,
                                                         const QEvent *event) const override;

    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
    void startDrag(Qt::DropActions supportedActions) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void rowsInserted(const QModelIndex &parent, int start, int end) override;
    bool viewportEvent(QEvent *event) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __TIMELINE_FRAMES_VIEW_H */
