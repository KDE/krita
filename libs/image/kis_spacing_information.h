/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KIS_SPACING_INFORMATION_H
#define KIS_SPACING_INFORMATION_H

#include "kritaimage_export.h"

/**
 * Contains information about distance-based spacing settings in a stroke. The spacing settings may
 * be different at different parts of a stroke, e.g. if spacing is linked to pressure; a
 * KisSpacingInformation represents the effective spacing at a single specific part of the stroke.
 */
class KRITAIMAGE_EXPORT KisSpacingInformation {
public:
    explicit KisSpacingInformation()
        : m_distanceSpacingEnabled(true)
        , m_distanceSpacing(0.0, 0.0)
        , m_rotation(0.0)
        , m_coordinateSystemFlipped(false)
    {
    }

    explicit KisSpacingInformation(qreal isotropicSpacing)
        : m_distanceSpacingEnabled(true)
        , m_distanceSpacing(isotropicSpacing, isotropicSpacing)
        , m_rotation(0.0)
        , m_coordinateSystemFlipped(false)
    {
    }

    explicit KisSpacingInformation(const QPointF &anisotropicSpacing, qreal rotation,
                                   bool coordinateSystemFlipped)
        : m_distanceSpacingEnabled(true)
        , m_distanceSpacing(anisotropicSpacing)
        , m_rotation(rotation)
        , m_coordinateSystemFlipped(coordinateSystemFlipped)
    {
    }

    explicit KisSpacingInformation(bool distanceSpacingEnabled, qreal isotropicSpacing)
        : m_distanceSpacingEnabled(distanceSpacingEnabled)
        , m_distanceSpacing(isotropicSpacing, isotropicSpacing)
        , m_rotation(0.0)
        , m_coordinateSystemFlipped(false)
    {
    }

    explicit KisSpacingInformation(bool distanceSpacingEnabled,
                                   const QPointF &anisotropicSpacing,
                                   qreal rotation,
                                   bool coordinateSystemFlipped)
        : m_distanceSpacingEnabled(distanceSpacingEnabled)
        , m_distanceSpacing(anisotropicSpacing)
        , m_rotation(rotation)
        , m_coordinateSystemFlipped(coordinateSystemFlipped)
    {
    }

    /**
     * @return True if and only if distance-based spacing is enabled.
     */
    inline bool isDistanceSpacingEnabled() const {
        return m_distanceSpacingEnabled;
    }

    inline QPointF distanceSpacing() const {
        return m_distanceSpacing;
    }

    inline bool isIsotropic() const {
        return m_distanceSpacing.x() == m_distanceSpacing.y();
    }

    inline qreal scalarApprox() const {
        return isIsotropic() ? m_distanceSpacing.x() : QVector2D(m_distanceSpacing).length();
    }

    inline qreal rotation() const {
        return m_rotation;
    }

    bool coordinateSystemFlipped() const {
        return m_coordinateSystemFlipped;
    }

private:

    // Distance-based spacing
    bool m_distanceSpacingEnabled;
    QPointF m_distanceSpacing;

    qreal m_rotation;
    bool m_coordinateSystemFlipped;
};

#endif // KIS_SPACING_INFORMATION_H
