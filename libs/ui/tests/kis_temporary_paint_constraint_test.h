/*
 *  SPDX-FileCopyrightText: 2026 Ayanami Kaine <personal@ayanamikaine.com>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TEMPORARY_PAINT_CONSTRAINT_TEST_H
#define KIS_TEMPORARY_PAINT_CONSTRAINT_TEST_H

#include <simpletest.h>

class KisTemporaryPaintConstraintTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testOptionalConstraintState();
    void testInteractionMode();
    void testInfiniteLineProjection();
    void testDegenerateLineReturnsOriginalPoint();
    void testDegenerateLineCannotBeLocked();
    void testUnlockedLineReturnsOriginalPoint();
    void testInitialMovementThreshold();
    void testLockedLineIgnoresEndpointUpdates();
    void testSnappedLineEndpoint();
    void testSnappedLinePersistsAfterShiftRelease();
    void testDecorationAvailabilitySignal();
};

#endif // KIS_TEMPORARY_PAINT_CONSTRAINT_TEST_H
