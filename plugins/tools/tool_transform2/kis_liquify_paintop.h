/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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
