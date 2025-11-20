/*
 *  SPDX-FileCopyrightText: 2006-2010 Thomas Zander <zander@kde.org>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "TestDependentShapes.h"

#include <MockShapes.h>
#include <testflake.h>

#include <simpletest.h>

#include <KoShapeBulkActionInterface.h>
#include <KoShapeBulkActionLock.h>

struct BulkActionState {
    QRectF originalBoundingRect;
    bool linkedShapesChangedWhileLocked = false;
};

struct MockBulkShape : MockShape, KoShapeBulkActionInterface
{
    using ChangesList = std::vector<std::pair<ChangeType, KoShape*>>;

    void startBulkAction() override {
        KIS_SAFE_ASSERT_RECOVER_RETURN(!m_bulkActionState);
        m_bulkActionState = {boundingRect(), false};
    }
    QRectF endBulkAction() override {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_bulkActionState, QRectF());

        QRectF updateRect;

        if (m_bulkActionState->linkedShapesChangedWhileLocked) {
            forceUpdateLinkedShapes();
            notifyChanged();
            updateRect = m_bulkActionState->originalBoundingRect | boundingRect();
        }

        m_bulkActionState = std::nullopt;

        return updateRect;
    }

    bool isBulkActionInProgress() const {
        return m_bulkActionState.has_value();
    }

    void shapeChanged(ChangeType type, KoShape *shape) override {
        if (!shape && type == KoShape::GenericMatrixChange) {
            // we skip our own changes for unittest simplicity reasons
            return;
        }

        if (m_bulkActionState) {
            m_receivedChangesWhileLocked.emplace_back(type, shape);
        } else {
            m_receivedChangesWhileUnlocked.emplace_back(type, shape);
        }

        if (m_linkedShapes.contains(shape) && type == KoShape::GenericMatrixChange) {
            if (m_bulkActionState) {
                // postpone until bulk action is finished
                m_bulkActionState->linkedShapesChangedWhileLocked = true;
             } else {
                // slow recalculation method!
                forceUpdateLinkedShapes();
            }

            // KoShapeManager's updates are issued without any delay,
            // since they depend on the shape's boundingRect() and that
            // is guaranteed to be correct all the time
            notifyChanged();

            // pass the update further to let unittest check
            // transitive links
            shapeChangedPriv(KoShape::GenericMatrixChange);
        }
    }

    const ChangesList& receivedChangesWhileLocked() const {
        return m_receivedChangesWhileLocked;
    }

    const ChangesList& receivedChangesWhileUnlocked() const {
        return m_receivedChangesWhileUnlocked;
    }

    void clearReceivedChanges() {
        m_receivedChangesWhileLocked.clear();
        m_receivedChangesWhileUnlocked.clear();
    }

    void registerLinkedShapes(std::initializer_list<KoShape*> shapes) {
        m_linkedShapes = std::move(shapes);

        Q_FOREACH(KoShape *shape, m_linkedShapes) {
            shape->addDependee(this);
        }

        forceUpdateLinkedShapes();
    }

    QRectF outlineRect() const override {
        /**
         * outlineRect() is recalculated on the fly as required
         * by KoShapeBulkActionInterface
         */

        QRectF dependentOutlineRect;
        Q_FOREACH(KoShape *shape, m_linkedShapes) {
            dependentOutlineRect |= shape->absoluteOutlineRect();
        }
        return dependentOutlineRect;
    }

    // dependentOutlineRect() is the value that is "slow" to
    // calculate, so we postpone that untill the end of the action
    QRectF dependentOutlineRect() const {
        return m_dependentOutlineRect;
    }

    void forceUpdateLinkedShapes() {
        m_dependentOutlineRect = {};
        Q_FOREACH(KoShape *shape, m_linkedShapes) {
            m_dependentOutlineRect |= shape->absoluteOutlineRect();
        }
    }

private:
    ChangesList m_receivedChangesWhileLocked;
    ChangesList m_receivedChangesWhileUnlocked;
    QRectF m_dependentOutlineRect;
    QVector<KoShape*> m_linkedShapes;

    std::optional<BulkActionState> m_bulkActionState;
};

void TestDependentShapes::testDeletionNotifications_data()
{
    QTest::addColumn<bool>("dependeeFirst");

    QTest::addRow("dependee-source") << true;
    QTest::addRow("source-dependee") << false;
}

void TestDependentShapes::testDeletionNotifications()
{
    QFETCH(bool, dependeeFirst);

    MockShape *shape1(new MockShape());
    MockShape *shape2(new MockShape());

    shape1->addDependee(shape2);

    if (dependeeFirst) {
        delete shape2;
        QVERIFY(shape1->dependees().isEmpty());
        delete shape1;
    } else {
        delete shape1;
        delete shape2;
    }
}

