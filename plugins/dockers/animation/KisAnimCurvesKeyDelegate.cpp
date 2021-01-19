/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAnimCurvesKeyDelegate.h"

#include <QPainter>
#include <QApplication>
#include <QVector2D>

#include "KisAnimCurvesModel.h"
#include "kis_scalar_keyframe_channel.h"
#include "kis_keyframe.h"

const int NODE_RENDER_RADIUS = 4;
const int NODE_UI_RADIUS = 8;

struct KisAnimCurvesKeyDelegate::Private
{
    Private(const KisAnimTimelineTimeHeader *horizontalRuler, const KisAnimCurvesValuesHeader *verticalRuler)
        : horizontalRuler(horizontalRuler)
        , verticalRuler(verticalRuler)
    {}
    const KisAnimTimelineTimeHeader *horizontalRuler;
    const KisAnimCurvesValuesHeader *verticalRuler;
    QPointF selectionOffset;

    int adjustedHandle;
    QPointF handleAdjustment;
};

KisAnimCurvesKeyDelegate::KisAnimCurvesKeyDelegate(const KisAnimTimelineTimeHeader *horizontalRuler, const KisAnimCurvesValuesHeader *verticalRuler, QObject *parent)
    : QAbstractItemDelegate(parent)
    , m_d(new Private(horizontalRuler, verticalRuler))
{
}

KisAnimCurvesKeyDelegate::~KisAnimCurvesKeyDelegate()
{}


void KisAnimCurvesKeyDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    bool selected = option.state & QStyle::State_Selected;
    bool active = option.state & QStyle::State_HasFocus;
    QPointF center = nodeCenter(index, selected);

    QColor color;
    QColor bgColor = qApp->palette().color(QPalette::Window);
    if (selected) {
        color = (bgColor.value() > 128) ? Qt::black : Qt::white;
    } else {
        color = index.data(KisAnimCurvesModel::CurveColorRole).value<QColor>();
    }

    painter->setPen(QPen(color, 0));
    painter->setBrush(color);
    painter->drawEllipse(center, NODE_RENDER_RADIUS, NODE_RENDER_RADIUS);

    if (selected) {
        painter->setPen(QPen(color, 1));
        painter->setBrush(bgColor);

        if (hasHandle(index, 0)) {
            QPointF leftTangent = leftHandle(index, active);
            paintHandle(painter, center, leftTangent);
        }

        if (hasHandle(index, 1)) {
            QPointF rightTangent = rightHandle(index, active);
            paintHandle(painter, center, rightTangent);
        }
    }
}

QSize KisAnimCurvesKeyDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(2*NODE_UI_RADIUS, 2*NODE_UI_RADIUS);
}

QPointF KisAnimCurvesKeyDelegate::nodeCenter(const QModelIndex index, bool selected) const
{
    int section = m_d->horizontalRuler->logicalIndex(index.column());
    int x = m_d->horizontalRuler->sectionViewportPosition(section)
            + (m_d->horizontalRuler->sectionSize(section) / 2);

    float value = index.data(KisAnimCurvesModel::ScalarValueRole).toReal();
    float y = m_d->verticalRuler->valueToWidget(value);

    QPointF center = QPointF(x, y);
    if (selected) center += m_d->selectionOffset;
    return center;
}

bool KisAnimCurvesKeyDelegate::hasHandle(const QModelIndex index, int handle) const
{
    QModelIndex interpolatedIndex;

    if (handle == 0) {
        // Left handle: use previous keyframe's interpolation mode

        QVariant previous = index.data(KisAnimCurvesModel::PreviousKeyframeTime);
        if (!previous.isValid()) return false;

        interpolatedIndex = index.model()->index(index.row(), previous.toInt());
    } else {
        interpolatedIndex = index;
    }

    return (interpolatedIndex.data(KisAnimCurvesModel::InterpolationModeRole).toInt() == KisScalarKeyframe::Bezier);
}

QPointF KisAnimCurvesKeyDelegate::leftHandle(const QModelIndex index, bool active) const
{
    return handlePosition(index, active, 0);
}

QPointF KisAnimCurvesKeyDelegate::rightHandle(const QModelIndex index, bool active) const
{
    return handlePosition(index, active, 1);
}

