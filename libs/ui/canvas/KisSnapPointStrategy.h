/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSNAPPOINTSTRATEGY_H
#define KISSNAPPOINTSTRATEGY_H

#include <QScopedPointer>

#include "KoSnapStrategy.h"
#include "kritaui_export.h"

/**
 * The KisSnapPointStrategy class is a custom strategy that allows snapping to
 * arbitrary points on canvas, not linked to any real objects. It can be used,
 * for example, for snapping to the *previous position* of the handle, while it
 * is dragging by the user.
 */

class KRITAUI_EXPORT KisSnapPointStrategy : public KoSnapStrategy
{
public:
    KisSnapPointStrategy(KoSnapGuide::Strategy type = KoSnapGuide::CustomSnapping);
    ~KisSnapPointStrategy() override;

    bool snap(const QPointF &mousePosition, KoSnapProxy * proxy, qreal maxSnapDistance) override;
    QPainterPath decoration(const KoViewConverter &converter) const override;

     void addPoint(const QPointF &pt);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISSNAPPOINTSTRATEGY_H
