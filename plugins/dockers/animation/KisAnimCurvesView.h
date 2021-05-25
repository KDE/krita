/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_ANIMATION_CURVES_VIEW_H
#define _KIS_ANIMATION_CURVES_VIEW_H

#include <QScopedPointer>
#include <QTableView>
#include <KisKineticScroller.h>

class KisAction;
class KisZoomButton;

class KisAnimCurvesView : public QAbstractItemView
{
    Q_OBJECT
public:
    KisAnimCurvesView(QWidget *parent);
    ~KisAnimCurvesView() override;

    void setModel(QAbstractItemModel *model) override;

    QRect visualRect(const QModelIndex &index) const override;
    void scrollTo(const QModelIndex &index, ScrollHint hint) override;
    QModelIndex indexAt(const QPoint &point) const override;
    bool indexHasKey(const QModelIndex& index);

Q_SIGNALS:
    void activeDataChanged(const QModelIndex& index);

protected:
    void paintEvent(QPaintEvent *event) override;
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
    int horizontalOffset() const override;
    int verticalOffset() const override;
    bool isIndexHidden(const QModelIndex &index) const override;
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags) override;
    QRegion visualRegionForSelection(const QItemSelection &selection) const override;
    void scrollContentsBy(int dx, int dy) override;

    void mousePressEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
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

    void zoomToFitCurve();
    void zoomToFitChannel();
    void changeZoom(Qt::Orientation orientation, qreal zoomDelta);

    void slotScrollerStateChanged(QScroller::State state){KisKineticScroller::updateCursor(this, state);}

protected Q_SLOTS:
    void updateGeometries() override;

private Q_SLOTS:
    void slotRowsChanged(const QModelIndex &parentIndex, int first, int last);
    void slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void slotDataAdded(const QModelIndex &index);
    void slotHeaderDataChanged(Qt::Orientation orientation, int first, int last);
    void slotUpdateInfiniteFramesCount();
    void slotUpdateHorizontalScrollbarSize();

private:
    struct Private;
    const QScopedPointer<Private> m_d;

    void paintGrid(QPainter &painter);
    void paintCurves(QPainter &painter, int firstFrame, int lastFrame);
    void paintCurve(int channel, int firstFrame, int lastFrame, QPainter &painter);
    void paintCurveSegment(QPainter &painter, QPointF pos1, QPointF rightTangent, QPointF leftTangent, QPointF pos2, QVariant limitData);
    void paintKeyframes(QPainter &painter, int firstFrame, int lastFrame);

    QModelIndex findNextKeyframeIndex(int channel, int time, int selectionOffset, bool backward);
    void findExtremes(qreal *minimum, qreal *maximum);
};

#endif
