/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisAnimCurvesView.h"

#include <QPaintEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QScrollBar>
#include <qpainter.h>
#include <QtMath>

#include "KisAnimCurvesModel.h"
#include "KisAnimTimelineTimeHeader.h"
#include "KisAnimCurvesValuesHeader.h"
#include "KisAnimCurvesKeyDelegate.h"
#include "KisAnimTimelineColors.h"
#include "kis_scalar_keyframe_channel.h"
#include "kis_zoom_button.h"
#include "kis_custom_modifiers_catcher.h"
#include <kis_painting_tweaks.h>
#include "kis_zoom_scrollbar.h"

struct KisAnimCurvesView::Private
{
    Private()
    {}

    KisAnimCurvesModel *model {0};
    KisAnimTimelineTimeHeader *horizontalHeader {0};
    KisAnimCurvesValuesHeader *verticalHeader {0};
    KisAnimCurvesKeyDelegate *itemDelegate {0};
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

    bool dragPanning {false};
    QPoint panAnchor;

    bool dragZooming {false};
    QPoint zoomAnchor;

    bool deselectIntended {false};
    QModelIndex toDeselect;
};

KisAnimCurvesView::KisAnimCurvesView(QWidget *parent)
    : QAbstractItemView(parent)
    , m_d(new Private())
{
    m_d->horizontalHeader = new KisAnimTimelineTimeHeader(this);

    m_d->verticalHeader = new KisAnimCurvesValuesHeader(this);
    m_d->itemDelegate = new KisAnimCurvesKeyDelegate(m_d->horizontalHeader, m_d->verticalHeader, this);

    m_d->modifiersCatcher = new KisCustomModifiersCatcher(this);
    m_d->modifiersCatcher->addModifier("pan-zoom", Qt::Key_Space);

    setSelectionMode(QAbstractItemView::ExtendedSelection);

    KisZoomableScrollBar* horiZoomableBar = new KisZoomableScrollBar(this);
    setHorizontalScrollBar(horiZoomableBar);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    connect(horiZoomableBar, &KisZoomableScrollBar::valueChanged, [this](qreal value){
        m_d->horizontalHeader->setPixelOffset(value);
        slotUpdateInfiniteFramesCount();
        viewport()->update();
    });

    connect(horiZoomableBar, &KisZoomableScrollBar::sliderReleased, this, &KisAnimCurvesView::slotUpdateHorizontalScrollbarSize);

    connect(horiZoomableBar, &KisZoomableScrollBar::overscroll, [this](qreal overscroll){
        m_d->horizontalHeader->setPixelOffset(m_d->horizontalHeader->offset() + overscroll);
        slotUpdateInfiniteFramesCount();
        slotUpdateHorizontalScrollbarSize();
        viewport()->update();
    });

    connect(horiZoomableBar, &KisZoomableScrollBar::zoom, [this](qreal zoomDelta){
        qreal currentZoomLevel = m_d->horizontalHeader->zoom();
        m_d->horizontalHeader->setZoom(currentZoomLevel + zoomDelta);
        slotUpdateInfiniteFramesCount();
        slotUpdateHorizontalScrollbarSize();
        viewport()->update();
    });


    KisZoomableScrollBar* vertZoomableBar = new KisZoomableScrollBar(this);
    setVerticalScrollBar(vertZoomableBar);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    vertZoomableBar->setWheelOverscrollSensitivity( 0.04f );

    connect(vertZoomableBar, &KisZoomableScrollBar::zoom, [this](qreal zoomDelta){
        const qreal currentZoomLevel = m_d->verticalHeader->scale();
        m_d->verticalHeader->setScale(currentZoomLevel + zoomDelta / m_d->verticalHeader->step());
    });

    connect(vertZoomableBar, &KisZoomableScrollBar::overscroll, [this](qreal overscroll){
       qreal currentOffset = m_d->verticalHeader->valueOffset();
       m_d->verticalHeader->setValueOffset(currentOffset - overscroll * m_d->verticalHeader->step() * 0.25);
    });
    
    connect(m_d->verticalHeader, &KisAnimCurvesValuesHeader::scaleChanged, [this](qreal){
        viewport()->update();
    });
    
    connect(m_d->verticalHeader, &KisAnimCurvesValuesHeader::valueOffsetChanged, [this](qreal){
        viewport()->update();
    });

    QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(this);
    if (scroller){
        connect(scroller, SIGNAL(stateChanged(QScroller::State)),
                this, SLOT(slotScrollerStateChanged(QScroller::State)));

        QScrollerProperties props = scroller->scrollerProperties();
        props.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
        props.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
        scroller->setScrollerProperties(props);
    }
}

