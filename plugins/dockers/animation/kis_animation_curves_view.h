/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
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

#ifndef _KIS_ANIMATION_CURVES_VIEW_H
#define _KIS_ANIMATION_CURVES_VIEW_H

#include <QScopedPointer>
#include <QTableView>

class KisAction;
class TimelineWidget;

class KisAnimationCurvesView : public QAbstractItemView
{
    Q_OBJECT
public:
    KisAnimationCurvesView(QWidget *parent);
    ~KisAnimationCurvesView();

    void setModel(QAbstractItemModel *model);

public:
    QRect visualRect(const QModelIndex &index) const;
    void scrollTo(const QModelIndex &index, ScrollHint hint);
    QModelIndex indexAt(const QPoint &point) const;

protected:
    void paintEvent(QPaintEvent *);
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
    int horizontalOffset() const;
    int verticalOffset() const;
    bool isIndexHidden(const QModelIndex &index) const;
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);
    QRegion visualRegionForSelection(const QItemSelection &selection) const;

protected Q_SLOTS:
    void updateGeometries();

private Q_SLOTS:

    void slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void slotHeaderDataChanged(Qt::Orientation orientation, int first, int last);

private:
    struct Private;
    const QScopedPointer<Private> m_d;

    void paintCurves(QPainter &painter, int firstFrame, int lastFrame);
    void paintKeyframes(QPainter &painter, int firstFrame, int lastFrame);
};

#endif
