/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef _KIS_DISTANCE_INFORMATION_H_
#define _KIS_DISTANCE_INFORMATION_H_

#include <QPointF>
#include <QVector2D>
#include "kritaimage_export.h"

class KisPaintInformation;


/**
 * This structure contains information about the desired spacing
 * requested by the paintAt call
 */
class KisSpacingInformation {
public:
    explicit KisSpacingInformation()
        : m_spacing(0.0, 0.0)
        , m_isIsotropic(true)
        , m_rotation(0.0)
    {
    }

    explicit KisSpacingInformation(qreal isotropicSpacing)
        : m_spacing(isotropicSpacing, isotropicSpacing)
        , m_isIsotropic(true)
        , m_rotation(0.0)
    {
    }

    explicit KisSpacingInformation(const QPointF &anisotropicSpacing, qreal rotation)
        : m_spacing(anisotropicSpacing)
        , m_isIsotropic(anisotropicSpacing.x() == anisotropicSpacing.y())
        , m_rotation(rotation)
    {
    }

    inline QPointF spacing() const {
        return m_spacing;
    }

    inline bool isIsotropic() const {
        return m_isIsotropic;
    }

    inline qreal scalarApprox() const {
        return m_isIsotropic ? m_spacing.x() : QVector2D(m_spacing).length();
    }

    inline qreal rotation() const {
        return m_rotation;
    }

private:
    QPointF m_spacing;
    bool m_isIsotropic;
    qreal m_rotation;
};

/**
 * This structure is used as return value of paintLine to contain
 * information that is needed to be passed for the next call.
 */
class KRITAIMAGE_EXPORT KisDistanceInformation {
public:
    KisDistanceInformation();
    KisDistanceInformation(const QPointF &lastPosition, qreal lastTime);
    KisDistanceInformation(const KisDistanceInformation &rhs);
    KisDistanceInformation(const KisDistanceInformation &rhs, int levelOfDetail);
    KisDistanceInformation& operator=(const KisDistanceInformation &rhs);

    ~KisDistanceInformation();

    const KisSpacingInformation& currentSpacing() const;
    bool hasLastDabInformation() const;
    QPointF lastPosition() const;
    qreal lastTime() const;
    qreal lastDrawingAngle() const;

    bool hasLastPaintInformation() const;
    const KisPaintInformation& lastPaintInformation() const;

    void registerPaintedDab(const KisPaintInformation &info,
                            const KisSpacingInformation &spacing);

    qreal getNextPointPosition(const QPointF &start,
                               const QPointF &end);

    /**
     * \return true if at least one dab has been painted with this
     *         distance information
     */
    bool isStarted() const;

private:
    qreal getNextPointPositionIsotropic(const QPointF &start,
                                        const QPointF &end);
    qreal getNextPointPositionAnisotropic(const QPointF &start,
                                          const QPointF &end);
private:
    struct Private;
    Private * const m_d;
};

#endif
