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
#include "kis_animation_curves_view.h"

#include "kis_animation_curves_model.h"
#include "timeline_ruler_header.h"
#include "kis_animation_curves_value_ruler.h"
#include "kis_animation_curves_keyframe_delegate.h"
#include "kis_scalar_keyframe_channel.h"

#include <QPaintEvent>
#include <QMouseEvent>
#include <QApplication>
#include <qpainter.h>

struct KisAnimationCurvesView::Private
{
    Private()
        : isDragging(false)
    {}

    TimelineRulerHeader *horizontalHeader;
    KisAnimationCurvesValueRuler *verticalHeader;
    KisAnimationCurvesKeyframeDelegate *itemDelegate;

    bool isDragging;
    QPoint dragStart;
    QPoint dragOffset;
};

KisAnimationCurvesView::KisAnimationCurvesView(QWidget *parent)
    : QAbstractItemView(parent)
    , m_d(new Private())
{
    m_d->horizontalHeader = new TimelineRulerHeader(this);
    m_d->verticalHeader = new KisAnimationCurvesValueRuler(this);
    m_d->itemDelegate = new KisAnimationCurvesKeyframeDelegate(m_d->horizontalHeader, m_d->verticalHeader, this);

    setSelectionMode(QAbstractItemView::ExtendedSelection);

}

KisAnimationCurvesView::~KisAnimationCurvesView()
{}

void KisAnimationCurvesView::setModel(QAbstractItemModel *model)
{
    QAbstractItemView::setModel(model);
    m_d->horizontalHeader->setModel(model);

    connect(model, &QAbstractItemModel::dataChanged,
            this, &KisAnimationCurvesView::slotDataChanged);

    connect(model, &QAbstractItemModel::headerDataChanged,
            this, &KisAnimationCurvesView::slotHeaderDataChanged);
}

QRect KisAnimationCurvesView::visualRect(const QModelIndex &index) const
{
    return m_d->itemDelegate->itemRect(index);
}

void KisAnimationCurvesView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{
    // TODO
}

QModelIndex KisAnimationCurvesView::indexAt(const QPoint &point) const
{
    int time = m_d->horizontalHeader->logicalIndexAt(point.x());

    int rows = model()->rowCount();
    for (int row=0; row < rows; row++) {
        QModelIndex index = model()->index(row, time);

        if (index.data(KisTimeBasedItemModel::SpecialKeyframeExists).toBool()) {
            QRect nodePos = m_d->itemDelegate->itemRect(index);

            if (nodePos.contains(point)) {
                return index;
            }
        }
    }

    return QModelIndex();
}

void KisAnimationCurvesView::paintEvent(QPaintEvent *e)
{
    QPainter painter(viewport());

    QRect r = e->rect();
    r.translate(dirtyRegionOffset());

    int firstFrame = m_d->horizontalHeader->visualIndexAt(r.left());
    int lastFrame = m_d->horizontalHeader->visualIndexAt(r.right());

    paintCurves(painter, firstFrame, lastFrame);
    paintKeyframes(painter, firstFrame, lastFrame);
}

void KisAnimationCurvesView::paintCurves(QPainter &painter, int firstFrame, int lastFrame)
{
    int channels = model()->rowCount();
    for (int channel = 0; channel < channels; channel++) {
        QModelIndex index0 = model()->index(channel, 0);
        QColor color = index0.data(KisAnimationCurvesModel::CurveColorRole).value<QColor>();
        painter.setPen(QPen(color, 1));

        paintCurve(channel, firstFrame, lastFrame, painter);
    }
}

