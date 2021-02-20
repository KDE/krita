/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SNAP_LINE_STRATEGY_H
#define __KIS_SNAP_LINE_STRATEGY_H

#include <QScopedPointer>

#include "KoSnapStrategy.h"


class KisSnapLineStrategy : public KoSnapStrategy
{
public:
    KisSnapLineStrategy(KoSnapGuide::Strategy type = KoSnapGuide::CustomSnapping);
    ~KisSnapLineStrategy() override;

    bool snap(const QPointF &mousePosition, KoSnapProxy * proxy, qreal maxSnapDistance) override;
    QPainterPath decoration(const KoViewConverter &converter) const override;

    void addLine(Qt::Orientation orientation, qreal pos);

    void setHorizontalLines(const QList<qreal> &lines);
    void setVerticalLines(const QList<qreal> &lines);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_SNAP_LINE_STRATEGY_H */
