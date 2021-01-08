/*
 *  SPDX-FileCopyrightText: 2014, 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_mesh_transform_worker_test.h"

#include <QTest>

#include <KoColor.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include "testutil.h"
#include <kis_algebra_2d.h>
#include "KisCppQuirks.h"

#include <KisBezierUtils.h>
#include <KisBezierPatch.h>


#include <KisBezierMesh.h>
#include <KisBezierGradientMesh.h>
#include <KisBezierTransformMesh.h>

#include "kis_dom_utils.h"

#include <kis_grid_interpolation_tools.h>

using namespace KisBezierUtils;


bool testCurveLinear(const QPointF &p0,
                     const QPointF &p1,
                     const QPointF &p2,
                     const QPointF &p3,
                     qreal threshold,
                     bool expectedValue)
{
    qDebug() << "== Testing curve:" << p0 << p1 << p2 << p3 << ppVar(threshold);

    const bool isLinear = isLinearSegmentByControlPoints(p0, p1, p2, p3, threshold);

    //qDebug() << ppVar(isLinear);

    bool distanceCorrect = true;

    for (qreal t = 0.0; t < 1.0; t += 0.05) {
        const QPointF pt = bezierCurve(p0, p1, p2, p3, t);

        //qDebug() << ppVar(t) << pt;

        const qreal distance = kisDistanceToLine(pt, QLineF(p0, p3));

        if (distance > threshold) {
            distanceCorrect = false;
            //qDebug() << "Non-linear point" << ppVar(t) << ppVar(pt) << ppVar(distance);
        }
    }


    //qDebug() << "==";

    return isLinear == expectedValue && (!expectedValue || distanceCorrect);
}

void KisMeshTransformWorkerTest::testIsCurveLinear()
{
    QVERIFY(testCurveLinear(QPointF(10,10), QPointF(10.5, 10.5),
                            QPointF(19.5, 9.5), QPointF(20, 10), 1.0, true));
    QVERIFY(testCurveLinear(QPointF(10,10), QPointF(12, 12),
                            QPointF(18, 8), QPointF(20, 10), 0.5, false));

    QVERIFY(testCurveLinear(QPointF(10,10), QPointF(9.5, 9.5),
                            QPointF(19.5, 9.5), QPointF(20, 10), 1.0, true));
    QVERIFY(testCurveLinear(QPointF(10,10), QPointF(8, 8),
                            QPointF(22, 8), QPointF(20, 10), 1.0, false));

}

void KisMeshTransformWorkerTest::testPointsQImage()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage srcImage(TestUtil::fetchDataFileLazy("test_transform_quality_second.png"));

    KisPaintDeviceSP srcDev = new KisPaintDevice(cs);
    srcDev->convertFromQImage(srcImage, 0);

    const QRect initialRect(srcImage.rect());

    KisBezierPatch patch;
    patch.originalRect = initialRect;

    patch.points[0] = initialRect.topLeft();
    patch.points[1] = initialRect.topLeft() + QPointF(300, 30);
    patch.points[2] = initialRect.topLeft() + QPointF(20, 300);
    patch.points[3] = initialRect.topRight();
    patch.points[4] = initialRect.topRight() + QPointF(-300, 30);
    patch.points[5] = initialRect.topRight() + QPointF(-20, 300);
    patch.points[6] = initialRect.bottomLeft();
    patch.points[7] = initialRect.bottomLeft() + QPointF(300, 30);
    patch.points[8] = initialRect.bottomLeft() + QPointF(20, -300);
    patch.points[9] = initialRect.bottomRight();
    patch.points[10] = initialRect.bottomRight() + QPointF(-300, 30);
    patch.points[11] = initialRect.bottomRight() + QPointF(-20, -300);

    const QRect dstBoundsI = patch.dstBoundingRect().toAlignedRect();

    {


        QImage dstImage(dstBoundsI.size(), srcImage.format());
        dstImage.fill(0);

        const QPoint srcQImageOffset;
        const QPoint dstQImageOffset;

        KisBezierTransformMesh::transformPatch(patch,
                                               srcQImageOffset, srcImage,
                                               dstQImageOffset, &dstImage);
        dstImage.save("dd_mesh_result.png");
    }

    {
        KisPaintDeviceSP dstDev = new KisPaintDevice(srcDev->colorSpace());
        dstDev->prepareClone(srcDev);


        KisBezierTransformMesh::transformPatch(patch, srcDev, dstDev);

        dstDev->convertToQImage(0, dstBoundsI).save("dd_mesh_result_dev.png");
    }
}

void KisMeshTransformWorkerTest::testGradient()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP srcDev = new KisPaintDevice(cs);

    const QRect initialRect(0,0,1600, 1200);

    KisBezierGradientMeshDetail::GradientMeshPatch patch;
    patch.originalRect = QRectF(0, 0, 1.0, 1.0);

    patch.points[0] = initialRect.topLeft();
    patch.points[1] = initialRect.topLeft() + QPointF(300, 30);
    patch.points[2] = initialRect.topLeft() + QPointF(20, 300);
    patch.points[3] = initialRect.topRight();
    patch.points[4] = initialRect.topRight() + QPointF(-300, 30);
    patch.points[5] = initialRect.topRight() + QPointF(-20, 300);
    patch.points[6] = initialRect.bottomLeft();
    patch.points[7] = initialRect.bottomLeft() + QPointF(300, 30);
    patch.points[8] = initialRect.bottomLeft() + QPointF(20, -300);
    patch.points[9] = initialRect.bottomRight();
    patch.points[10] = initialRect.bottomRight() + QPointF(-300, 30);
    patch.points[11] = initialRect.bottomRight() + QPointF(-20, -300);

    patch.colors[0] = Qt::white;
    patch.colors[1] = Qt::red;
    patch.colors[2] = Qt::green;
    patch.colors[3] = Qt::yellow;

    const QRect dstBoundsI = patch.dstBoundingRect().toAlignedRect();
    QImage dstImage(dstBoundsI.size(), QImage::Format_ARGB32);
    dstImage.fill(255);

    KisBezierGradientMesh::renderPatch(patch, QPoint(), &dstImage);

    dstImage.save("dd_mesh_result_grad.png");
}

void KisMeshTransformWorkerTest::testMeshSubdivision()
{

    {
        KisBezierMesh mesh(QRectF(0,0,100,100));

        mesh.subdivideRow(0.5);
        mesh.subdivideColumn(0.5);

        QCOMPARE(mesh.size(), QSize(3, 3));
        QCOMPARE(mesh.node(0, 0).node, QPointF(0, 0));
        QCOMPARE(mesh.node(0, 1).node, QPointF(0, 50));
        QCOMPARE(mesh.node(0, 2).node, QPointF(0, 100));
        QCOMPARE(mesh.node(1, 0).node, QPointF(50, 0));
        QCOMPARE(mesh.node(1, 1).node, QPointF(50, 50));
        QCOMPARE(mesh.node(1, 2).node, QPointF(50, 100));
        QCOMPARE(mesh.node(2, 0).node, QPointF(100, 0));
        QCOMPARE(mesh.node(2, 1).node, QPointF(100, 50));
        QCOMPARE(mesh.node(2, 2).node, QPointF(100, 100));
    }


    {
        KisBezierMesh mesh(QRectF(0,0,100,100));
        mesh.node(0,0).setRightControlRelative(QPointF(10, -5));
        mesh.node(0,0).setBottomControlRelative(QPointF(-5, 10));

        mesh.node(1,0).setLeftControlRelative(QPointF(-10, -5));
        mesh.node(1,0).setBottomControlRelative(QPointF(5, 10));

        mesh.node(0,1).setRightControlRelative(QPointF(10, 5));
        mesh.node(0,1).setTopControlRelative(QPointF(-5, -10));

        mesh.node(1,1).setLeftControlRelative(QPointF(-10, 5));
        mesh.node(1,1).setTopControlRelative(QPointF(5, -10));

        mesh.subdivideRow(0.5);
        mesh.subdivideColumn(0.5);

        QCOMPARE(mesh.size(), QSize(3, 3));
        QCOMPARE(mesh.node(0, 0).node, QPointF(0, 0));
        QCOMPARE(mesh.node(0, 1).node, QPointF(-3.75, 50));
        QCOMPARE(mesh.node(0, 2).node, QPointF(0, 100));
        QCOMPARE(mesh.node(1, 0).node, QPointF(50, -3.75));
        QCOMPARE(mesh.node(1, 1).node, QPointF(50, 50));
        QCOMPARE(mesh.node(1, 2).node, QPointF(50, 103.75));
        QCOMPARE(mesh.node(2, 0).node, QPointF(100, 0));
        QCOMPARE(mesh.node(2, 1).node, QPointF(103.75, 50));
        QCOMPARE(mesh.node(2, 2).node, QPointF(100, 100));
    }
}

void KisMeshTransformWorkerTest::testGlobalToLocal()
{
    KisBezierMesh mesh(QRectF(0,0,100,100));
    mesh.node(0,0).setRightControlRelative(QPointF(10, -5));
    mesh.node(0,0).setBottomControlRelative(QPointF(-5, 10));

    mesh.node(1,0).setLeftControlRelative(QPointF(-10, -5));
    mesh.node(1,0).setBottomControlRelative(QPointF(5, 10));

    mesh.node(0,1).setRightControlRelative(QPointF(10, 5));
    mesh.node(0,1).setTopControlRelative(QPointF(-5, -10));

    mesh.node(1,1).setLeftControlRelative(QPointF(-10, 5));
    mesh.node(1,1).setTopControlRelative(QPointF(5, -10));


    KisBezierPatch patch = mesh.makePatch(0,0);


    auto verifyPoint = [&] (const QPointF &globalPoint, const QPointF &expectedLocalPoint) {
        const qreal eps = 1e-3;
        const QPointF local = KisBezierUtils::calculateLocalPos(patch.points, globalPoint);

        bool result = true;

        if (!KisAlgebra2D::fuzzyPointCompare(local, expectedLocalPoint, eps)) {
            qDebug() << "Failed to find local point:" << ppVar(globalPoint) << ppVar(local) << ppVar(expectedLocalPoint);
            result = false;
        }

        return result;
    };

    QVERIFY(verifyPoint(QPointF(0,0), QPointF(0,0)));
    QVERIFY(verifyPoint(QPointF(-3.75,50), QPointF(0,0.5)));
    QVERIFY(verifyPoint(QPointF(0,100), QPointF(0, 1.0)));

    QVERIFY(verifyPoint(QPointF(50,-3.75), QPointF(0.5,0)));
    QVERIFY(verifyPoint(QPointF(50,50), QPointF(0.5,0.5)));
    QVERIFY(verifyPoint(QPointF(50,103.75), QPointF(0.5, 1.0)));

    QVERIFY(verifyPoint(QPointF(100,0), QPointF(1.0,0)));
    QVERIFY(verifyPoint(QPointF(103.75,50), QPointF(1.0,0.5)));
    QVERIFY(verifyPoint(QPointF(100,100), QPointF(1.0, 1.0)));
}

void KisMeshTransformWorkerTest::testDistanceToCurve()
{
    const QPointF p0(100, 100);
    const QPointF p1(120, 120);
    const QPointF p2(180, 120);
    const QPointF p3(200, 100);

    const QList<QPointF> controlPoints({p0, p1, p2, p3});

    auto verifyPoint = [&] (const QPointF &pt, const QPointF &expectedNearestPoint) {
        const qreal eps = 1e-3;

        const qreal t = KisBezierUtils::nearestPoint(controlPoints, pt);
        const QPointF nearestPoint = bezierCurve(controlPoints, t);

        bool result = true;

        if (!KisAlgebra2D::fuzzyPointCompare(nearestPoint, expectedNearestPoint, eps)) {
            qDebug() << "Failed to find nearest point:" << ppVar(pt) << ppVar(nearestPoint) << ppVar(expectedNearestPoint);
            result = false;
        }

        return result;
    };

    QVERIFY(verifyPoint(QPointF(0,0), p0));
    QVERIFY(verifyPoint(QPointF(300,0), p3));
    QVERIFY(verifyPoint(QPointF(150,300), QPointF(150, 115)));
    QVERIFY(verifyPoint(QPointF(100,150), QPointF(115.425,109.326)));
    QVERIFY(verifyPoint(QPointF(200,150), QPointF(184.575,109.326)));
}

void KisMeshTransformWorkerTest::testRemovePoint()
{
    const QPointF p0(100, 100);
    const QPointF p1(120, 120);
    const QPointF p2(180, 120);
    const QPointF p3(200, 100);

    QPointF c0, c1, c2, c3;
    QPointF q0, q1, q2, q3;

    c0 = p0;
    q3 = p3;

    const qreal t1 = 0.05;

    deCasteljau(p0, p1, p2, p3, t1, &c1, &c2, &c3, &q1, &q2);
    q0 = c3;

//    qDebug() << ppVar(p0) << ppVar(p1) << ppVar(p2) << ppVar(p3);
//    qDebug() << ppVar(c0) << ppVar(c1) << ppVar(c2) << ppVar(c3);
//    qDebug() << ppVar(q0) << ppVar(q1) << ppVar(q2) << ppVar(q3);

    QPointF r1;
    QPointF r2;

    std::tie(r1, r2) = KisBezierUtils::removeBezierNode(c0, c1, c2, c3, q1, q2, q3);

    QVERIFY(KisAlgebra2D::fuzzyPointCompare(r1, QPointF(121.314,120.167), 0.01));
    QVERIFY(KisAlgebra2D::fuzzyPointCompare(r2, QPointF(180.184,119.801), 0.01));
}

void KisMeshTransformWorkerTest::testIsIdentity()
{
    KisBezierMesh mesh(QRectF(0,0,100,100));

    QVERIFY(mesh.isIdentity());

    mesh.node(0,0).setRightControlRelative(QPointF(18, 0));
    mesh.node(0,0).setBottomControlRelative(QPointF(0, 18));

    /**
     * WISHLIST: in the current implementation even a slight change of the
     * mesh, which doesn't cause any deformations, will be considered as
     * making the mesh non-identity. Ideally, we could check the mesh for
     * mathematical transformation identity, but that seems to be a bit of
     * overkill for the current usecase (silently canceling transform tool
     * a action). Implementing this change is rather complicated because the
     * same functionality is used in the KisBezierGradientMesh.
     */
    QVERIFY(!mesh.isIdentity());

    mesh.node(0,0).setRightControlRelative(QPointF(10, -5));
    mesh.node(0,0).setBottomControlRelative(QPointF(-5, 10));

    QVERIFY(!mesh.isIdentity());

}