QPointF KisAnimCurvesKeyDelegate::handlePosition(const QModelIndex index, bool active, int handle) const
{
    int role = (handle == 0) ? KisAnimCurvesModel::LeftTangentRole : KisAnimCurvesModel::RightTangentRole;
    QPointF tangent = index.data(role).toPointF();

    float x = tangent.x() * m_d->horizontalRuler->defaultSectionSize();
    float y = m_d->verticalRuler->valueToPixelOffset(tangent.y());
    QPointF handlePos = QPointF(x, y);

    if (active && !m_d->handleAdjustment.isNull()) {
        if (handle == m_d->adjustedHandle) {
            handlePos += m_d->handleAdjustment;
            if ((handle == 0 && handlePos.x() > 0) ||
                (handle == 1 && handlePos.x() < 0)) {
                handlePos.setX(0);
            }
        } else if (index.data(KisAnimCurvesModel::TangentsModeRole).toInt() == KisScalarKeyframe::Smooth) {
            qreal length = QVector2D(handlePos).length();
            QVector2D opposite(handlePosition(index, active, 1-handle));
            handlePos = (-length * opposite.normalized()).toPointF();
        }
    }

    return handlePos;
}

void KisAnimCurvesKeyDelegate::setSelectedItemVisualOffset(QPointF offset, bool axisSnap)
{
    if (axisSnap) {
        offset = qAbs(offset.y()) >= qAbs(offset.x()) ? QPointF(0.0f, offset.y()) : QPointF(offset.x(), 0.0f);
    }
    m_d->selectionOffset = offset;
}

void KisAnimCurvesKeyDelegate::setHandleAdjustment(QPointF offset, int handle)
{
    m_d->adjustedHandle = handle;
    m_d->handleAdjustment = offset;
}

QPointF KisAnimCurvesKeyDelegate::unscaledTangent(QPointF handlePosition) const
{
    qreal x = handlePosition.x() / m_d->horizontalRuler->defaultSectionSize();
    qreal y = m_d->verticalRuler->pixelsToValueOffset(handlePosition.y());

    return QPointF(x, y);
}

void KisAnimCurvesKeyDelegate::paintHandle(QPainter *painter, QPointF nodePos, QPointF tangent) const
{
    QPointF handlePos = nodePos + tangent;

    painter->drawLine(nodePos, handlePos);
    painter->drawEllipse(handlePos, NODE_RENDER_RADIUS, NODE_RENDER_RADIUS);
}

QRect KisAnimCurvesKeyDelegate::itemRect(const QModelIndex index) const
{
    QPointF center = nodeCenter(index, false);

    return QRect(center.x() - NODE_UI_RADIUS, center.y() - NODE_UI_RADIUS, 2*NODE_UI_RADIUS, 2*NODE_UI_RADIUS);
}

QRect KisAnimCurvesKeyDelegate::frameRect(const QModelIndex index) const
{
    int section = m_d->horizontalRuler->logicalIndex(index.column());
    int x = m_d->horizontalRuler->sectionViewportPosition(section);
    int xSize = m_d->horizontalRuler->sectionSize(section);

    float value = index.data(KisAnimCurvesModel::ScalarValueRole).toReal();
    float y = m_d->verticalRuler->valueToWidget(value);
    int ySize = m_d->verticalRuler->height();

   return QRect(x, y, xSize, ySize);
}

QRect KisAnimCurvesKeyDelegate::visualRect(const QModelIndex index) const
{
    QPointF center = nodeCenter(index, false);
    QPointF leftHandlePos = center + leftHandle(index, false);
    QPointF rightHandlePos = center + rightHandle(index, false);

    int minX = qMin(center.x(), leftHandlePos.x()) - NODE_RENDER_RADIUS;
    int maxX = qMax(center.x(), rightHandlePos.x()) + NODE_RENDER_RADIUS;

    int minY = qMin(center.y(), qMin(leftHandlePos.y(), rightHandlePos.y())) - NODE_RENDER_RADIUS;
    int maxY = qMax(center.y(), qMax(leftHandlePos.y(), rightHandlePos.y())) + NODE_RENDER_RADIUS;

    return QRect(QPoint(minX, minY), QPoint(maxX, maxY));
}
