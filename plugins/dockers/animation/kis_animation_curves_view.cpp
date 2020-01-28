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

#include <QPaintEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QScrollBar>
#include <qpainter.h>
#include <QtMath>

#include "kis_animation_curves_model.h"
#include "timeline_ruler_header.h"
#include "kis_animation_curves_value_ruler.h"
#include "kis_animation_curves_keyframe_delegate.h"
#include "kis_scalar_keyframe_channel.h"
#include "kis_zoom_button.h"
#include "kis_custom_modifiers_catcher.h"

const int VERTICAL_PADDING = 30;

struct KisAnimationCurvesView::Private
{
    Private()
    {}

    KisAnimationCurvesModel *model {0};
    TimelineRulerHeader *horizontalHeader {0};
    KisAnimationCurvesValueRuler *verticalHeader {0};
    KisAnimationCurvesKeyframeDelegate *itemDelegate {0};
    KisZoomButton *horizontalZoomButton {0};
    KisZoomButton *verticalZoomButton {0};
    KisCustomModifiersCatcher *modifiersCatcher {0};

    bool isDraggingKeyframe {false};
    bool isAdjustingHandle {false};
    int adjustedHandle {0}; // 0 = left, 1 = right
    QPoint dragStart;
    QPoint dragOffset;

    int horizontalZoomStillPointIndex {0};
    int horizontalZoomStillPointOriginalOffset {0};
    qreal verticalZoomStillPoint {0.0};
    qreal verticalZoomStillPointOriginalOffset {0.0};

    bool panning {false};
    QPoint panStartOffset;
};

KisAnimationCurvesView::KisAnimationCurvesView(QWidget *parent)
    : QAbstractItemView(parent)
    , m_d(new Private())
{
    m_d->horizontalHeader = new TimelineRulerHeader(this);
    m_d->verticalHeader = new KisAnimationCurvesValueRuler(this);
    m_d->itemDelegate = new KisAnimationCurvesKeyframeDelegate(m_d->horizontalHeader, m_d->verticalHeader, this);

    m_d->modifiersCatcher = new KisCustomModifiersCatcher(this);
    m_d->modifiersCatcher->addModifier("pan-zoom", Qt::Key_Space);

    setSelectionMode(QAbstractItemView::ExtendedSelection);

    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(this);
    if (scroller){
        connect(scroller, SIGNAL(stateChanged(QScroller::State)),
                this, SLOT(slotScrollerStateChanged(QScroller::State)));
    }
}

KisAnimationCurvesView::~KisAnimationCurvesView()
{}

void KisAnimationCurvesView::setModel(QAbstractItemModel *model)
{
    m_d->model = dynamic_cast<KisAnimationCurvesModel*>(model);

    QAbstractItemView::setModel(model);
    m_d->horizontalHeader->setModel(model);

    connect(model, &QAbstractItemModel::rowsInserted,
            this, &KisAnimationCurvesView::slotRowsChanged);

    connect(model, &QAbstractItemModel::rowsRemoved,
            this, &KisAnimationCurvesView::slotRowsChanged);

    connect(model, &QAbstractItemModel::dataChanged,
            this, &KisAnimationCurvesView::slotDataChanged);

    connect(model, &QAbstractItemModel::headerDataChanged,
            this, &KisAnimationCurvesView::slotHeaderDataChanged);
}

void KisAnimationCurvesView::setZoomButtons(KisZoomButton *horizontal, KisZoomButton *vertical)
{
    m_d->horizontalZoomButton = horizontal;
    m_d->verticalZoomButton = vertical;

    connect(horizontal, &KisZoomButton::zoomStarted, this, &KisAnimationCurvesView::slotHorizontalZoomStarted);
    connect(horizontal, &KisZoomButton::zoomLevelChanged, this, &KisAnimationCurvesView::slotHorizontalZoomLevelChanged);
    connect(vertical, &KisZoomButton::zoomStarted, this, &KisAnimationCurvesView::slotVerticalZoomStarted);
    connect(vertical, &KisZoomButton::zoomLevelChanged, this, &KisAnimationCurvesView::slotVerticalZoomLevelChanged);
}

QRect KisAnimationCurvesView::visualRect(const QModelIndex &index) const
{
    return m_d->itemDelegate->itemRect(index);
}

void KisAnimationCurvesView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{
    // TODO
    Q_UNUSED(index);
    Q_UNUSED(hint);
}