void TestDependentShapes::testBulkActionInterface_data()
{
    QTest::addColumn<bool>("useBulkAction");

    QTest::addRow("non-bulk-action") << false;
    QTest::addRow("bulk-action") << true;
}

bool verifyShapePickable(KoShape *shape, KoShapeManager *shapeManager)
{
    auto pickOnePoint = [=] (KoFlake::AnchorPosition pos) {
        const QPointF pt = shape->absolutePosition(pos);
        KoShape *pickedShape = shapeManager->shapeAt(pt);
        if (pickedShape != shape) {
            qCritical() << "Failed to pick a shape using shape manager"
                << ppVar(shape->name())
                << ppVar(shape->boundingRect())
                << ppVar(pos)
                << ppVar(pt);
            return false;
        }
        return true;
    };
    return
        pickOnePoint(KoFlake::TopLeft) &&
        pickOnePoint(KoFlake::TopRight) &&
        pickOnePoint(KoFlake::BottomRight) &&
        pickOnePoint(KoFlake::BottomLeft);
}

void TestDependentShapes::testBulkActionInterface()
{
    QFETCH(bool, useBulkAction);

    MockShapeController controller;
    MockCanvas canvas(&controller);

    KoShapeManager *shapeManager = canvas.shapeManager();

    MockShape *shape1(new MockShape());
    shape1->setSize({10, 10});
    shape1->setAbsolutePosition({5, 5}, KoFlake::TopLeft);
    QCOMPARE(shape1->absoluteOutlineRect(), QRectF(5, 5, 10, 10));

    MockShape *shape2(new MockShape());
    shape2->setSize({10, 10});
    shape2->setAbsolutePosition({90, 90}, KoFlake::TopLeft);
    QCOMPARE(shape2->absoluteOutlineRect(), QRectF(90, 90, 10, 10));

    MockBulkShape *bulkInterfaceShape(new MockBulkShape());
    QCOMPARE(bulkInterfaceShape->dependentOutlineRect(), QRectF());
    bulkInterfaceShape->registerLinkedShapes({shape1, shape2});
    QCOMPARE(bulkInterfaceShape->dependentOutlineRect(), QRectF(5, 5, 95, 95));

    shapeManager->addShape(bulkInterfaceShape);
    QVERIFY(verifyShapePickable(bulkInterfaceShape, shapeManager));

    QVERIFY(bulkInterfaceShape->receivedChangesWhileLocked().empty());
    QVERIFY(bulkInterfaceShape->receivedChangesWhileUnlocked().empty());

    if (!useBulkAction) {
        // try update child shapes without locking
        // move shape1 by 10,10
        shape1->applyTransformation(QTransform::fromTranslate(10, 10));
        QCOMPARE(shape1->absoluteOutlineRect(), QRectF(15, 15, 10, 10));
        QCOMPARE(bulkInterfaceShape->receivedChangesWhileUnlocked(),
                 MockBulkShape::ChangesList({
                   std::make_pair(KoShape::GenericMatrixChange, shape1)
                 }));
        QCOMPARE(bulkInterfaceShape->receivedChangesWhileLocked(), {});
        QCOMPARE(bulkInterfaceShape->dependentOutlineRect(), QRectF(15, 15, 85, 85));
        QVERIFY(verifyShapePickable(bulkInterfaceShape, shapeManager));

        // move shape2 by 10,10
        shape2->applyTransformation(QTransform::fromTranslate(10, 10));
        QCOMPARE(shape2->absoluteOutlineRect(), QRectF(100, 100, 10, 10));
        QCOMPARE(bulkInterfaceShape->receivedChangesWhileUnlocked(),
                 MockBulkShape::ChangesList({
                    std::make_pair(KoShape::GenericMatrixChange, shape1),
                    std::make_pair(KoShape::GenericMatrixChange, shape2)
                 }));
        QCOMPARE(bulkInterfaceShape->receivedChangesWhileLocked(), {});
        QCOMPARE(bulkInterfaceShape->dependentOutlineRect(), QRectF(15, 15, 95, 95));
        QVERIFY(verifyShapePickable(bulkInterfaceShape, shapeManager));
    } else {
        bulkInterfaceShape->startBulkAction();

        // move shape1 by 10,10
        shape1->applyTransformation(QTransform::fromTranslate(10, 10));
        QCOMPARE(shape1->absoluteOutlineRect(), QRectF(15, 15, 10, 10));
        QCOMPARE(bulkInterfaceShape->receivedChangesWhileUnlocked(), {});
        QCOMPARE(bulkInterfaceShape->receivedChangesWhileLocked(),
                 MockBulkShape::ChangesList({
                     std::make_pair(KoShape::GenericMatrixChange, shape1)
                 }));

        // unchanged!!!
        QCOMPARE(bulkInterfaceShape->dependentOutlineRect(), QRectF(5, 5, 95, 95));
        // pickable, because boundingRect() is guaranteed to be correct
        QVERIFY(verifyShapePickable(bulkInterfaceShape, shapeManager));

        // move shape2 by 10,10
        shape2->applyTransformation(QTransform::fromTranslate(10, 10));
        QCOMPARE(shape2->absoluteOutlineRect(), QRectF(100, 100, 10, 10));

        QCOMPARE(bulkInterfaceShape->receivedChangesWhileUnlocked(), {});
        QCOMPARE(bulkInterfaceShape->receivedChangesWhileLocked(),
                 MockBulkShape::ChangesList({
                     std::make_pair(KoShape::GenericMatrixChange, shape1),
                     std::make_pair(KoShape::GenericMatrixChange, shape2)
                 }));

        // unchanged!!!
        QCOMPARE(bulkInterfaceShape->dependentOutlineRect(), QRectF(5, 5, 95, 95));
        // pickable, because boundingRect() is guaranteed to be correct
        QVERIFY(verifyShapePickable(bulkInterfaceShape, shapeManager));

        const QRectF resultingUpdate = bulkInterfaceShape->endBulkAction();

        // now finally updated!
        QCOMPARE(bulkInterfaceShape->dependentOutlineRect(), QRectF(15, 15, 95, 95));

        // the update should include old and new bounding rects!
        QCOMPARE(resultingUpdate, QRectF(5, 5, 95, 95) | QRectF(15, 15, 95, 95));
    }

    shapeManager->setShapes({});
    delete shape1;
    delete shape2;
    delete bulkInterfaceShape;
}

