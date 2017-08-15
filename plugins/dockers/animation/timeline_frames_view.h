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

    QMap<QString, KisAction*> globalActions() const;

    void setShowInTimeline(KisAction *action);

public Q_SLOTS:
    void slotSelectionChanged();

private Q_SLOTS:
    void slotUpdateLayersMenu();

    void slotAddNewLayer();
    void slotAddExistingLayer(QAction *action);
    void slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    void slotRemoveLayer();

    void slotLayerContextMenuRequested(const QPoint &globalPos);

    void slotNewFrame();
    void slotCopyFrame();
    void slotRemoveFrame();

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