QModelIndex KisAnimationCurvesView::indexAt(const QPoint &point) const
{
    if (!model()) return QModelIndex();

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

    int firstFrame = m_d->horizontalHeader->logicalIndexAt(r.left());
    int lastFrame = m_d->horizontalHeader->logicalIndexAt(r.right());
    if (lastFrame == -1) lastFrame = model()->columnCount();

    paintCurves(painter, firstFrame, lastFrame);
    paintKeyframes(painter, firstFrame, lastFrame);
}

void KisAnimationCurvesView::paintCurves(QPainter &painter, int firstFrame, int lastFrame)
{
    int channels = model()->rowCount();
    for (int channel = 0; channel < channels; channel++) {
        QModelIndex index0 = model()->index(channel, 0);

        if (!isIndexHidden(index0)) {
            QColor color = index0.data(KisAnimationCurvesModel::CurveColorRole).value<QColor>();
            painter.setPen(QPen(color, 1));

            paintCurve(channel, firstFrame, lastFrame, painter);
        }
    }
}

void KisAnimationCurvesView::paintCurve(int channel, int firstFrame, int lastFrame, QPainter &painter)
{
    int selectionOffset = m_d->isDraggingKeyframe ? (m_d->dragOffset.x() / m_d->horizontalHeader->defaultSectionSize()) : 0;

    QModelIndex index = findNextKeyframeIndex(channel, firstFrame+1, selectionOffset, true);
    if (!index.isValid()) {
        index = findNextKeyframeIndex(channel, firstFrame, selectionOffset, false);
    }
    if (!index.isValid()) return;

    QPointF previousKeyPos = m_d->itemDelegate->nodeCenter(index, selectionModel()->isSelected(index));
    QPointF rightTangent = m_d->itemDelegate->rightHandle(index, index == currentIndex());

    while(index.column() <= lastFrame) {
        int interpolationMode = index.data(KisAnimationCurvesModel::InterpolationModeRole).toInt();

        int time = (m_d->isDraggingKeyframe && selectionModel()->isSelected(index)) ? index.column() + selectionOffset : index.column();
        index = findNextKeyframeIndex(channel, time, selectionOffset, false);
        if (!index.isValid()) return;

        bool active = (index == currentIndex());
        QPointF nextKeyPos = m_d->itemDelegate->nodeCenter(index, selectionModel()->isSelected(index));
        QPointF leftTangent = m_d->itemDelegate->leftHandle(index, active);

        if (interpolationMode == KisKeyframe::Constant) {
            painter.drawLine(previousKeyPos, QPointF(nextKeyPos.x(), previousKeyPos.y()));
        } else if (interpolationMode == KisKeyframe::Linear) {
            painter.drawLine(previousKeyPos, nextKeyPos);
        } else {
            paintCurveSegment(painter, previousKeyPos, rightTangent, leftTangent, nextKeyPos);
        }

        previousKeyPos = nextKeyPos;
        rightTangent = m_d->itemDelegate->rightHandle(index, active);
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

            if (keyframeExists && !isIndexHidden(index)) {
                QStyleOptionViewItem opt;

                if (selectionModel()->isSelected(index)) {
                    opt.state |= QStyle::State_Selected;
                }

                if (index == selectionModel()->currentIndex()) {
                    opt.state |= QStyle::State_HasFocus;
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

void KisAnimationCurvesView::findExtremes(qreal *minimum, qreal *maximum)
{
    if (!model()) return;

    qreal min = qInf();
    qreal max = -qInf();

    int rows = model()->rowCount();
    for (int row = 0; row < rows; row++) {
        QModelIndex index = model()->index(row, 0);

        if (isIndexHidden(index)) continue;

        QVariant nextTime;
        do {
            qreal value = index.data(KisAnimationCurvesModel::ScalarValueRole).toReal();

            if (value < min) min = value;
            if (value > max) max = value;

            nextTime = index.data(KisAnimationCurvesModel::NextKeyframeTime);
            if (nextTime.isValid()) index = model()->index(row, nextTime.toInt());
        } while (nextTime.isValid());
    }

    if (qIsFinite(min)) *minimum = min;
    if (qIsFinite(max)) *maximum = max;
}

void KisAnimationCurvesView::updateVerticalRange()
{
    if (!model()) return;

    qreal minimum = 0;
    qreal maximum = 0;
    findExtremes(&minimum, &maximum);

    int viewMin = maximum * m_d->verticalHeader->scaleFactor();
    int viewMax = minimum * m_d->verticalHeader->scaleFactor();

    viewMin -= VERTICAL_PADDING;
    viewMax += VERTICAL_PADDING;

    verticalScrollBar()->setRange(viewMin, viewMax - viewport()->height());
}

void KisAnimationCurvesView::startPan(QPoint mousePos)
{
    m_d->dragStart = mousePos;
    m_d->panStartOffset = QPoint(horizontalOffset(), verticalOffset());
    m_d->panning = true;
}

QModelIndex KisAnimationCurvesView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    // TODO
    Q_UNUSED(cursorAction);
    Q_UNUSED(modifiers);
    return QModelIndex();
}

int KisAnimationCurvesView::horizontalOffset() const
{
    return m_d->horizontalHeader->offset();
}

int KisAnimationCurvesView::verticalOffset() const
{
    return m_d->verticalHeader->offset();
}

bool KisAnimationCurvesView::isIndexHidden(const QModelIndex &index) const
{
    return !index.data(KisAnimationCurvesModel::CurveVisibleRole).toBool();
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
    if (m_d->modifiersCatcher->modifierPressed("pan-zoom")) {
        if (e->button() == Qt::LeftButton) {
            startPan(e->pos());
        } else {
            qreal horizontalStaticPoint = m_d->horizontalHeader->logicalIndexAt(e->pos().x());
            qreal verticalStaticPoint = m_d->verticalHeader->mapViewToValue(e->pos().y());

            m_d->horizontalZoomButton->beginZoom(QPoint(e->pos().x(), 0), horizontalStaticPoint);
            m_d->verticalZoomButton->beginZoom(QPoint(0, e->pos().y()), verticalStaticPoint);
        }
    } else if (e->button() == Qt::LeftButton) {
        m_d->dragStart = e->pos();

        Q_FOREACH(QModelIndex index, selectedIndexes()) {
            QPointF center = m_d->itemDelegate->nodeCenter(index, false);
            bool hasLeftHandle = m_d->itemDelegate->hasHandle(index, 0);
            bool hasRightHandle = m_d->itemDelegate->hasHandle(index, 1);

            QPointF leftHandle = center + m_d->itemDelegate->leftHandle(index, false);
            QPointF rightHandle = center + m_d->itemDelegate->rightHandle(index, false);

            if (hasLeftHandle && (e->localPos() - leftHandle).manhattanLength() < 8) {
                m_d->isAdjustingHandle = true;
                m_d->adjustedHandle = 0;
                setCurrentIndex(index);
                return;
            } else if (hasRightHandle && (e->localPos() - rightHandle).manhattanLength() < 8) {
                m_d->isAdjustingHandle = true;
                m_d->adjustedHandle = 1;
                setCurrentIndex(index);
                return;
            }
        }
    }

    QAbstractItemView::mousePressEvent(e);
}

void KisAnimationCurvesView::mouseMoveEvent(QMouseEvent *e)
{
    if (m_d->modifiersCatcher->modifierPressed("pan-zoom")) {
        if (e->buttons() & Qt::LeftButton) {
            if (!m_d->panning) startPan(e->pos());

            QPoint diff = e->pos() - m_d->dragStart;
            QPoint newOffset = m_d->panStartOffset - diff;

            horizontalScrollBar()->setValue(newOffset.x());
            verticalScrollBar()->setValue(newOffset.y());
            m_d->verticalHeader->setOffset(newOffset.y());
            viewport()->update();
        } else {
            m_d->horizontalZoomButton->continueZoom(QPoint(e->pos().x(), 0));
            m_d->verticalZoomButton->continueZoom(QPoint(0, e->pos().y()));
        }
    } else if (e->buttons() & Qt::LeftButton) {
        m_d->dragOffset = e->pos() - m_d->dragStart;

        if (m_d->isAdjustingHandle) {
            m_d->itemDelegate->setHandleAdjustment(m_d->dragOffset, m_d->adjustedHandle);
            viewport()->update();
            return;
        } else if (m_d->isDraggingKeyframe) {
            m_d->itemDelegate->setSelectedItemVisualOffset(m_d->dragOffset);
            viewport()->update();
            return;
        } else if (selectionModel()->hasSelection()) {
            if ((e->pos() - m_d->dragStart).manhattanLength() > QApplication::startDragDistance()) {
                m_d->isDraggingKeyframe = true;
            }
        }
    }

    QAbstractItemView::mouseMoveEvent(e);
}

void KisAnimationCurvesView::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_d->panning = false;

        if (m_d->isDraggingKeyframe) {
            QModelIndexList indexes = selectedIndexes();
            int timeOffset = m_d->dragOffset.x() / m_d->horizontalHeader->defaultSectionSize();
            qreal valueOffset = m_d->dragOffset.y() / m_d->verticalHeader->scaleFactor();

            KisAnimationCurvesModel *curvesModel = dynamic_cast<KisAnimationCurvesModel*>(model());
            curvesModel->adjustKeyframes(indexes, timeOffset, valueOffset);

            m_d->isDraggingKeyframe = false;
            m_d->itemDelegate->setSelectedItemVisualOffset(QPointF());
            viewport()->update();
        } else if (m_d->isAdjustingHandle) {
            QModelIndex index = currentIndex();
            int mode = index.data(KisAnimationCurvesModel::TangentsModeRole).toInt();

            m_d->model->beginCommand(kundo2_i18n("Adjust tangent"));

            if (mode == KisKeyframe::Smooth) {
                QPointF leftHandle = m_d->itemDelegate->leftHandle(index, true);
                QPointF rightHandle = m_d->itemDelegate->rightHandle(index, true);

                QPointF leftTangent = m_d->itemDelegate->unscaledTangent(leftHandle);
                QPointF rightTangent = m_d->itemDelegate->unscaledTangent(rightHandle);

                model()->setData(index, leftTangent, KisAnimationCurvesModel::LeftTangentRole);
                model()->setData(index, rightTangent, KisAnimationCurvesModel::RightTangentRole);
            } else {
                if (m_d->adjustedHandle == 0) {
                    QPointF leftHandle = m_d->itemDelegate->leftHandle(index, true);
                    model()->setData(index, m_d->itemDelegate->unscaledTangent(leftHandle), KisAnimationCurvesModel::LeftTangentRole);
                } else {
                    QPointF rightHandle = m_d->itemDelegate->rightHandle(index, true);
                    model()->setData(index, m_d->itemDelegate->unscaledTangent(rightHandle), KisAnimationCurvesModel::RightTangentRole);
                }
            }

            m_d->model->endCommand();

            m_d->isAdjustingHandle = false;
            m_d->itemDelegate->setHandleAdjustment(QPointF(), m_d->adjustedHandle);
        }
    }

    QAbstractItemView::mouseReleaseEvent(e);
}

void KisAnimationCurvesView::scrollContentsBy(int dx, int dy)
{
    m_d->horizontalHeader->setOffset(horizontalScrollBar()->value());
    m_d->verticalHeader->setOffset(verticalScrollBar()->value());

    scrollDirtyRegion(dx, dy);
    viewport()->scroll(dx, dy);
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

    horizontalScrollBar()->setRange(0, m_d->horizontalHeader->length() - viewport()->width());
    updateVerticalRange();

    QAbstractItemView::updateGeometries();
}

void KisAnimationCurvesView::slotRowsChanged(const QModelIndex &parentIndex, int first, int last)
{
    Q_UNUSED(parentIndex);
    Q_UNUSED(first);
    Q_UNUSED(last);

    updateVerticalRange();
    viewport()->update();
}

void KisAnimationCurvesView::slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);
    updateVerticalRange();
    viewport()->update();
}