bool compareUpdatesLists(const KoShapeBulkActionLock::UpdatesList &lhs,
                         const KoShapeBulkActionLock::UpdatesList &rhs)
{
    auto updateListToHash =
    [] (const KoShapeBulkActionLock::UpdatesList &list) {

        std::unordered_map<KoShape*, QRectF> result;

        for (auto it = list.begin(); it != list.end(); ++it) {
            result.insert(*it);
        }

        return result;
    };

    auto lhsHash = updateListToHash(lhs);
    auto rhsHash = updateListToHash(rhs);

    const bool result = lhsHash == rhsHash;

    if (!result || (lhsHash.size() != lhs.size()) || (rhsHash.size() != rhs.size())) {
        auto printList = [] (KoShapeBulkActionLock::UpdatesList list) {
            std::sort(list.begin(), list.end(),
                      kismpl::mem_less(&KoShapeBulkActionLock::Update::first));
            for (auto it = list.begin(); it != list.end(); ++it) {
                qWarning() << "      " << it->first << it->second;
            }
        };

        qWarning() << "Failed to compare update lists:";
        qWarning() << "   lhs:";
        printList(lhs);
        qWarning() << "   rhs:";
        printList(rhs);
    }

    return result;
}

void TestDependentShapes::testBulkActionLock_data()
{
    QTest::addColumn<QPointF>("moveOffset");
    QTest::addColumn<bool>("useBulkInterfaceShape");
    QTest::addColumn<bool>("useSecondBulkInterfaceShape");

    QTest::addRow("no-change") << QPointF() << false << false;
    QTest::addRow("move") << QPointF(10, 10) << false << false;
    QTest::addRow("move-and-check-bulk") << QPointF(10, 10) << true << false;
    QTest::addRow("move-and-check-chained-bulk") << QPointF(10, 10) << true << false;
}

