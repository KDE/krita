/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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