void KisAnimationCurvesView::slotHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
    Q_UNUSED(orientation);
    Q_UNUSED(first);
    Q_UNUSED(last);
    viewport()->update();
}

void KisAnimationCurvesView::slotHorizontalZoomStarted(qreal staticPoint)
{
    m_d->horizontalZoomStillPointIndex =
        qIsNaN(staticPoint) ? currentIndex().column() : staticPoint;

    const int w = m_d->horizontalHeader->defaultSectionSize();

    m_d->horizontalZoomStillPointOriginalOffset =
        w * m_d->horizontalZoomStillPointIndex -
        horizontalScrollBar()->value();
}

void KisAnimationCurvesView::slotHorizontalZoomLevelChanged(qreal zoomLevel)
{
    if (m_d->horizontalHeader->setZoom(zoomLevel)) {
        const int w = m_d->horizontalHeader->defaultSectionSize();
        horizontalScrollBar()->setValue(w * m_d->horizontalZoomStillPointIndex - m_d->horizontalZoomStillPointOriginalOffset);

        viewport()->update();
    }
}

void KisAnimationCurvesView::slotVerticalZoomStarted(qreal staticPoint)
{
    m_d->verticalZoomStillPoint = qIsNaN(staticPoint) ? 0 : staticPoint;

    const float scale = m_d->verticalHeader->scaleFactor();

    m_d->verticalZoomStillPointOriginalOffset =
        scale * m_d->verticalZoomStillPoint - m_d->verticalHeader->offset();
}

