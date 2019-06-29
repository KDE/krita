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

#include "kis_animation_curves_keyframe_delegate.h"

#include <QPainter>
#include <QApplication>
#include <QVector2D>

#include "kis_animation_curves_model.h"
#include "kis_keyframe.h"

const int NODE_RENDER_RADIUS = 4;
const int NODE_UI_RADIUS = 8;

struct KisAnimationCurvesKeyframeDelegate::Private
{
    Private(const TimelineRulerHeader *horizontalRuler, const KisAnimationCurvesValueRuler *verticalRuler)
        : horizontalRuler(horizontalRuler)
        , verticalRuler(verticalRuler)
    {}
    const TimelineRulerHeader *horizontalRuler;
    const KisAnimationCurvesValueRuler *verticalRuler;
    QPointF selectionOffset;

    int adjustedHandle;
    QPointF handleAdjustment;

};

KisAnimationCurvesKeyframeDelegate::KisAnimationCurvesKeyframeDelegate(const TimelineRulerHeader *horizontalRuler, const KisAnimationCurvesValueRuler *verticalRuler, QObject *parent)
    : QAbstractItemDelegate(parent)
    , m_d(new Private(horizontalRuler, verticalRuler))
{
}

KisAnimationCurvesKeyframeDelegate::~KisAnimationCurvesKeyframeDelegate()
{}


void KisAnimationCurvesKeyframeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    bool selected = option.state & QStyle::State_Selected;
    bool active = option.state & QStyle::State_HasFocus;
    QPointF center = nodeCenter(index, selected);

    QColor color;
    QColor bgColor = qApp->palette().color(QPalette::Window);
    if (selected) {
        color = (bgColor.value() > 128) ? Qt::black : Qt::white;
    } else {
        color = index.data(KisAnimationCurvesModel::CurveColorRole).value<QColor>();
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

QSize KisAnimationCurvesKeyframeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(2*NODE_UI_RADIUS, 2*NODE_UI_RADIUS);
}

QPointF KisAnimationCurvesKeyframeDelegate::nodeCenter(const QModelIndex index, bool selected) const
{
    int section = m_d->horizontalRuler->logicalIndex(index.column());
    int x = m_d->horizontalRuler->sectionViewportPosition(section)
            + (m_d->horizontalRuler->sectionSize(section) / 2);

    float value = index.data(KisAnimationCurvesModel::ScalarValueRole).toReal();
    float y = m_d->verticalRuler->mapValueToView(value);

    QPointF center = QPointF(x, y);
    if (selected) center += m_d->selectionOffset;
    return center;
}

bool KisAnimationCurvesKeyframeDelegate::hasHandle(const QModelIndex index, int handle) const
{
    QModelIndex interpolatedIndex;

    if (handle == 0) {
        // Left handle: use previous keyframe's interpolation mode

        QVariant previous = index.data(KisAnimationCurvesModel::PreviousKeyframeTime);
        if (!previous.isValid()) return false;

        interpolatedIndex = index.model()->index(index.row(), previous.toInt());
    } else {
        interpolatedIndex = index;
    }

    return (interpolatedIndex.data(KisAnimationCurvesModel::InterpolationModeRole).toInt() == KisKeyframe::Bezier);
}

QPointF KisAnimationCurvesKeyframeDelegate::leftHandle(const QModelIndex index, bool active) const
{
    return handlePosition(index, active, 0);
}

QPointF KisAnimationCurvesKeyframeDelegate::rightHandle(const QModelIndex index, bool active) const
{
    return handlePosition(index, active, 1);
}

QPointF KisAnimationCurvesKeyframeDelegate::handlePosition(const QModelIndex index, bool active, int handle) const
{
    int role = (handle == 0) ? KisAnimationCurvesModel::LeftTangentRole : KisAnimationCurvesModel::RightTangentRole;
    QPointF tangent = index.data(role).toPointF();

    float x = tangent.x() * m_d->horizontalRuler->defaultSectionSize();
    float y = tangent.y() * m_d->verticalRuler->scaleFactor();
    QPointF handlePos = QPointF(x, y);

    if (active && !m_d->handleAdjustment.isNull()) {
        if (handle == m_d->adjustedHandle) {
            handlePos += m_d->handleAdjustment;
            if ((handle == 0 && handlePos.x() > 0) ||
                (handle == 1 && handlePos.x() < 0)) {
                handlePos.setX(0);
            }
        } else if (index.data(KisAnimationCurvesModel::TangentsModeRole).toInt() == KisKeyframe::Smooth) {
            qreal length = QVector2D(handlePos).length();
            QVector2D opposite(handlePosition(index, active, 1-handle));
            handlePos = (-length * opposite.normalized()).toPointF();
        }
    }

    return handlePos;
}

void KisAnimationCurvesKeyframeDelegate::setSelectedItemVisualOffset(QPointF offset)
{
    m_d->selectionOffset = offset;
}

void KisAnimationCurvesKeyframeDelegate::setHandleAdjustment(QPointF offset, int handle)
{
    m_d->adjustedHandle = handle;
    m_d->handleAdjustment = offset;
}

QPointF KisAnimationCurvesKeyframeDelegate::unscaledTangent(QPointF handlePosition) const
{
    qreal x = handlePosition.x() / m_d->horizontalRuler->defaultSectionSize();
    qreal y = handlePosition.y() / m_d->verticalRuler->scaleFactor();

    return QPointF(x, y);
}

void KisAnimationCurvesKeyframeDelegate::paintHandle(QPainter *painter, QPointF nodePos, QPointF tangent) const
{
    QPointF handlePos = nodePos + tangent;

    painter->drawLine(nodePos, handlePos);
    painter->drawEllipse(handlePos, NODE_RENDER_RADIUS, NODE_RENDER_RADIUS);
}

QRect KisAnimationCurvesKeyframeDelegate::itemRect(const QModelIndex index) const
{
    QPointF center = nodeCenter(index, false);

    return QRect(center.x() - NODE_UI_RADIUS, center.y() - NODE_UI_RADIUS, 2*NODE_UI_RADIUS, 2*NODE_UI_RADIUS);
}

QRect KisAnimationCurvesKeyframeDelegate::frameRect(const QModelIndex index) const
{
    int section = m_d->horizontalRuler->logicalIndex(index.column());
    int x = m_d->horizontalRuler->sectionViewportPosition(section);
    int xSize = m_d->horizontalRuler->sectionSize(section);

    float value = index.data(KisAnimationCurvesModel::ScalarValueRole).toReal();
    float y = m_d->verticalRuler->mapValueToView(value);
    int ySize = m_d->verticalRuler->height();

   return QRect(x, y, xSize, ySize);
}

QRect KisAnimationCurvesKeyframeDelegate::visualRect(const QModelIndex index) const
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
