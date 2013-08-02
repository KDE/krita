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
#include "krita_export.h"


/**
 * This structure containg information about the desired spacing
 * requested by the paintAt call
 */
class KisSpacingInformation {
public:
    KisSpacingInformation()
        : m_isIsotropic(true)
    {
    }

    KisSpacingInformation(qreal isotropicSpacing)
        : m_spacing(isotropicSpacing, isotropicSpacing),
          m_isIsotropic(true)
    {
    }

    KisSpacingInformation(const QPointF &anisotropicSpacing)
        : m_spacing(anisotropicSpacing),
          m_isIsotropic(anisotropicSpacing.x() == anisotropicSpacing.y())
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

private:
    QPointF m_spacing;
    bool m_isIsotropic;
};

/**
 * This structure is used as return value of paintLine to contain
 * information that is needed to be passed for the next call.
 */
class KRITAIMAGE_EXPORT KisDistanceInformation {
public:
    KisDistanceInformation();
    KisDistanceInformation(const KisDistanceInformation &rhs);
    KisDistanceInformation& operator=(const KisDistanceInformation &rhs);

    ~KisDistanceInformation();

    qreal getNextPointPosition(const QPointF &start,
                               const QPointF &end);

    const KisSpacingInformation& spacing() const;
    void setSpacing(const KisSpacingInformation &spacing);

private:
    qreal getNextPointPositionIsotropic(const QPointF &start,
                                        const QPointF &end);
    qreal getNextPointPositionAnisotropic(const QPointF &start,
                                          const QPointF &end);
private:
    class Private;
    Private * const m_d;
};

#endif
