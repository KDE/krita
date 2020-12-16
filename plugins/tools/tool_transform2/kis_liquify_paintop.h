/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LIQUIFY_PAINTOP_H
#define __KIS_LIQUIFY_PAINTOP_H

#include <QScopedPointer>

class KisLiquifyTransformWorker;
class KisPaintInformation;
class KisSpacingInformation;
class KisTimingInformation;
class KisDistanceInformation;
class KisLiquifyProperties;
class QPainterPath;


class KisLiquifyPaintop
{
public:
    KisLiquifyPaintop(const KisLiquifyProperties &props,
                      KisLiquifyTransformWorker *worker);
    ~KisLiquifyPaintop();

    KisSpacingInformation paintAt(const KisPaintInformation &pi);

    /**
     * Updates the spacing in currentDistance based on the provided information.
     */
    void updateSpacing(const KisPaintInformation &info, KisDistanceInformation &currentDistance)
        const;

    /**
     * Updates the timing in currentDistance based on the provided information.
     */
    void updateTiming(const KisPaintInformation &info, KisDistanceInformation &currentDistance)
        const;

    KisTimingInformation updateTimingImpl(const KisPaintInformation &pi) const;

    static QPainterPath brushOutline(const KisLiquifyProperties &props, const KisPaintInformation &info);

protected:
    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &pi) const;

private:
    qreal computeSize(const KisPaintInformation &pi) const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_LIQUIFY_PAINTOP_H */