KisAnimCurvesView::~KisAnimCurvesView()
{}

void KisAnimCurvesView::setModel(QAbstractItemModel *model)
{
    m_d->model = dynamic_cast<KisAnimCurvesModel*>(model);

    QAbstractItemView::setModel(model);
    m_d->horizontalHeader->setModel(model);

    connect(model, &QAbstractItemModel::rowsInserted,
            this, &KisAnimCurvesView::slotRowsChanged);

    connect(model, &QAbstractItemModel::rowsRemoved,
            this, &KisAnimCurvesView::slotRowsChanged);

    connect(model, &QAbstractItemModel::dataChanged,
            this, &KisAnimCurvesView::slotDataChanged);

    connect(model, &QAbstractItemModel::headerDataChanged,
            this, &KisAnimCurvesView::slotHeaderDataChanged);

    connect(selectionModel(), &QItemSelectionModel::selectionChanged,
        [this](const QItemSelection& newSelection, const QItemSelection& /*oldSelection*/) {
            if (newSelection.count() == 0) {
                activeDataChanged(QModelIndex());
            } else {
                activeDataChanged(selectionModel()->currentIndex());
            }
        });

    connect(m_d->model, &KisAnimCurvesModel::dataAdded,
            this, &KisAnimCurvesView::slotDataAdded);
}

QRect KisAnimCurvesView::visualRect(const QModelIndex &index) const
{
    return m_d->itemDelegate->itemRect(index);
}

void KisAnimCurvesView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{
    // TODO
    Q_UNUSED(index);
    Q_UNUSED(hint);
}

QModelIndex KisAnimCurvesView::indexAt(const QPoint &point) const
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

void KisAnimCurvesView::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());

    QRect rect = event->rect();
    rect.translate(dirtyRegionOffset());

    int firstFrame = m_d->horizontalHeader->logicalIndexAt(rect.left());
    int lastFrame = m_d->horizontalHeader->logicalIndexAt(rect.right());
    if (lastFrame == -1) lastFrame = model()->columnCount();

    paintGrid(painter);
    paintCurves(painter, firstFrame, lastFrame);
    paintKeyframes(painter, firstFrame, lastFrame);
}

void KisAnimCurvesView::paintGrid(QPainter &painter)
{
    const QColor backgroundColor = qApp->palette().window().color();
    const QColor textColor = qApp->palette().text().color();
    const QColor lineColor = KisPaintingTweaks::blendColors(textColor, backgroundColor, 0.1);
    const QColor zeroLineColor = qApp->palette().highlight().color();
    const QColor activeFrameColor = KisAnimTimelineColors::instance()->headerActive().color();

    // Draw vertical lines..
    const int visibleFrames = m_d->horizontalHeader->estimateLastVisibleColumn() - m_d->horizontalHeader->estimateFirstVisibleColumn() + 1;
    const int firstVisibleFrame = qMax( m_d->horizontalHeader->estimateFirstVisibleColumn() - 1, 0);
    for (int time = 0; time <= visibleFrames; time++) {
        QVariant data = m_d->model->headerData(firstVisibleFrame + time, Qt::Horizontal, KisTimeBasedItemModel::ActiveFrameRole);
        const bool activeFrame = data.isValid() && data.toBool();

        data = m_d->model->headerData(firstVisibleFrame + time, Qt::Horizontal, KisTimeBasedItemModel::WithinClipRange);
        const bool withinClipRange = data.isValid() && data.toBool();

        const int offsetHori = m_d->horizontalHeader ? m_d->horizontalHeader->offset() : 0;
        const int stepHori = m_d->horizontalHeader->defaultSectionSize();
        const int xPosition = stepHori * (firstVisibleFrame + time) - offsetHori;

        QRect frameRect = QRect(xPosition, -10, stepHori, 9999);

        const QPoint top = frameRect.topLeft() + 0.5 * (frameRect.topRight() - frameRect.topLeft());
        const QPoint bottom = frameRect.bottomLeft() + 0.5 * (frameRect.bottomRight() - frameRect.bottomLeft());

        QColor fadedLineColor = lineColor;
        fadedLineColor.setAlphaF(0.33);

        QColor finalColor = withinClipRange ? lineColor : fadedLineColor;
        finalColor = activeFrame ? activeFrameColor : finalColor;

        painter.setPen(finalColor);
        painter.drawLine(top, bottom);
    }

    // Draw horizontal lines..
    const int visibleSteps = m_d->verticalHeader->visibleValueDifference() / m_d->verticalHeader->step();
    const qreal stepAmount = m_d->verticalHeader->step();
    for (int step = 0; step <= visibleSteps; step++) {
        const qreal value = m_d->verticalHeader->firstVisibleStep() + stepAmount * step;
        const int CORRECTION = -1;
        int yPosition = m_d->verticalHeader->valueToWidget(value);

        QRect frameRect = QRect(-10, yPosition + CORRECTION, 9999, 1);

        const QPoint left = frameRect.topLeft();
        const QPoint right = frameRect.topRight();

        painter.setPen(value == 0 ? zeroLineColor : lineColor);
        painter.drawLine(left, right);
    }
}