void KisMeshTransformWorkerTest::testSerialization()
{
    KisBezierTransformMesh mesh(QRectF(0,0,100,100));
    mesh.node(0,0).setRightControlRelative(QPointF(10, -5));
    mesh.node(0,0).setBottomControlRelative(QPointF(-5, 10));

    mesh.node(1,0).setLeftControlRelative(QPointF(-10, -5));
    mesh.node(1,0).setBottomControlRelative(QPointF(5, 10));

    mesh.node(0,1).setRightControlRelative(QPointF(10, 5));
    mesh.node(0,1).setTopControlRelative(QPointF(-5, -10));

    mesh.node(1,1).setLeftControlRelative(QPointF(-10, 5));
    mesh.node(1,1).setTopControlRelative(QPointF(5, -10));

    mesh.subdivideRow(0.5);
    mesh.subdivideColumn(0.5);

    QDomDocument doc;
    QDomElement e = doc.createElement("root");
    doc.appendChild(e);

    KisDomUtils::saveValue(&e, "mytransform", mesh);

    //printf("%s", doc.toString(4).toLatin1().data());

    KisBezierTransformMesh roundTripMesh;

    KisDomUtils::loadValue(e, "mytransform", &roundTripMesh);

    QCOMPARE(mesh, roundTripMesh);
}

