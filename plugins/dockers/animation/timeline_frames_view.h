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
#include "kis_action_manager.h"
#include "kritaanimationdocker_export.h"

class KisAction;
class TimelineWidget;


class KRITAANIMATIONDOCKER_EXPORT TimelineFramesView : public QTableView
{
    Q_OBJECT
public:
    TimelineFramesView(QWidget *parent);
    ~TimelineFramesView() override;

    void setModel(QAbstractItemModel *model) override;

    void updateGeometries() override;

    void setShowInTimeline(KisAction *action);

    void setActionManager( KisActionManager * actionManager);

public Q_SLOTS:
    void slotSelectionChanged();
    void slotUpdateIcons();

private Q_SLOTS:
    void slotUpdateLayersMenu();
    void slotUpdateFrameActions();

    void slotAddNewLayer();
    void slotAddExistingLayer(QAction *action);
    void slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    void slotRemoveLayer();

    void slotLayerContextMenuRequested(const QPoint &globalPos);

    void slotNewFrame();
    void slotCopyFrame();


    void slotInsertKeyframesLeft(int count = -1, bool forceEntireColumn = false);
    void slotInsertKeyframesRight(int count = -1, bool forceEntireColumn = false);

    void slotInsertKeyframesLeftCustom();
    void slotInsertKeyframesRightCustom();

    void slotRemoveFrame(bool forceEntireColumn = false, bool needsOffset = false);
    void slotRemoveFramesAndShift(bool forceEntireColumn = false);

    void slotInsertColumnsLeft(int count = -1);
    void slotInsertColumnsLeftCustom();

    void slotInsertColumnsRight(int count = -1);
    void slotInsertColumnsRightCustom();

    void slotRemoveColumns();
    void slotRemoveColumnsAndShift();

    void slotInsertHoldFrames(int count = 1, bool forceEntireColumn = false);
    void slotRemoveHoldFrames(int count = 1, bool forceEntireColumn = false);

    void slotInsertHoldFramesCustom();
    void slotRemoveHoldFramesCustom();

    void slotInsertHoldColumns(int count = 1);
    void slotRemoveHoldColumns(int count = 1);

    void slotInsertHoldColumnsCustom();
    void slotRemoveHoldColumnsCustom();

    void slotMirrorFrames(bool forceEntireColumn = false);
    void slotMirrorColumns();


    void slotCopyFrames(bool forceEntireColumn = false);
    void slotCutFrames(bool forceEntireColumn = false);
    void slotPasteFrames(bool forceEntireColumn = false);

    void slotCopyColumns();
    void slotCutColumns();
    void slotPasteColumns();

    void slotReselectCurrentIndex();

    void slotUpdateInfiniteFramesCount();

    void slotHeaderDataChanged(Qt::Orientation orientation, int first, int last);

    void slotZoomButtonPressed(qreal staticPoint);
    void slotZoomButtonChanged(qreal value);

    void slotColorLabelChanged(int);
    void slotEnsureRowVisible(int row);


    void slotSelectAudioChannelFile();
    void slotAudioChannelMute(bool value);
    void slotAudioChannelRemove();
    void slotUpdateAudioActions();
    void slotAudioVolumeChanged(int value);

private:
    void setFramesPerSecond(int fps);
    void calculateSelectionMetrics(int &minColumn, int &maxColumn, QSet<int> &rows) const;

    void insertFramesImpl(int insertAtColumn, int count, QSet<int> rows, bool forceEntireColumn);

    void createFrameEditingMenuActions(QMenu *menu, bool addFrameCreationActions);

    QModelIndexList calculateSelectionSpan(bool forceEntireColumn, bool editableOnly = true) const;
    void cutCopyImpl(bool forceEntireColumn, bool copy);

    int defaultNumberOfFramesToAdd() const;
    void setDefaultNumberOfFramesToAdd(int value) const;

    int defaultNumberOfColumnsToAdd() const;
    void setDefaultNumberOfColumnsToAdd(int value) const;

    int defaultNumberOfFramesToRemove() const;
    void setDefaultNumberOfFramesToRemove(int value) const;

    int defaultNumberOfColumnsToRemove() const;
    void setDefaultNumberOfColumnsToRemove(int value) const;

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
    void rowsInserted(const QModelIndex& parent, int start, int end) override;
    bool viewportEvent(QEvent *event) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __TIMELINE_FRAMES_VIEW_H */