void KisAnimationCurvesView::paintCurve(int channel, int firstFrame, int lastFrame, QPainter &painter)
{
    int selectionOffset = m_d->isDragging ? (m_d->dragOffset.x() / m_d->horizontalHeader->defaultSectionSize()) : 0;

    QModelIndex index = findNextKeyframeIndex(channel, firstFrame+1, selectionOffset, true);
    if (!index.isValid()) {
        index = findNextKeyframeIndex(channel, firstFrame, selectionOffset, false);
    }
    if (!index.isValid()) return;

    QPointF previousKeyPos = m_d->itemDelegate->nodeCenter(index, selectionModel()->isSelected(index));
    QPointF rightTangent = m_d->itemDelegate->rightHandle(index);

    while(index.column() <= lastFrame) {
        int currentIndexTime = (m_d->isDragging && selectionModel()->isSelected(index)) ? index.column() + selectionOffset : index.column();
        index = findNextKeyframeIndex(channel, currentIndexTime, selectionOffset, false);
        if (!index.isValid()) return;

        QPointF nextKeyPos = m_d->itemDelegate->nodeCenter(index, selectionModel()->isSelected(index));
        QPointF leftTangent = m_d->itemDelegate->leftHandle(index);

        paintCurveSegment(painter, previousKeyPos, rightTangent, leftTangent, nextKeyPos);

        previousKeyPos = nextKeyPos;
        rightTangent = m_d->itemDelegate->rightHandle(index);
    }
}

void KisAnimationCurvesView::paintCurveSegment(QPainter &painter, QPointF pos1, QPointF rightTangent, QPointF leftTangent, QPointF pos2) {
    const int steps = 16;
    QPointF previousPos;

    for (int step = 0; step <= steps; step++) {
        qreal t = 1.0 * step / steps;

        QPointF nextPos = KisScalarKeyframeChannel::interpolate(pos1, rightTangent, leftTangent, pos2, t);

        if (step > 0) {
            painter.drawLine(previousPos, nextPos);
        }

        previousPos = nextPos;
    }
}

void KisAnimationCurvesView::paintKeyframes(QPainter &painter, int firstFrame, int lastFrame)
{
    int channels = model()->rowCount();
    for (int channel = 0; channel < channels; channel++) {
        for (int time=firstFrame; time <= lastFrame; time++) {
            QModelIndex index = model()->index(channel, time);
            bool keyframeExists = model()->data(index, KisAnimationCurvesModel::SpecialKeyframeExists).toReal();

            if (keyframeExists) {
                QStyleOptionViewItem opt;

                if (selectionModel()->isSelected(index)) {
                    opt.state = QStyle::State_Selected;
                }

                m_d->itemDelegate->paint(&painter, opt, index);
            }
        }
    }
}

QModelIndex KisAnimationCurvesView::findNextKeyframeIndex(int channel, int time, int selectionOffset, bool backward)
{
    KisAnimationCurvesModel::ItemDataRole role =
            backward ? KisAnimationCurvesModel::PreviousKeyframeTime : KisAnimationCurvesModel::NextKeyframeTime;
    QModelIndex currentIndex = model()->index(channel, time);

    if (!selectionOffset) {
        QVariant next = currentIndex.data(role);
        return (next.isValid()) ? model()->index(channel, next.toInt()) : QModelIndex();
    } else {
        // Find the next unselected index
        QModelIndex nextIndex = currentIndex;
        do {
            QVariant next = nextIndex.data(role);
            nextIndex = (next.isValid()) ? model()->index(channel, next.toInt()) : QModelIndex();
        } while(nextIndex.isValid() && selectionModel()->isSelected(nextIndex));

        // Find the next selected index, accounting for D&D offset
        QModelIndex draggedIndex = model()->index(channel, qMax(0, time - selectionOffset));
        do {
            QVariant next = draggedIndex.data(role);
            draggedIndex = (next.isValid()) ? model()->index(channel, next.toInt()) : QModelIndex();
        } while(draggedIndex.isValid() && !selectionModel()->isSelected(draggedIndex));

        // Choose the earlier of the two
        if (draggedIndex.isValid() && nextIndex.isValid()) {
            if (draggedIndex.column() + selectionOffset <= nextIndex.column()) {
                return draggedIndex;
            } else {
                return nextIndex;
            }
        } else if (draggedIndex.isValid()) {
            return draggedIndex;
        } else {
            return nextIndex;
        }
    }
}