void TestDependentShapes::testBulkActionLock()
{
    QFETCH(QPointF, moveOffset);
    QFETCH(bool, useBulkInterfaceShape);
    QFETCH(bool, useSecondBulkInterfaceShape);

    const QRectF originalRectShape1 = QRectF(5, 5, 10, 10);
    const QRectF originalRectShape2 = QRectF(90, 90, 10, 10);

    const QRectF expectedUpdateRectShape1 = originalRectShape1 | originalRectShape1.translated(moveOffset);
    const QRectF expectedUpdateRectShape2 = originalRectShape2 | originalRectShape2.translated(moveOffset);

    MockShape *shape1(new MockShape());
    shape1->setSize(originalRectShape1.size());
    shape1->setAbsolutePosition(originalRectShape1.topLeft(), KoFlake::TopLeft);
    QCOMPARE(shape1->absoluteOutlineRect(), originalRectShape1);

    MockShape *shape2(new MockShape());
    shape2->setSize(originalRectShape2.size());
    shape2->setAbsolutePosition(originalRectShape2.topLeft(), KoFlake::TopLeft);
    QCOMPARE(shape2->absoluteOutlineRect(), originalRectShape2);

    MockBulkShape *bulkInterfaceShape = nullptr;
    MockBulkShape *secondBulkInterfaceShape = nullptr;

    if (useBulkInterfaceShape) {
        bulkInterfaceShape = new MockBulkShape();
        QCOMPARE(bulkInterfaceShape->dependentOutlineRect(), QRectF());
        bulkInterfaceShape->registerLinkedShapes({shape1, shape2});
        QCOMPARE(bulkInterfaceShape->dependentOutlineRect(), QRectF(5, 5, 95, 95));
    }

    if (useSecondBulkInterfaceShape) {
        secondBulkInterfaceShape = new MockBulkShape();
        QCOMPARE(secondBulkInterfaceShape->dependentOutlineRect(), QRectF());
        secondBulkInterfaceShape->registerLinkedShapes({bulkInterfaceShape});
        QCOMPARE(secondBulkInterfaceShape->dependentOutlineRect(), QRectF(5, 5, 95, 95));
    }

    QList<KoShape*> shapes = {shape1, shape2};

    KoShapeBulkActionLock lock(shapes);

    if (!moveOffset.isNull()) {
        shape1->applyTransformation(QTransform::fromTranslate(moveOffset.x(), moveOffset.y()));
        shape2->applyTransformation(QTransform::fromTranslate(moveOffset.x(), moveOffset.y()));

        if (bulkInterfaceShape) {
            QCOMPARE(bulkInterfaceShape->receivedChangesWhileLocked(),
                    MockBulkShape::ChangesList({
                        std::make_pair(KoShape::GenericMatrixChange, shape1),
                        std::make_pair(KoShape::GenericMatrixChange, shape2)
                    }));

            // the dependent value is outdated!
            QCOMPARE(bulkInterfaceShape->dependentOutlineRect(), originalRectShape1 | originalRectShape2);
        }
    }

    if (secondBulkInterfaceShape) {
        // we are going to receive this update twice!
        QCOMPARE(secondBulkInterfaceShape->receivedChangesWhileLocked(),
                 MockBulkShape::ChangesList({
                     std::make_pair(KoShape::GenericMatrixChange, bulkInterfaceShape),
                     std::make_pair(KoShape::GenericMatrixChange, bulkInterfaceShape)
                 }));
        // the dependent value is outdated!
        QCOMPARE(secondBulkInterfaceShape->dependentOutlineRect(), originalRectShape1 | originalRectShape2);
    }

    const KoShapeBulkActionLock::UpdatesList resultingUpdate = lock.unlock();

    KoShapeBulkActionLock::UpdatesList expectedUpdatesList;

    if (bulkInterfaceShape) {
        // the dependent value is now correct!
        QCOMPARE(bulkInterfaceShape->dependentOutlineRect(), shape1->absoluteOutlineRect() | shape2->absoluteOutlineRect());
    }

    if (secondBulkInterfaceShape) {
        // the dependent value is now correct!
        QCOMPARE(secondBulkInterfaceShape->dependentOutlineRect(), shape1->absoluteOutlineRect() | shape2->absoluteOutlineRect());
    }

    expectedUpdatesList.emplace_back(shape1, expectedUpdateRectShape1);
    expectedUpdatesList.emplace_back(shape2, expectedUpdateRectShape2);

    if (bulkInterfaceShape) {
        expectedUpdatesList.emplace_back(bulkInterfaceShape,
            originalRectShape1 |
            originalRectShape2 |
            shape1->boundingRect() |
            shape2->boundingRect());
    }

    if (secondBulkInterfaceShape) {
        expectedUpdatesList.emplace_back(secondBulkInterfaceShape,
            originalRectShape1 |
            originalRectShape2 |
            bulkInterfaceShape->boundingRect());
    }

    // NOTE: The order of shape updates is undefined by protocol
    QVERIFY(compareUpdatesLists(resultingUpdate, expectedUpdatesList));

    /**
     * Delete bulk interface shapes **before** the actual shapes
     * to possibly trigger "dead dependee" issue.
     */
    delete secondBulkInterfaceShape;
    delete bulkInterfaceShape;

    delete shape1;
    delete shape2;
}

KISTEST_MAIN(TestDependentShapes)