void KisAnimCurvesView::paintCurves(QPainter &painter, int firstFrame, int lastFrame)
{
    int channels = model()->rowCount();
    for (int channel = 0; channel < channels; channel++) {
        QModelIndex index0 = model()->index(channel, 0);

        if (!isIndexHidden(index0)) {
            QColor color = index0.data(KisAnimCurvesModel::CurveColorRole).value<QColor>();
            painter.setPen(QPen(color, 1));
            painter.setRenderHint(QPainter::Antialiasing);

            paintCurve(channel, firstFrame, lastFrame, painter);
        }
    }
}

void KisAnimCurvesView::paintCurve(int channel, int firstFrame, int lastFrame, QPainter &painter)
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
        int interpolationMode = index.data(KisAnimCurvesModel::InterpolationModeRole).toInt();

        int time = (m_d->isDraggingKeyframe && selectionModel()->isSelected(index)) ? index.column() + selectionOffset : index.column();
        index = findNextKeyframeIndex(channel, time, selectionOffset, false);
        if (!index.isValid()) return;

        bool active = (index == currentIndex());
        QPointF nextKeyPos = m_d->itemDelegate->nodeCenter(index, selectionModel()->isSelected(index));
        QPointF leftTangent = m_d->itemDelegate->leftHandle(index, active);

        if (interpolationMode == KisScalarKeyframe::Constant) {
            painter.drawLine(previousKeyPos, QPointF(nextKeyPos.x(), previousKeyPos.y()));
        } else if (interpolationMode == KisScalarKeyframe::Linear) {
            painter.drawLine(previousKeyPos, nextKeyPos);
        } else {
            QVariant limitData = m_d->model->data(m_d->model->index(channel, index.column()), KisAnimCurvesModel::ChannelLimits);
            paintCurveSegment(painter, previousKeyPos, rightTangent, leftTangent, nextKeyPos, limitData);
        }

        previousKeyPos = nextKeyPos;
        rightTangent = m_d->itemDelegate->rightHandle(index, active);
    }
}

void KisAnimCurvesView::paintCurveSegment(QPainter &painter, QPointF pos1, QPointF rightTangent, QPointF leftTangent, QPointF pos2, QVariant limitData) {
    const int steps = 32;
    QPointF previousPos;

    for (int step = 0; step <= steps; step++) {
        qreal t = 1.0 * step / steps;

        QPointF nextPos = KisScalarKeyframeChannel::interpolate(pos1, rightTangent, leftTangent, pos2, t);
        if (limitData.isValid()) {
            ChannelLimitsMetatype limits = limitData.value<ChannelLimitsMetatype>();

            nextPos.setY(qMin(nextPos.y(), m_d->verticalHeader->valueToWidget(limits.first)));
            nextPos.setY(qMax(nextPos.y(), m_d->verticalHeader->valueToWidget(limits.second)));
        }

        if (step > 0) {
            painter.drawLine(previousPos, nextPos);
        }

        previousPos = nextPos;
    }
}

