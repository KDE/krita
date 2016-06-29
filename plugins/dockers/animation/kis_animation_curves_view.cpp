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

#include <QPaintEvent>
#include <qpainter.h>

struct KisAnimationCurvesView::Private
{
    Private()
    {}

    TimelineRulerHeader *horizontalHeader;
    KisAnimationCurvesValueRuler *verticalHeader;
    KisAnimationCurvesKeyframeDelegate *itemDelegate;
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
    // Make sure we draw lines crossing the edges of the refreshed rect
    firstFrame = qMax(0, firstFrame -1);
    lastFrame = lastFrame + 1;

    int channels = model()->rowCount();
    for (int channel = 0; channel < channels; channel++) {
        QPointF previousNodePos;

        QModelIndex index0 = model()->index(channel, 0);
        QColor color = index0.data(KisAnimationCurvesModel::CurveColorRole).value<QColor>();
        painter.setPen(QPen(color, 1));

        for (int time = firstFrame; time <= lastFrame; time++) {
            QModelIndex index = model()->index(channel, time);
            QPointF nodePos = m_d->itemDelegate->nodeCenter(index);

            if (time > firstFrame) {
                painter.drawLine(previousNodePos, nodePos);
            }

            previousNodePos = nodePos;
        }
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
    update();
}

void KisAnimationCurvesView::slotHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
    update();
}
