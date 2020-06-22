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
#include <KisKineticScroller.h>

class KisAction;
class KisZoomButton;

class KisAnimationCurvesView : public QAbstractItemView
{
    Q_OBJECT
public:
    KisAnimationCurvesView(QWidget *parent);
    ~KisAnimationCurvesView() override;

    void setModel(QAbstractItemModel *model) override;
    void setZoomButtons(KisZoomButton *horizontal, KisZoomButton *vertical);

    QRect visualRect(const QModelIndex &index) const override;
    void scrollTo(const QModelIndex &index, ScrollHint hint) override;
    QModelIndex indexAt(const QPoint &point) const override;

protected:
    void paintEvent(QPaintEvent *) override;
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
    int horizontalOffset() const override;
    int verticalOffset() const override;
    bool isIndexHidden(const QModelIndex &index) const override;
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) override;
    QRegion visualRegionForSelection(const QItemSelection &selection) const override;
    void scrollContentsBy(int dx, int dy) override;

    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;

public Q_SLOTS:
    void applyConstantMode();
    void applyLinearMode();
    void applyBezierMode();
    void applySmoothMode();
    void applySharpMode();

    void createKeyframe();
    void removeKeyframes();

    void zoomToFit();

    void slotScrollerStateChanged(QScroller::State state){KisKineticScroller::updateCursor(this, state);}

protected Q_SLOTS:
    void updateGeometries() override;

private Q_SLOTS:
    void slotRowsChanged(const QModelIndex &parentIndex, int first, int last);
    void slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void slotHeaderDataChanged(Qt::Orientation orientation, int first, int last);

    void slotHorizontalZoomStarted(qreal staticPoint);
    void slotVerticalZoomStarted(qreal staticPoint);
    void slotHorizontalZoomLevelChanged(qreal level);
    void slotVerticalZoomLevelChanged(qreal level);

private:
    struct Private;
    const QScopedPointer<Private> m_d;

    void paintFrames(QPainter &painter);
    void paintCurves(QPainter &painter, int firstFrame, int lastFrame);
    void paintCurve(int channel, int firstFrame, int lastFrame, QPainter &painter);
    void paintCurveSegment(QPainter &painter, QPointF pos1, QPointF rightTangent, QPointF leftTangent, QPointF pos2);
    void paintKeyframes(QPainter &painter, int firstFrame, int lastFrame);
    QModelIndex findNextKeyframeIndex(int channel, int time, int selectionOffset, bool backward);

    void findExtremes(qreal *minimum, qreal *maximum);
    void updateVerticalRange();



    void startPan(QPoint mousePos);
};

#endif
