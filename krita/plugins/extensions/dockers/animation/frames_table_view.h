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

#ifndef __FRAMES_TABLE_VIEW_H
#define __FRAMES_TABLE_VIEW_H

#include <QScopedPointer>
#include <QTableView>

#include "kritaanimationdocker_export.h"

class TimelineWidget;


class KRITAANIMATIONDOCKER_EXPORT FramesTableView : public QTableView
{
    Q_OBJECT
public:
    FramesTableView(QWidget *parent);
    ~FramesTableView();

    void setModel(QAbstractItemModel *model);

    void updateGeometries();

    qreal zoom() const;

public Q_SLOTS:
    void setZoom(qreal zoom);
    void setZoomDouble(double zoom);

private Q_SLOTS:
    void slotUpdateLayersMenu();

    void slotAddNewLayer();
    void slotAddExistingLayer(QAction *action);
    void slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    void slotRemoveLayer();
    void slotHideLayerFromTimeline();

    void slotLayerContextMenuRequested(const QPoint &globalPos);

    void slotNewFrame();
    void slotCopyFrame();
    void slotRemoveFrame();

    void slotReselectCurrentIndex();

    void slotUpdateInfiniteFramesCount();

    void slotHeaderDataChanged(Qt::Orientation orientation, int first, int last);

    void slotZoomButtonPressed();
    void slotZoomButtonChanged(int value);

private:
    void setFramesPerSecond(int fps);
    void slotZoomButtonPressedImpl();

protected:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __FRAMES_TABLE_VIEW_H */