void KisAnimationCurvesView::slotVerticalZoomLevelChanged(qreal zoomLevel)
{
    if (!qFuzzyCompare((float)zoomLevel, m_d->verticalHeader->scaleFactor())) {
        m_d->verticalHeader->setScale(zoomLevel);
        m_d->verticalHeader->setOffset(-zoomLevel * m_d->verticalZoomStillPoint - m_d->verticalZoomStillPointOriginalOffset);
        verticalScrollBar()->setValue(m_d->verticalHeader->offset());

        viewport()->update();
    }
}

void KisAnimationCurvesView::applyConstantMode()
{
    m_d->model->beginCommand(kundo2_i18n("Set interpolation mode"));
    Q_FOREACH(QModelIndex index, selectedIndexes()) {
        m_d->model->setData(index, KisKeyframe::Constant, KisAnimationCurvesModel::InterpolationModeRole);
    }
    m_d->model->endCommand();
}

void KisAnimationCurvesView::applyLinearMode()
{
    m_d->model->beginCommand(kundo2_i18n("Set interpolation mode"));
    Q_FOREACH(QModelIndex index, selectedIndexes()) {
        m_d->model->setData(index, KisKeyframe::Linear, KisAnimationCurvesModel::InterpolationModeRole);
    }
    m_d->model->endCommand();
}