void KisAnimCurvesView::paintKeyframes(QPainter &painter, int firstFrame, int lastFrame)
{
    int channels = model()->rowCount();
    for (int channel = 0; channel < channels; channel++) {
        for (int time=firstFrame; time <= lastFrame; time++) {
            QModelIndex index = model()->index(channel, time);
            bool keyframeExists = model()->data(index, KisAnimCurvesModel::SpecialKeyframeExists).toReal();

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

QModelIndex KisAnimCurvesView::findNextKeyframeIndex(int channel, int time, int selectionOffset, bool backward)
{
    KisAnimCurvesModel::ItemDataRole role =
            backward ? KisAnimCurvesModel::PreviousKeyframeTime : KisAnimCurvesModel::NextKeyframeTime;
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

void KisAnimCurvesView::findExtremes(qreal *minimum, qreal *maximum)
{
    if (!model()) return;

    qreal min = qInf();
    qreal max = -qInf();

    int curveCount = model()->rowCount();
    for (int curveIndex = 0; curveIndex < curveCount; curveIndex++) {
        QModelIndex index = model()->index(curveIndex, 0);
        if (isIndexHidden(index)) continue;

        QVariant nextTime;
        do {
            qreal value = index.data(KisAnimCurvesModel::ScalarValueRole).toReal();

            if (value < min) min = value;
            if (value > max) max = value;

            const int MAX_NUM_TANGENTS = 2;
            for (int i = 0; i < MAX_NUM_TANGENTS; i++)  {
                QVariant tangent = index.data(KisAnimCurvesModel::LeftTangentRole + i);
                if (!tangent.isValid())
                    continue;

                QPointF tangentPointF = tangent.toPointF();
                if (value + tangentPointF.y() < min) min = value + tangentPointF.y();
                if (value + tangentPointF.y() > max) max = value + tangentPointF.y();
            }

            nextTime = index.data(KisAnimCurvesModel::NextKeyframeTime);
            if (nextTime.isValid()) index = model()->index(curveIndex, nextTime.toInt());
        } while (nextTime.isValid());
    }

    if (qIsFinite(min)) *minimum = min;
    if (qIsFinite(max)) *maximum = max;
}

QModelIndex KisAnimCurvesView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    // TODO
    Q_UNUSED(cursorAction);
    Q_UNUSED(modifiers);
    return QModelIndex();
}

int KisAnimCurvesView::horizontalOffset() const
{
    return m_d->horizontalHeader->offset();
}

int KisAnimCurvesView::verticalOffset() const
{
    return m_d->verticalHeader->valueOffset();
}

bool KisAnimCurvesView::isIndexHidden(const QModelIndex &index) const
{
    return !index.data(KisAnimCurvesModel::CurveVisibleRole).toBool();
}

void KisAnimCurvesView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags)
{
    int timeFrom = m_d->horizontalHeader->logicalIndexAt(rect.left());
    int timeTo = m_d->horizontalHeader->logicalIndexAt(rect.right());

    QItemSelection selection;

    int rows = model()->rowCount();
    for (int row=0; row < rows; row++) {
        for (int time = timeFrom; time <= timeTo; time++) {

            QModelIndex index = model()->index(row, time);

            if (isIndexHidden(index)) continue;

            if (index.data(KisTimeBasedItemModel::SpecialKeyframeExists).toBool()) {
                QRect itemRect = m_d->itemDelegate->itemRect(index);

                if (itemRect.intersects(rect)) {
                    selection.select(index, index);
                }
            }
        }
    }

    if (!selection.contains(selectionModel()->currentIndex()) && selection.size() > 0) {
        selectionModel()->setCurrentIndex(selection.first().topLeft(), flags);
    }

    selectionModel()->select(selection, flags);
    activated(selectionModel()->currentIndex());
}

QRegion KisAnimCurvesView::visualRegionForSelection(const QItemSelection &selection) const
{
    QRegion region;

    Q_FOREACH(QModelIndex index, selection.indexes()) {
        region += m_d->itemDelegate->visualRect(index);
    }

    return region;
}

void KisAnimCurvesView::mousePressEvent(QMouseEvent *e)
{
    if (m_d->modifiersCatcher->modifierPressed("pan-zoom")) {
        if (e->button() == Qt::LeftButton) { // PAN
            m_d->dragPanning = true;
            m_d->panAnchor = e->pos();

        } else if (e->button() == Qt::RightButton) { // ZOOM
            m_d->dragZooming = true;
            m_d->zoomAnchor = e->pos();
        }
    } else if (e->button() == Qt::LeftButton) { // SELECT
        m_d->dragStart = e->pos();

        const int handleClickRadius = 16;

        Q_FOREACH(QModelIndex index, selectedIndexes()) {

            if (isIndexHidden(index)) continue;

            QPointF center = m_d->itemDelegate->nodeCenter(index, false);
            bool hasLeftHandle = m_d->itemDelegate->hasHandle(index, 0);
            bool hasRightHandle = m_d->itemDelegate->hasHandle(index, 1);

            QPointF leftHandle = center + m_d->itemDelegate->leftHandle(index, false);
            QPointF rightHandle = center + m_d->itemDelegate->rightHandle(index, false);

            if (hasLeftHandle && (e->localPos() - leftHandle).manhattanLength() < handleClickRadius) {
                m_d->isAdjustingHandle = true;
                m_d->adjustedHandle = 0;
                setCurrentIndex(index);
                continue;
            } else if (hasRightHandle && (e->localPos() - rightHandle).manhattanLength() < handleClickRadius) {
                m_d->isAdjustingHandle = true;
                m_d->adjustedHandle = 1;
                setCurrentIndex(index);
                continue;
            }
        }

    }

    QModelIndex clickedIndex = indexAt(e->pos());
    if(indexHasKey(clickedIndex)) {
        if ((e->modifiers() & Qt::ShiftModifier) == 0 && selectionModel()->currentIndex() != clickedIndex) {
            clearSelection();
        }

        if (clickedIndex == selectionModel()->currentIndex() && selectionModel()->hasSelection()) {
            m_d->deselectIntended = true;
            m_d->toDeselect = clickedIndex;
        } else {
            QModelIndex prevCurrent = selectionModel()->currentIndex();
            selectionModel()->select(clickedIndex, QItemSelectionModel::Select);
            selectionModel()->setCurrentIndex(clickedIndex, QItemSelectionModel::NoUpdate);
            emit currentChanged(clickedIndex, prevCurrent);
        }

        emit clicked(clickedIndex);
        emit activeDataChanged(selectionModel()->currentIndex());
    } else {
        QAbstractItemView::mousePressEvent(e);
    }
}


void KisAnimCurvesView::mouseDoubleClickEvent(QMouseEvent *e)
{
    QModelIndex clicked = indexAt(e->pos());

    if(clicked.isValid() && indexHasKey(clicked)) {
        selectionModel()->clear();
        bool firstSelection = true;
        if (e->modifiers() & Qt::AltModifier) {
            for (int column = 0; column <= model()->columnCount(); column++) {
                QModelIndex toSelect = model()->index(clicked.row(), column);
                const bool hasSpecial = toSelect.data(KisTimeBasedItemModel::SpecialKeyframeExists).toBool();
                if (toSelect.isValid() && hasSpecial) {
                    selectionModel()->select(toSelect, firstSelection ? QItemSelectionModel::SelectCurrent : QItemSelectionModel::Select);
                    firstSelection = false;
                }
            }
        } else {
            for (int row = 0; row <= model()->rowCount(); row++) {
                QModelIndex toSelect = model()->index(row, clicked.column());
                const bool hasSpecial = toSelect.data(KisTimeBasedItemModel::SpecialKeyframeExists).toBool();
                if (toSelect.isValid() && hasSpecial) {
                    selectionModel()->select(toSelect, firstSelection ? QItemSelectionModel::SelectCurrent : QItemSelectionModel::Select);
                    firstSelection = false;
                }
            }
        }

        QModelIndex oldCurrent = selectionModel()->currentIndex();
        selectionModel()->setCurrentIndex(clicked, QItemSelectionModel::NoUpdate);
        currentChanged(clicked, oldCurrent);
    } else {
        QAbstractItemView::mouseDoubleClickEvent(e);
    }
}

void KisAnimCurvesView::mouseMoveEvent(QMouseEvent *e)
{
    if (m_d->modifiersCatcher->modifierPressed("pan-zoom")) {
        if (m_d->dragPanning) {
            const int timelineScrubAmnt = m_d->panAnchor.x() - e->pos().x();
            const qreal valueScrubAmnt = m_d->verticalHeader->pixelsToValueOffset(m_d->panAnchor.y() -  e->pos().y());

            slotUpdateInfiniteFramesCount();
            slotUpdateHorizontalScrollbarSize();
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() + timelineScrubAmnt);

            m_d->verticalHeader->setValueOffset(m_d->verticalHeader->valueOffset() + valueScrubAmnt);

            m_d->panAnchor = e->pos();
            viewport()->update();
            return;
        }

        if (m_d->dragZooming) {
            const qreal zoomScale = 50.0f;
            const qreal updown = ((m_d->zoomAnchor.y() - e->pos().y())) / zoomScale;
            const qreal leftright = (m_d->zoomAnchor.x() - e->pos().x()) / zoomScale;

            changeZoom(Qt::Vertical, updown);
            changeZoom(Qt::Horizontal, leftright * -0.5);

            m_d->zoomAnchor = e->pos();
            viewport()->update();
            return;
        }

    } else if (e->buttons() & Qt::LeftButton) {

        m_d->dragOffset = e->pos() - m_d->dragStart;

        if (m_d->isAdjustingHandle) {
            m_d->itemDelegate->setHandleAdjustment(m_d->dragOffset, m_d->adjustedHandle);
            viewport()->update();
            return;
        } else if (m_d->isDraggingKeyframe) {
            const bool axisSnap = (e->modifiers() & Qt::ShiftModifier);
            m_d->itemDelegate->setSelectedItemVisualOffset(m_d->dragOffset, axisSnap);
            viewport()->update();
            return;
        } else if (selectionModel()->hasSelection()) {
            if ((e->pos() - m_d->dragStart).manhattanLength() > QApplication::startDragDistance()) {
                m_d->isDraggingKeyframe = true;
            }
        }
    } else {
        QAbstractItemView::mouseMoveEvent(e);
    }
}

void KisAnimCurvesView::mouseReleaseEvent(QMouseEvent *e)
{

    if (e->button() == Qt::LeftButton) {
        m_d->dragPanning = false;
        m_d->dragZooming = false;

        if (m_d->isDraggingKeyframe) {
            const QModelIndexList indices = selectedIndexes();
            const QPointF offset = qAbs(m_d->dragOffset.y()) > qAbs(m_d->dragOffset.x()) ? QPointF(0.0f, m_d->dragOffset.y()) : QPointF(m_d->dragOffset.x(), 0.0f);
            const int timeOffset = qRound( qreal(offset.x()) / m_d->horizontalHeader->defaultSectionSize() );
            const qreal valueOffset = m_d->verticalHeader->pixelsToValueOffset(offset.y());

            KisAnimCurvesModel *curvesModel = dynamic_cast<KisAnimCurvesModel*>(model());
            curvesModel->adjustKeyframes(indices, timeOffset, valueOffset);

            //Adjust selection to match new keyframe adjustment.
            Q_FOREACH(const QModelIndex& index, indices) {
                const bool wasCurrent = (index == selectionModel()->currentIndex());
                selectionModel()->select(index, QItemSelectionModel::Deselect);
                const QModelIndex newIndex = m_d->model->index(index.row(), index.column() + timeOffset);
                if (wasCurrent) {
                    selectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::SelectCurrent);
                } else {
                    selectionModel()->select(newIndex, QItemSelectionModel::Select);
                }
            }

            m_d->isDraggingKeyframe = false;
            m_d->itemDelegate->setSelectedItemVisualOffset(QPointF());
            viewport()->update();
        } else if (m_d->isAdjustingHandle) {
            QModelIndex index = currentIndex();
            int mode = index.data(KisAnimCurvesModel::TangentsModeRole).toInt();

            m_d->model->beginCommand(kundo2_i18n("Adjust tangent"));

            if (mode == KisScalarKeyframe::Smooth) {
                QPointF leftHandle = m_d->itemDelegate->leftHandle(index, true);
                QPointF rightHandle = m_d->itemDelegate->rightHandle(index, true);

                QPointF leftTangent = m_d->itemDelegate->unscaledTangent(leftHandle);
                QPointF rightTangent = m_d->itemDelegate->unscaledTangent(rightHandle);

                model()->setData(index, leftTangent, KisAnimCurvesModel::LeftTangentRole);
                model()->setData(index, rightTangent, KisAnimCurvesModel::RightTangentRole);
            } else {
                if (m_d->adjustedHandle == 0) {
                    QPointF leftHandle = m_d->itemDelegate->leftHandle(index, true);
                    model()->setData(index, m_d->itemDelegate->unscaledTangent(leftHandle), KisAnimCurvesModel::LeftTangentRole);
                } else {
                    QPointF rightHandle = m_d->itemDelegate->rightHandle(index, true);
                    model()->setData(index, m_d->itemDelegate->unscaledTangent(rightHandle), KisAnimCurvesModel::RightTangentRole);
                }
            }

            m_d->model->endCommand();

            m_d->isAdjustingHandle = false;
            m_d->itemDelegate->setHandleAdjustment(QPointF(), m_d->adjustedHandle);
        } else {

            if (m_d->deselectIntended){
                selectionModel()->select(m_d->toDeselect, QItemSelectionModel::Deselect);
            }

        }

        m_d->deselectIntended = false;
        m_d->toDeselect = QModelIndex();
    }

    QAbstractItemView::mouseReleaseEvent(e);
}

