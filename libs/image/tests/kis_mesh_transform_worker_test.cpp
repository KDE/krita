/*
 *  SPDX-FileCopyrightText: 2014, 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_mesh_transform_worker_test.h"

#include <simpletest.h>

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
#include "KisBezierPatchParamSpaceUtils.h"

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
//        using ControlPointIndex = KisBezierTransformMesh::ControlPointIndex;
//        using ControlType = KisBezierTransformMesh::ControlType;

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

void KisMeshTransformWorkerTest::testHitTestPatchInSourceSpace()
{
    KisBezierTransformMesh mesh(QRectF(50,50,200,100), QSize(4,4));

    QVERIFY(mesh.isIdentity());

    QRect rect;

    rect = mesh.hitTestPatchInSourceSpace(QRectF(60, 60, 10, 10));
    QCOMPARE(rect, QRect(0,0,1,1));

    rect = mesh.hitTestPatchInSourceSpace(QRectF(60, 60, 60, 10));
    QCOMPARE(rect, QRect(0,0,2,1));

    rect = mesh.hitTestPatchInSourceSpace(QRectF(60, 60, 60, 24));
    QCOMPARE(rect, QRect(0,0,2,2));

    rect = mesh.hitTestPatchInSourceSpace(QRectF(50, 60, 199.99, 24));
    QCOMPARE(rect, QRect(0,0,3,2));

    rect = mesh.hitTestPatchInSourceSpace(QRectF(50, 60, 200, 24));
    QCOMPARE(rect, QRect(0,0,3,2));

    rect = mesh.hitTestPatchInSourceSpace(QRectF(60, 50, 60, 100));
    QCOMPARE(rect, QRect(0,0,2,3));

    rect = mesh.hitTestPatchInSourceSpace(QRectF(50, 50, 200, 100));
    QCOMPARE(rect, QRect(0,0,3,3));
}


void KisMeshTransformWorkerTest::testParamToSourceSpace()
{
    using namespace KisBezierUtils;

    Range searchParamRange = {0.0, 1.0};
    Range searchSrcRange = {0, 100};
    auto func = [] (qreal param) -> Range {
        const qreal base = param * 100.0;
        return {std::max(0.0, base - 3), std::min(base + 3, 100.0)};
    };

    Range externalRange;
    Range internalRange;


    {
        Range rect = {60, 70};

        std::tie(externalRange, internalRange) =
            calcTightSrcRectRangeInParamSpace1D(searchParamRange, searchSrcRange, rect, func, 0.001);

        KIS_COMPARE_FLT(externalRange.start, 0.569992, 6);
        KIS_COMPARE_FLT(externalRange.end, 0.730003, 6);

        KIS_COMPARE_FLT(internalRange.start, 0.570007, 6);
        KIS_COMPARE_FLT(internalRange.end, 0.729996, 6);
    }

    {

        Range rect = {53, 70};

        std::tie(externalRange, internalRange) =
            calcTightSrcRectRangeInParamSpace1D(searchParamRange, searchSrcRange, rect, func, 0.001);

        KIS_COMPARE_FLT(externalRange.start, 0.499992, 6);
        KIS_COMPARE_FLT(externalRange.end, 0.730003, 6);

        KIS_COMPARE_FLT(internalRange.start, 0.500000, 6);
        KIS_COMPARE_FLT(internalRange.end, 0.729996, 6);
    }

    {

        Range rect = {0, 10};

        std::tie(externalRange, internalRange) =
            calcTightSrcRectRangeInParamSpace1D(searchParamRange, searchSrcRange, rect, func, 0.001);

        KIS_COMPARE_FLT(externalRange.start, 0.000000, 6);
        KIS_COMPARE_FLT(externalRange.end, 0.130005, 6);

        KIS_COMPARE_FLT(internalRange.start, 0.000000, 6);
        KIS_COMPARE_FLT(internalRange.end, 0.129883, 6);

    }

    {

        Range rect = {78, 100};

        std::tie(externalRange, internalRange) =
            calcTightSrcRectRangeInParamSpace1D(searchParamRange, searchSrcRange, rect, func, 0.001);

        KIS_COMPARE_FLT(externalRange.start, 0.749992, 6);
        KIS_COMPARE_FLT(externalRange.end, 1.000000, 6);

        KIS_COMPARE_FLT(internalRange.start, 0.750000, 6);
        KIS_COMPARE_FLT(internalRange.end, 1.00000, 6);

        /**
         * The second pass for better precision
         */
        std::tie(externalRange, internalRange) =
            calcTightSrcRectRangeInParamSpace1D(externalRange, searchSrcRange, rect, func, 0.001, Range{0.4, 0.6});

        KIS_COMPARE_FLT(externalRange.start, 0.773995, 6);
        KIS_COMPARE_FLT(externalRange.end, 1.000000, 6);

        KIS_COMPARE_FLT(internalRange.start, 0.77401, 6);
        KIS_COMPARE_FLT(internalRange.end, 1.00000, 6);
    }
}