void KisAnimationCurvesView::applyBezierMode()
{
    m_d->model->beginCommand(kundo2_i18n("Set interpolation mode"));
    Q_FOREACH(QModelIndex index, selectedIndexes()) {
        m_d->model->setData(index, KisKeyframe::Bezier, KisAnimationCurvesModel::InterpolationModeRole);
    }
    m_d->model->endCommand();
}

void KisAnimationCurvesView::applySmoothMode()
{
    m_d->model->beginCommand(kundo2_i18n("Set interpolation mode"));
    Q_FOREACH(QModelIndex index, selectedIndexes()) {
        QVector2D leftVisualTangent(m_d->itemDelegate->leftHandle(index, false));
        QVector2D rightVisualTangent(m_d->itemDelegate->rightHandle(index, false));

        if (leftVisualTangent.lengthSquared() > 0 && rightVisualTangent.lengthSquared() > 0) {
            float leftAngle = qAtan2(-leftVisualTangent.y(), -leftVisualTangent.x());
            float rightAngle = qAtan2(rightVisualTangent.y(), rightVisualTangent.x());
            float angle = (leftAngle + rightAngle) / 2;
            QVector2D unit(qCos(angle), qSin(angle));

            leftVisualTangent = -unit * QVector2D(leftVisualTangent).length();
            rightVisualTangent = unit * QVector2D(rightVisualTangent).length();

            QPointF leftTangent = m_d->itemDelegate->unscaledTangent(leftVisualTangent.toPointF());
            QPointF rightTangent = m_d->itemDelegate->unscaledTangent(rightVisualTangent.toPointF());

            model()->setData(index, leftTangent, KisAnimationCurvesModel::LeftTangentRole);
            model()->setData(index, rightTangent, KisAnimationCurvesModel::RightTangentRole);
        }

        model()->setData(index, KisKeyframe::Smooth, KisAnimationCurvesModel::TangentsModeRole);
    }
    m_d->model->endCommand();
}

void KisAnimationCurvesView::applySharpMode()
{
    m_d->model->beginCommand(kundo2_i18n("Set interpolation mode"));
    Q_FOREACH(QModelIndex index, selectedIndexes()) {
        model()->setData(index, KisKeyframe::Sharp, KisAnimationCurvesModel::TangentsModeRole);
    }
    m_d->model->endCommand();
}

void KisAnimationCurvesView::createKeyframe()
{
    QModelIndex active = currentIndex();
    int channel = active.isValid() ? active.row() : 0;

    int time = m_d->model->currentTime();
    QModelIndex index = m_d->model->index(channel, time);

    qreal value = index.data(KisAnimationCurvesModel::ScalarValueRole).toReal();
    m_d->model->setData(index, value, KisAnimationCurvesModel::ScalarValueRole);
}

void KisAnimationCurvesView::removeKeyframes()
{
    m_d->model->removeFrames(selectedIndexes());
}

void KisAnimationCurvesView::zoomToFit()
{
    if (!model()) return;

    qreal minimum, maximum;
    findExtremes(&minimum, &maximum);
    if (minimum == maximum) return;

    qreal zoomLevel = (viewport()->height() - 2 * VERTICAL_PADDING) / (maximum - minimum);
    qreal offset = -VERTICAL_PADDING - zoomLevel * maximum;

    m_d->verticalHeader->setScale(zoomLevel);
    m_d->verticalHeader->setOffset(offset);
    verticalScrollBar()->setValue(offset);
    viewport()->update();
}
