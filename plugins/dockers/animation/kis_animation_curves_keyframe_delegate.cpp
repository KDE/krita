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

#include "kis_animation_curves_model.h"

struct KisAnimationCurvesKeyframeDelegate::Private
{
    Private(const TimelineRulerHeader *horizontalRuler, const KisAnimationCurvesValueRuler *verticalRuler)
        : horizontalRuler(horizontalRuler)
        , verticalRuler(verticalRuler)
    {}
    const TimelineRulerHeader *horizontalRuler;
    const KisAnimationCurvesValueRuler *verticalRuler;
};

KisAnimationCurvesKeyframeDelegate::KisAnimationCurvesKeyframeDelegate(const TimelineRulerHeader *horizontalRuler, const KisAnimationCurvesValueRuler *verticalRuler, QObject *parent)
    : QAbstractItemDelegate(parent)
    , m_d(new Private(horizontalRuler, verticalRuler))
{}

KisAnimationCurvesKeyframeDelegate::~KisAnimationCurvesKeyframeDelegate()
{}

void KisAnimationCurvesKeyframeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QColor color = index.data(KisAnimationCurvesModel::CurveColorRole).value<QColor>();
    qreal value = index.data(KisAnimationCurvesModel::ScalarValueRole).toReal();

    int section = m_d->horizontalRuler->logicalIndex(index.column());
    int x = m_d->horizontalRuler->sectionViewportPosition(section)
            + (m_d->horizontalRuler->sectionSize(section) / 2);
    int y = m_d->verticalRuler->mapValueToView(value);

    painter->setPen(QPen(color, 0));
    painter->setBrush(color);
    painter->drawEllipse(QPoint(x, y), 2, 2);
}

QSize KisAnimationCurvesKeyframeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(4,4);
}