void KisMeshTransformWorkerTest::testApproximateSourceToParam()
{
    KisBezierMesh mesh(QRectF(50, 50, 100, 100));
    KisBezierPatch patch = mesh.makePatch(0,0);
    const QRect rect(60, 60, 20, 20);


    const QRectF result = KisBezierTransformMesh::calcTightSrcRectRangeInParamSpace(patch, rect, 0.1);

    KIS_COMPARE_FLT(result.left(), 0.164063, 6);
    KIS_COMPARE_FLT(result.top(),  0.164063, 6);
    KIS_COMPARE_FLT(result.width(), 0.184570, 6);
    KIS_COMPARE_FLT(result.height(), 0.184570, 6);
}

void KisMeshTransformWorkerTest::testChangeRect()
{
    KisBezierTransformMesh mesh(QRectF(0,0,100,100));

    mesh.node(0,0).setRightControlRelative(QPointF(30, -5));
    mesh.node(0,0).setBottomControlRelative(QPointF(-5, 30));

    mesh.node(1,0).setLeftControlRelative(QPointF(-30, -5));
    mesh.node(1,0).setBottomControlRelative(QPointF(5, 30));

    mesh.node(0,1).setRightControlRelative(QPointF(30, 5));
    mesh.node(0,1).setTopControlRelative(QPointF(-5, -30));

    mesh.node(1,1).setLeftControlRelative(QPointF(-30, 5));
    mesh.node(1,1).setTopControlRelative(QPointF(5, -30));

    mesh.node(1,0).translate({30, -30});
    mesh.node(1,1).translate({30, 30});

    const QRect changeRect = mesh.approxChangeRect(QRect(60,60,20,20));
    QCOMPARE(changeRect, QRect(77,63,29,40));

    const QRect needRect = mesh.approxNeedRect(changeRect);
    QCOMPARE(needRect, QRect(59,58,22,29));
}