QModelIndex KisAnimationCurvesView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    // TODO
    return QModelIndex();
}

int KisAnimationCurvesView::horizontalOffset() const
{
    return m_d->horizontalHeader->offset();
}

int KisAnimationCurvesView::verticalOffset() const
{
    return 0;
}

bool KisAnimationCurvesView::isIndexHidden(const QModelIndex &index) const
{
    // TODO
    return false;
}

void KisAnimationCurvesView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    int timeFrom = m_d->horizontalHeader->logicalIndexAt(rect.left());
    int timeTo = m_d->horizontalHeader->logicalIndexAt(rect.right());

    QItemSelection selection;

    int rows = model()->rowCount();
    for (int row=0; row < rows; row++) {
        for (int time = timeFrom; time <= timeTo; time++) {
            QModelIndex index = model()->index(row, time);

            if (index.data(KisTimeBasedItemModel::SpecialKeyframeExists).toBool()) {
                QRect itemRect = m_d->itemDelegate->itemRect(index);

                if (itemRect.intersects(rect)) {
                    selection.select(index, index);
                }
            }
        }
    }

    selectionModel()->select(selection, command);
}

QRegion KisAnimationCurvesView::visualRegionForSelection(const QItemSelection &selection) const
{
    QRegion region;

    Q_FOREACH(QModelIndex index, selection.indexes()) {
        region += m_d->itemDelegate->visualRect(index);
    }

    return region;
}

void KisAnimationCurvesView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_d->dragStart = e->pos();
    }

    QAbstractItemView::mousePressEvent(e);
}

void KisAnimationCurvesView::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons() & Qt::LeftButton) {
        if (!m_d->isDragging && selectionModel()->hasSelection()) {
            if ((e->pos() - m_d->dragStart).manhattanLength() > QApplication::startDragDistance()) {
                m_d->isDragging = true;
            }
        }

        if (m_d->isDragging) {
            m_d->dragOffset = e->pos() - m_d->dragStart;
            m_d->itemDelegate->setSelectedItemVisualOffset(m_d->dragOffset);
            viewport()->update();
            return;
        }
    }

    QAbstractItemView::mouseMoveEvent(e);
}

void KisAnimationCurvesView::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        if (m_d->isDragging) {
            QModelIndexList indexes = selectedIndexes();
            int timeOffset = m_d->dragOffset.x() / m_d->horizontalHeader->defaultSectionSize();
            qreal valueOffset = m_d->dragOffset.y() / m_d->verticalHeader->scaleFactor();

            KisAnimationCurvesModel *curvesModel = dynamic_cast<KisAnimationCurvesModel*>(model());
            curvesModel->adjustKeyframes(indexes, timeOffset, valueOffset);

            m_d->isDragging = false;
            m_d->itemDelegate->setSelectedItemVisualOffset(QPointF());
            viewport()->update();
        }
    }

    QAbstractItemView::mouseReleaseEvent(e);
}

void KisAnimationCurvesView::updateGeometries()
{
    int topMargin = qMax(m_d->horizontalHeader->minimumHeight(),
                         m_d->horizontalHeader->sizeHint().height());

    int leftMargin = m_d->verticalHeader->sizeHint().width();

    setViewportMargins(leftMargin, topMargin, 0, 0);

    QRect viewRect = viewport()->geometry();
    m_d->horizontalHeader->setGeometry(leftMargin, 0, viewRect.width(), topMargin);
    m_d->verticalHeader->setGeometry(0, topMargin, leftMargin, viewRect.height());

    QAbstractItemView::updateGeometries();
}

void KisAnimationCurvesView::slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    viewport()->update();
}

void KisAnimationCurvesView::slotHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
    viewport()->update();
}