void KisAnimCurvesView::scrollContentsBy(int dx, int dy)
{
    scrollDirtyRegion(dx, dy);
    viewport()->scroll(dx, dy);
    viewport()->update();
}

bool KisAnimCurvesView::indexHasKey(const QModelIndex &index)
{
    const QVariant data = m_d->model->data(index, KisAnimCurvesModel::SpecialKeyframeExists);
    return data.isValid() && data.toBool();
}

void KisAnimCurvesView::updateGeometries()
{
    int topMargin = qMax(m_d->horizontalHeader->minimumHeight(),
                         m_d->horizontalHeader->sizeHint().height());

    int leftMargin = m_d->verticalHeader->sizeHint().width();

    setViewportMargins(leftMargin, topMargin, 0, 0);

    QRect viewRect = viewport()->geometry();
    m_d->horizontalHeader->setGeometry(leftMargin, 0, viewRect.width(), topMargin);
    m_d->verticalHeader->setGeometry(0, topMargin, leftMargin, viewRect.height());
    if (m_d->model) {
        slotUpdateInfiniteFramesCount();
    }

    QAbstractItemView::updateGeometries();
}

void KisAnimCurvesView::slotRowsChanged(const QModelIndex &parentIndex, int first, int last)
{
    Q_UNUSED(parentIndex);
    Q_UNUSED(first);
    Q_UNUSED(last);
    zoomToFitChannel();
    viewport()->update();
}