void KisMeshTransformWorkerTest::testComplexChangeRect()
{
    const QString meshString =
        "<root mode=\"5\">\n <mesh_transform>\n  <mesh type=\"transform-mesh\">\n   <size type=\"size\" h=\"3\" w=\"3\"/>\n   <srcRect type=\"rectf\" h=\"4917\" x=\"9\" y=\"15\" w=\"6984\"/>\n   <columns type=\"array\">\n    <item_0 type=\"value\" value=\"0\"/>\n    <item_1 type=\"value\" value=\"0.5\"/>\n    <item_2 type=\"value\" value=\"1\"/>\n   </columns>\n   <rows type=\"array\">\n    <item_0 type=\"value\" value=\"0\"/>\n    <item_1 type=\"value\" value=\"0.5\"/>\n    <item_2 type=\"value\" value=\"1\"/>\n   </rows>\n   <nodes type=\"array\">\n    <item_0 type=\"mesh-node\">\n     <node type=\"pointf\" x=\"9\" y=\"15\"/>\n     <left-control type=\"pointf\" x=\"-689.4\" y=\"15\"/>\n     <right-control type=\"pointf\" x=\"358.2\" y=\"15\"/>\n     <top-control type=\"pointf\" x=\"9\" y=\"-476.7\"/>\n     <bottom-control type=\"pointf\" x=\"9\" y=\"260.85\"/>\n    </item_0>\n    <item_1 type=\"mesh-node\">\n     <node type=\"pointf\" x=\"3309\" y=\"1097\"/>\n     <left-control type=\"pointf\" x=\"1833.6\" y=\"556\"/>\n     <right-control type=\"pointf\" x=\"4784.4\" y=\"1638\"/>\n     <top-control type=\"pointf\" x=\"3309\" y=\"605.3\"/>\n     <bottom-control type=\"pointf\" x=\"3207.44975545697\" y=\"1167.30800094127\"/>\n    </item_1>\n    <item_2 type=\"mesh-node\">\n     <node type=\"pointf\" x=\"6585\" y=\"2007\"/>\n     <left-control type=\"pointf\" x=\"5523.8\" y=\"2027\"/>\n     <right-control type=\"pointf\" x=\"7283.4\" y=\"2007\"/>\n     <top-control type=\"pointf\" x=\"6585\" y=\"1515.3\"/>\n     <bottom-control type=\"pointf\" x=\"6220.8171657967\" y=\"2211.25949394136\"/>\n    </item_2>\n    <item_3 type=\"mesh-node\">\n     <node type=\"pointf\" x=\"9\" y=\"2473.5\"/>\n     <left-control type=\"pointf\" x=\"-689.4\" y=\"2473.5\"/>\n     <right-control type=\"pointf\" x=\"358.2\" y=\"2473.5\"/>\n     <top-control type=\"pointf\" x=\"9\" y=\"1367.175\"/>\n     <bottom-control type=\"pointf\" x=\"9\" y=\"3579.825\"/>\n    </item_3>\n    <item_4 type=\"mesh-node\">\n     <node type=\"pointf\" x=\"1567.84324080161\" y=\"2475.28307326943\"/>\n     <left-control type=\"pointf\" x=\"90.3707868313418\" y=\"2463.2769491798\"/>\n     <right-control type=\"pointf\" x=\"3045.31569477187\" y=\"2487.28919735905\"/>\n     <top-control type=\"pointf\" x=\"1560.41977779106\" y=\"1790.41468651837\"/>\n     <bottom-control type=\"pointf\" x=\"1574.74764826163\" y=\"3112.26481506561\"/>\n    </item_4>\n    <item_5 type=\"mesh-node\">\n     <node type=\"pointf\" x=\"6656.56459647795\" y=\"2502.32499068488\"/>\n     <left-control type=\"pointf\" x=\"6307.36459647795\" y=\"2502.32499068488\"/>\n     <right-control type=\"pointf\" x=\"7354.96459647794\" y=\"2502.32499068488\"/>\n     <top-control type=\"pointf\" x=\"6622.19088022106\" y=\"2252.64079646939\"/>\n     <bottom-control type=\"pointf\" x=\"6683.38773428762\" y=\"2697.16320661377\"/>\n    </item_5>\n    <item_6 type=\"mesh-node\">\n     <node type=\"pointf\" x=\"9\" y=\"4932\"/>\n     <left-control type=\"pointf\" x=\"-689.4\" y=\"4932\"/>\n     <right-control type=\"pointf\" x=\"358.2\" y=\"4932\"/>\n     <top-control type=\"pointf\" x=\"9\" y=\"4686.15\"/>\n     <bottom-control type=\"pointf\" x=\"9\" y=\"5423.7\"/>\n    </item_6>\n    <item_7 type=\"mesh-node\">\n     <node type=\"pointf\" x=\"3317\" y=\"3972\"/>\n     <left-control type=\"pointf\" x=\"1837.6\" y=\"4452\"/>\n     <right-control type=\"pointf\" x=\"4796.4\" y=\"3492\"/>\n     <top-control type=\"pointf\" x=\"3229.2966556471\" y=\"3810.23069945457\"/>\n     <bottom-control type=\"pointf\" x=\"3317\" y=\"4463.7\"/>\n    </item_7>\n    <item_8 type=\"mesh-node\">\n     <node type=\"pointf\" x=\"6625\" y=\"3012\"/>\n     <left-control type=\"pointf\" x=\"5471.8\" y=\"3032\"/>\n     <right-control type=\"pointf\" x=\"7323.4\" y=\"3012\"/>\n     <top-control type=\"pointf\" x=\"6225.45548890306\" y=\"2781.2358553733\"/>\n     <bottom-control type=\"pointf\" x=\"6625\" y=\"3503.7\"/>\n    </item_8>\n   </nodes>\n  </mesh>\n </mesh_transform>\n</root>\n";
    const QRect parentLayerRect = QRect(0, 0, 7016, 4961);

    QDomDocument doc;
    doc.setContent(meshString);

    QDomElement e = doc.documentElement();

    QDomElement meshEl;

    bool result =
        KisDomUtils::findOnlyElement(e, "mesh_transform", &meshEl);


    QVERIFY(result);
    QVERIFY(!meshEl.isNull());

    KisBezierTransformMesh mesh;

    result = KisDomUtils::loadValue(meshEl, "mesh", &mesh);

    QVERIFY(result);
    QVERIFY(!mesh.isIdentity());
    QCOMPARE(mesh.size(), QSize(3, 3));

    QRect changeRect = mesh.approxChangeRect(parentLayerRect);

    QCOMPARE(changeRect, QRect(9,15,6648,4917));
    QVERIFY(mesh.dstBoundingRect().contains(changeRect));

}

SIMPLE_TEST_MAIN(KisMeshTransformWorkerTest)