void KisMeshTransformWorkerTest::testIteratorConstness()
{
    KisBezierTransformMesh mesh(QRectF(0,0,100,100));

    {
        auto controlIt = mesh.beginControlPoints();

        Q_STATIC_ASSERT((std::is_same<decltype(*controlIt), QPointF&>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(controlIt.node()), KisBezierTransformMesh::Node&>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(controlIt.topSegment().p0()), QPointF&>::value));

        auto constControlIt1 = mesh.constBeginControlPoints();

        Q_STATIC_ASSERT((std::is_same<decltype(*constControlIt1), const QPointF&>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(constControlIt1.node()), const KisBezierTransformMesh::Node&>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(constControlIt1.topSegment().p0()), const QPointF&>::value));

        auto constControlIt2 = std::as_const(mesh).beginControlPoints();

        Q_STATIC_ASSERT((std::is_same<decltype(*constControlIt2), const QPointF&>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(constControlIt2.node()), const KisBezierTransformMesh::Node&>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(constControlIt2.topSegment().p0()), const QPointF&>::value));
    }

    {
        auto segmentIt = mesh.beginSegments();

        Q_STATIC_ASSERT((std::is_same<decltype(segmentIt.p0()), QPointF&>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(segmentIt.firstNode()), KisBezierTransformMesh::Node&>::value));

        auto constSegmentIt1 = mesh.constBeginSegments();

        Q_STATIC_ASSERT((std::is_same<decltype(constSegmentIt1.p0()), const QPointF&>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(constSegmentIt1.firstNode()), const KisBezierTransformMesh::Node&>::value));

        auto constSegmentIt2 = std::as_const(mesh).beginSegments();

        Q_STATIC_ASSERT((std::is_same<decltype(constSegmentIt2.p0()), const QPointF&>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(constSegmentIt2.firstNode()), const KisBezierTransformMesh::Node&>::value));
    }

    {
        using NodeIndex = KisBezierTransformMesh::NodeIndex;
        using ControlPointIndex = KisBezierTransformMesh::ControlPointIndex;
        using ControlType = KisBezierTransformMesh::ControlType;

        auto controlIt = mesh.find(ControlPointIndex(NodeIndex(0,0), ControlType::Node));

        Q_STATIC_ASSERT((std::is_same<decltype(*controlIt), QPointF&>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(controlIt.node()), KisBezierTransformMesh::Node&>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(controlIt.topSegment().p0()), QPointF&>::value));

        auto constControlIt1 = mesh.constFind(ControlPointIndex(NodeIndex(0,0), ControlType::Node));;

        Q_STATIC_ASSERT((std::is_same<decltype(*constControlIt1), const QPointF&>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(constControlIt1.node()), const KisBezierTransformMesh::Node&>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(constControlIt1.topSegment().p0()), const QPointF&>::value));

        auto constControlIt2 = std::as_const(mesh).find(ControlPointIndex(NodeIndex(0,0), ControlType::Node));;

        Q_STATIC_ASSERT((std::is_same<decltype(*constControlIt2), const QPointF&>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(constControlIt2.node()), const KisBezierTransformMesh::Node&>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(constControlIt2.topSegment().p0()), const QPointF&>::value));
    }

    {
        using NodeIndex = KisBezierTransformMesh::NodeIndex;
        using SegmentIndex = KisBezierTransformMesh::SegmentIndex;
        using ControlPointIndex = KisBezierTransformMesh::ControlPointIndex;
        using ControlType = KisBezierTransformMesh::ControlType;

        auto segmentIt = mesh.find(SegmentIndex(NodeIndex(0,0), 1));

        Q_STATIC_ASSERT((std::is_same<decltype(segmentIt.p0()), QPointF&>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(segmentIt.firstNode()), KisBezierTransformMesh::Node&>::value));

        auto constSegmentIt1 = mesh.constFind(SegmentIndex(NodeIndex(0,0), 1));

        Q_STATIC_ASSERT((std::is_same<decltype(constSegmentIt1.p0()), const QPointF&>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(constSegmentIt1.firstNode()), const KisBezierTransformMesh::Node&>::value));

        auto constSegmentIt2 = std::as_const(mesh).find(SegmentIndex(NodeIndex(0,0), 1));

        Q_STATIC_ASSERT((std::is_same<decltype(constSegmentIt2.p0()), const QPointF&>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(constSegmentIt2.firstNode()), const KisBezierTransformMesh::Node&>::value));
    }

}

void KisMeshTransformWorkerTest::testLineCurveIntersections()
{

    QPointF p0(100,100);
    QPointF p1(110,110);
    QPointF p2(190,110);
    QPointF p3(200,100);

    QLineF line(QPointF(110,101), QPointF(160, 101));
    const qreal eps = 0.001;

    QVector<qreal> result = KisBezierUtils::intersectWithLine(p0, p1, p2, p3, line, eps);

    QCOMPARE(result.size(), 2);
    QVERIFY(KisAlgebra2D::fuzzyPointCompare(KisBezierUtils::bezierCurve(p0, p1, p2, p3, result[0]), QPointF(101.28,101), eps));
    QVERIFY(KisAlgebra2D::fuzzyPointCompare(KisBezierUtils::bezierCurve(p0, p1, p2, p3, result[1]), QPointF(198.72,101), eps));
}

QTEST_MAIN(KisMeshTransformWorkerTest)
