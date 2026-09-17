/*
 *  SPDX-FileCopyrightText: 2026 Ayanami Kaine <personal@ayanamikaine.com>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_temporary_paint_constraint_test.h"

#include <optional>

#include <QSignalSpy>

#include "kis_painting_assistants_decoration.h"
#include "kis_temporary_paint_constraint.h"

void KisTemporaryPaintConstraintTest::testOptionalConstraintState()
{
    std::optional<KisTemporaryPaintConstraint> constraint;
    QVERIFY(!constraint);

    constraint = KisTemporaryPaintConstraint();
    QVERIFY(constraint);
    QVERIFY(!constraint->hasLine());

    constraint->beginLine(QPointF(1.0, 2.0));
    QVERIFY(constraint->hasLine());

    constraint = std::nullopt;
    QVERIFY(!constraint);
}

void KisTemporaryPaintConstraintTest::testInteractionMode()
{
    KisTemporaryPaintConstraint constraint;
    QCOMPARE(constraint.interactionMode(), KisTemporaryPaintConstraint::InteractionMode::Normal);

    constraint.setInteractionMode(KisTemporaryPaintConstraint::InteractionMode::SnapToAngle);
    QCOMPARE(constraint.interactionMode(), KisTemporaryPaintConstraint::InteractionMode::SnapToAngle);

    constraint.beginLine(QPointF(0.0, 0.0));
    constraint.updateLine(QPointF(10.0, 6.0), true);
    QCOMPARE(constraint.interactionMode(), KisTemporaryPaintConstraint::InteractionMode::SnapToAngle);
}

void KisTemporaryPaintConstraintTest::testInfiniteLineProjection()
{
    KisTemporaryPaintConstraint constraint;
    constraint.beginLine(QPointF(0.0, 0.0));
    constraint.updateLine(QPointF(10.0, 0.0));
    constraint.setLocked(true);

    QCOMPARE(constraint.adjustPosition(QPointF(5.0, 7.0), QPointF(0.0, 0.0), false, 0.0), QPointF(5.0, 0.0));
    QCOMPARE(constraint.adjustPosition(QPointF(15.0, -3.0), QPointF(0.0, 0.0), false, 0.0), QPointF(15.0, 0.0));
}

void KisTemporaryPaintConstraintTest::testDegenerateLineReturnsOriginalPoint()
{
    KisTemporaryPaintConstraint constraint;
    constraint.beginLine(QPointF(10.0, 10.0));
    constraint.updateLine(QPointF(10.0, 10.0));

    const QPointF point(15.0, 20.0);
    QCOMPARE(constraint.adjustPosition(point, QPointF(0.0, 0.0), false, 0.0), point);
}

void KisTemporaryPaintConstraintTest::testDegenerateLineCannotBeLocked()
{
    KisTemporaryPaintConstraint constraint;
    constraint.beginLine(QPointF(10.0, 10.0));
    constraint.setLocked(true);

    QVERIFY(!constraint.isLocked());

    constraint.updateLine(QPointF(20.0, 10.0));
    QVERIFY(constraint.hasValidLine());

    constraint.setLocked(true);
    QVERIFY(constraint.isLocked());
}

void KisTemporaryPaintConstraintTest::testUnlockedLineReturnsOriginalPoint()
{
    KisTemporaryPaintConstraint constraint;
    constraint.beginLine(QPointF(0.0, 0.0));
    constraint.updateLine(QPointF(10.0, 0.0));

    const QPointF point(5.0, 7.0);
    QVERIFY(constraint.hasValidLine());
    QCOMPARE(constraint.adjustPosition(point, QPointF(0.0, 0.0), false, 0.0), point);
}

void KisTemporaryPaintConstraintTest::testInitialMovementThreshold()
{
    KisTemporaryPaintConstraint constraint;
    constraint.beginLine(QPointF(0.0, 0.0));
    constraint.updateLine(QPointF(10.0, 0.0));
    constraint.setLocked(true);

    const QPointF strokeBegin(3.0, 3.0);
    QCOMPARE(constraint.adjustPosition(QPointF(4.0, 3.0), strokeBegin, true, 2.0), strokeBegin);
    QCOMPARE(constraint.adjustPosition(QPointF(8.0, 3.0), strokeBegin, true, 2.0), QPointF(8.0, 0.0));
}

void KisTemporaryPaintConstraintTest::testLockedLineIgnoresEndpointUpdates()
{
    KisTemporaryPaintConstraint constraint;
    constraint.beginLine(QPointF(0.0, 0.0));
    constraint.updateLine(QPointF(10.0, 0.0));
    constraint.setLocked(true);

    constraint.updateLine(QPointF(0.0, 10.0));

    QCOMPARE(constraint.lineSecondPoint(), QPointF(10.0, 0.0));
    QCOMPARE(constraint.adjustPosition(QPointF(3.0, 5.0), QPointF(0.0, 0.0), false, 0.0), QPointF(3.0, 0.0));
}

void KisTemporaryPaintConstraintTest::testSnappedLineEndpoint()
{
    KisTemporaryPaintConstraint constraint;
    constraint.beginLine(QPointF(0.0, 0.0));

    constraint.updateLine(QPointF(10.0, 1.0), true);
    QVERIFY(qAbs(constraint.lineSecondPoint().y()) < 1e-6);

    constraint.updateLine(QPointF(10.0, 6.0), true);
    const QPointF snappedPoint = constraint.lineSecondPoint();
    QVERIFY(qAbs(snappedPoint.y() / snappedPoint.x() - 0.5773502691896257) < 1e-6);

    KisTemporaryPaintConstraint freeConstraint;
    freeConstraint.beginLine(QPointF(0.0, 0.0));
    freeConstraint.updateLine(QPointF(10.0, 6.0), false);
    QCOMPARE(freeConstraint.lineSecondPoint(), QPointF(10.0, 6.0));
}

void KisTemporaryPaintConstraintTest::testSnappedLinePersistsAfterShiftRelease()
{
    KisTemporaryPaintConstraint constraint;
    constraint.beginLine(QPointF(0.0, 0.0));

    constraint.updateLine(QPointF(10.0, 6.0), true);
    const QPointF snappedPoint = constraint.lineSecondPoint();

    constraint.updateLine(QPointF(10.0, 6.0), false);
    QCOMPARE(constraint.lineSecondPoint(), snappedPoint);

    constraint.updateLine(QPointF(1.0, 10.0), false);
    QCOMPARE(constraint.lineSecondPoint(), snappedPoint);

    constraint.updateLine(QPointF(1.0, 10.0), true);
    QVERIFY(qAbs(constraint.lineSecondPoint().x()) < 1e-6);
}

void KisTemporaryPaintConstraintTest::testDecorationAvailabilitySignal()
{
    KisPaintingAssistantsDecoration decoration(nullptr);
    QSignalSpy spy(&decoration, SIGNAL(temporaryConstraintAvailabilityChanged()));

    decoration.setTemporaryConstraint(KisTemporaryPaintConstraint());
    QCOMPARE(spy.count(), 1);

    KisTemporaryPaintConstraint constraint;
    constraint.beginLine(QPointF(0.0, 0.0));
    constraint.updateLine(QPointF(10.0, 0.0));
    decoration.setTemporaryConstraint(constraint);
    QCOMPARE(spy.count(), 1);

    decoration.setTemporaryConstraint(std::nullopt);
    QCOMPARE(spy.count(), 2);

    decoration.setTemporaryConstraint(std::nullopt);
    QCOMPARE(spy.count(), 2);
}

SIMPLE_TEST_MAIN(KisTemporaryPaintConstraintTest)
