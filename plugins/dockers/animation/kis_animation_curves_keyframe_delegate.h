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

#ifndef _KIS_ANIMATION_CURVES_KEYFRAME_DELEGATE_H
#define _KIS_ANIMATION_CURVES_KEYFRAME_DELEGATE_H

#include <qabstractitemdelegate.h>

#include "timeline_ruler_header.h"
#include "kis_animation_curves_value_ruler.h"

class KisAnimationCurvesKeyframeDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    KisAnimationCurvesKeyframeDelegate(const TimelineRulerHeader *horizontalRuler, const KisAnimationCurvesValueRuler *verticalRuler,  QObject *parent);
    ~KisAnimationCurvesKeyframeDelegate() override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QPointF nodeCenter(const QModelIndex index, bool selected) const;
    bool hasHandle(const QModelIndex index, int handle) const;
    QPointF leftHandle(const QModelIndex index, bool active) const;
    QPointF rightHandle(const QModelIndex index, bool active) const;
    void setSelectedItemVisualOffset(QPointF offset);
    void setHandleAdjustment(QPointF offset, int handle);
    QPointF unscaledTangent(QPointF handlePosition) const;

    QRect itemRect(const QModelIndex index) const;
    QRect frameRect(const QModelIndex index) const;
    QRect visualRect(const QModelIndex index) const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;

    void paintHandle(QPainter *painter, QPointF nodePos, QPointF tangent) const;
    QPointF handlePosition(const QModelIndex index, bool active, int handle) const;

};

#endif