void KisAnimCurvesView::slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);

    viewport()->update();

    // this forces the horizontal ruler to refresh. Repaint() doesn't do it for some reason
    // If you remove this, scrubbing the timeline will probably stop updating the indicator
    m_d->horizontalHeader->resize(m_d->horizontalHeader->width()-1, m_d->horizontalHeader->height());
    m_d->horizontalHeader->resize(m_d->horizontalHeader->width()+1, m_d->horizontalHeader->height());

    if (selectionModel()->selection().count() != 0 &&
        selectionModel()->currentIndex().isValid()) {
        emit activeDataChanged(selectionModel()->currentIndex());
    }
}

void KisAnimCurvesView::slotDataAdded(const QModelIndex &index)
{
    const qreal lastVisibleValue = m_d->verticalHeader->visibleValueMax();
    const qreal firstVisibleValue = m_d->verticalHeader->visibleValueMin();

    qreal value = index.data(KisAnimCurvesModel::ScalarValueRole).toReal();
    if ( value < firstVisibleValue || value > lastVisibleValue) {
        qreal min, max;
        findExtremes(&min, &max);
        qreal padding = (max - min) * 0.1;
        m_d->verticalHeader->zoomToFitRange(min - padding, max + padding);
        viewport()->update();
    }

    selectionModel()->clear();
    selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
}

