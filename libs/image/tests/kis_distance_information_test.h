/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KIS_DISTANCE_INFORMATION_TEST_H
#define KIS_DISTANCE_INFORMATION_TEST_H

#include <simpletest.h>

class KisPaintInformation;
class KisDistanceInformation;

/** Tests some functionality in KisDistanceInformation and related classes. */
class KisDistanceInformationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testInitInfo();
    void testInterpolation();

private:
    void testInitInfoEquality() const;
    void testInitInfoXMLClone() const;

    /**
     * Performs one interpolation using the specified KisDistanceInformation and checks the results.
     * @param interpFactor The interpolation factor that the KisDistanceInformation is expected to
     *                     return.
     * @param needSpacingUpdate Indicates whether the KisDistanceInformation is expected to need a
     *                          spacing update after the interpolation.
     * @param needTimingUpdate Indicates whether the KisDistanceInformation is expected to need a
     *                         timing update after the interpolation.
     */
    void testInterpolationImpl(const KisPaintInformation &p1, const KisPaintInformation &p2,
                               KisDistanceInformation &dist, qreal interpFactor,
                               bool needSpacingUpdate, bool needTimingUpdate,
                               qreal interpTolerance) const;
};

#endif // KIS_DISTANCE_INFORMATION_TEST_H
