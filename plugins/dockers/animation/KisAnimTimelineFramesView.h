/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __TIMELINE_FRAMES_VIEW_H
#define __TIMELINE_FRAMES_VIEW_H


#include <QScopedPointer>
#include <QTableView>
#include <QScroller>
#include <QScrollBar>
#include "kis_action_manager.h"
#include "kritaanimationdocker_export.h"

class KisAction;
class TimelineWidget;

enum TimelineDirection : short
{
    LEFT = -1,
    RIGHT = 1,
    BEFORE = LEFT,
    AFTER = RIGHT
};

class KRITAANIMATIONDOCKER_EXPORT KisAnimTimelineFramesView : public QTableView
{
    Q_OBJECT
public:
    KisAnimTimelineFramesView(QWidget *parent);
    ~KisAnimTimelineFramesView() override;

    void setModel(QAbstractItemModel *model) override;
    void setActionManager(KisActionManager *actionManager);

    void updateGeometries() override;

public Q_SLOTS:
    void slotCanvasUpdate(class KoCanvasBase* canvas);

    void slotUpdateIcons();
    void slotUpdateLayersMenu();
    void slotUpdateFrameActions();

    void slotSelectionChanged();
    void slotReselectCurrentIndex();

    void slotSetStartTimeToCurrentPosition();
    void slotSetEndTimeToCurrentPosition();
    void slotUpdatePlackbackRange();
    void slotUpdateInfiniteFramesCount();

    void slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void slotHeaderDataChanged(Qt::Orientation orientation, int first, int last);

    void slotColorLabelChanged(int);

    // Layer
    void slotAddNewLayer();
    void slotAddExistingLayer(QAction *action);
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

    // Copy, paste & clone.
    void slotCopyFrames() {cutCopyImpl(false, true);}
    void slotCutFrames() {cutCopyImpl(false, false);}

    void slotCopyColumns() {cutCopyImpl(true, true);}
    void slotCutColumns() {cutCopyImpl(true, false);}

    void slotPasteFrames(bool entireColumn = false);
    void slotPasteColumns() {slotPasteFrames(true);}

    void slotMakeClonesUnique();

    // Audio
    void slotSelectAudioChannelFile();
    void slotAudioChannelMute(bool value);
    void slotAudioChannelRemove();
    void slotUpdateAudioActions();
    void slotAudioVolumeChanged(int value);

    // Zoom & Scroll
    void slotScrollerStateChanged(QScroller::State state);
    void slotZoom(qreal zoom);
    void slotUpdateDragInfiniteFramesCount();
    void slotRealignScrollBars();
    void slotEnsureRowVisible(int row);

protected:
    bool viewportEvent(QEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

    void startDrag(Qt::DropActions supportedActions) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void wheelEvent(QWheelEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

    void rowsInserted(const QModelIndex &parent, int start, int end) override;
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

    QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &index,
                                                         const QEvent *event) const override;

private:
    void setFramesPerSecond(int fps);

    QModelIndexList calculateSelectionSpan(bool entireColumn, bool editableOnly = true) const;
    void calculateSelectionMetrics(int &minColumn, int &maxColumn, QSet<int> &rows) const;

    /**   Insert new keyframes/columns.
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
    void fanSelectedFrames(const QModelIndexList &selection, int count, bool ignoreKeyless = true);

    void cutCopyImpl(bool entireColumn, bool copy);
    void clone(bool entireColumn);

    void createFrameEditingMenuActions(QMenu *menu, bool emptyFrame, bool cloneFrameSelected);

    int scrollPositionFromColumn(int column);

    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __TIMELINE_FRAMES_VIEW_H */