void KisAnimCurvesView::slotHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
    Q_UNUSED(orientation);
    Q_UNUSED(first);
    Q_UNUSED(last);
    viewport()->update();
}

void KisAnimCurvesView::slotUpdateInfiniteFramesCount()
{
    if (m_d->model) {
        const int lastColumn = m_d->horizontalHeader->estimateLastVisibleColumn();
        m_d->model->setLastVisibleFrame(lastColumn);
    }
}

void KisAnimCurvesView::slotUpdateHorizontalScrollbarSize()
{
    if (m_d->model) {
        const int lastColumn = qMax( m_d->horizontalHeader->estimateLastVisibleColumn(), m_d->model->columnCount());
        const int numberOfColumnsOnScreen = lastColumn - m_d->horizontalHeader->estimateFirstVisibleColumn();
        const int overallSize = lastColumn * m_d->horizontalHeader->defaultSectionSize();
        const int pageStep = overallSize * (qreal(numberOfColumnsOnScreen) / lastColumn);
        horizontalScrollBar()->setRange(0, overallSize + pageStep);
        horizontalScrollBar()->setPageStep(pageStep);
    }
}

void KisAnimCurvesView::applyConstantMode()
{
    m_d->model->beginCommand(kundo2_i18n("Set interpolation mode"));
    Q_FOREACH(QModelIndex index, selectedIndexes()) {
        m_d->model->setData(index, KisScalarKeyframe::Constant, KisAnimCurvesModel::InterpolationModeRole);
    }
    m_d->model->endCommand();
}

void KisAnimCurvesView::applyLinearMode()
{
    m_d->model->beginCommand(kundo2_i18n("Set interpolation mode"));
    Q_FOREACH(QModelIndex index, selectedIndexes()) {
        m_d->model->setData(index, KisScalarKeyframe::Linear, KisAnimCurvesModel::InterpolationModeRole);
    }
    m_d->model->endCommand();
}

void KisAnimCurvesView::applyBezierMode()
{
    m_d->model->beginCommand(kundo2_i18n("Set interpolation mode"));
    Q_FOREACH(QModelIndex index, selectedIndexes()) {
        m_d->model->setData(index, KisScalarKeyframe::Bezier, KisAnimCurvesModel::InterpolationModeRole);
    }
    m_d->model->endCommand();
}

void KisAnimCurvesView::applySmoothMode()
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

            model()->setData(index, leftTangent, KisAnimCurvesModel::LeftTangentRole);
            model()->setData(index, rightTangent, KisAnimCurvesModel::RightTangentRole);
        }

        model()->setData(index, KisScalarKeyframe::Smooth, KisAnimCurvesModel::TangentsModeRole);
    }
    m_d->model->endCommand();
}

void KisAnimCurvesView::applySharpMode()
{
    m_d->model->beginCommand(kundo2_i18n("Set interpolation mode"));
    Q_FOREACH(QModelIndex index, selectedIndexes()) {
        model()->setData(index, KisScalarKeyframe::Sharp, KisAnimCurvesModel::TangentsModeRole);
    }
    m_d->model->endCommand();
}

void KisAnimCurvesView::createKeyframe()
{
    QModelIndex active = currentIndex();
    int channel = active.isValid() ? active.row() : 0;

    int time = m_d->model->currentTime();
    QModelIndex index = m_d->model->index(channel, time);

    qreal value = index.data(KisAnimCurvesModel::ScalarValueRole).toReal();
    m_d->model->setData(index, value, KisAnimCurvesModel::ScalarValueRole);
}

void KisAnimCurvesView::removeKeyframes()
{
    m_d->model->removeFrames(selectedIndexes());
}


void KisAnimCurvesView::zoomToFitCurve()
{
    if (!model()) return;

    qreal min, max;
    findExtremes(&min, &max);

    const qreal padding = (min != max) ? (max - min) * 0.1 : 10.0f;
    m_d->verticalHeader->zoomToFitRange(min - padding, max + padding);
    viewport()->update();
}

void KisAnimCurvesView::zoomToFitChannel()
{
    if (!model()) return;

    const int channels = model()->rowCount();

    qreal min = 0;
    qreal max = min;

    for (int channel = 0; channel < channels; channel++) {
        QModelIndex index = m_d->model->index(channel, 0);
        QVariant variant = m_d->model->data(index, KisAnimCurvesModel::ChannelLimits);

        if (!variant.isValid())
            continue;

        ChannelLimitsMetatype limits = variant.value<ChannelLimitsMetatype>();
        min = qMin(limits.first, min);
        max = qMax(limits.second, max);
    }

    if (min == max)
    {
        zoomToFitCurve();
        return;
    }

    const qreal padding = (max - min) * 0.1;
    m_d->verticalHeader->zoomToFitRange(min - padding, max + padding);
    viewport()->update();
}

void KisAnimCurvesView::changeZoom(Qt::Orientation orientation, qreal zoomDelta)
{
    if (orientation == Qt::Horizontal) {
        m_d->horizontalHeader->setZoom( m_d->horizontalHeader->zoom() + zoomDelta);
        slotUpdateInfiniteFramesCount();
    } else {
        const qreal currentZoomLevel = m_d->verticalHeader->scale();
        m_d->verticalHeader->setScale(currentZoomLevel + zoomDelta / m_d->verticalHeader->step());
    }
    viewport()->update();
}
