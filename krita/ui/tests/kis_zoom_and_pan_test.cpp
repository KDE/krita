/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_zoom_and_pan_test.h"

#include <qtest_kde.h>

#include "testutil.h"
#include "qimage_based_test.h"

#include <kactioncollection.h>

#include "KoMainWindow.h"
#include "KoZoomController.h"
#include "kis_doc2.h"
#include "kis_part2.h"
#include "kis_view2.h"
#include "kis_canvas2.h"
#include "kis_canvas_controller.h"
#include "kis_coordinates_converter.h"


class ZoomAndPanTester : public TestUtil::QImageBasedTest
{
public:
    ZoomAndPanTester()
        // we are not going to use our own QImage sets,so
        // just exploit the set of the selection manager test
        : QImageBasedTest("selection_manager_test")
    {
        m_undoStore = new KisSurrogateUndoStore();
        m_image = createImage(m_undoStore);
        m_image->initialRefreshGraph();
        QVERIFY(checkLayers(m_image, "initial"));

        m_part = new KisPart2(0);

        m_doc = new KisDoc2();
        m_doc->setCurrentImage(m_image);

        m_part->setDocument(m_doc);

        m_shell = new KoMainWindow(m_part->componentData());
        m_view = new KisView2(m_part, m_doc, m_shell);

        m_image->refreshGraph();

        m_shell->show();
    }

    ~ZoomAndPanTester() {
        m_image->waitForDone();

        delete m_shell;
        delete m_part;
        delete m_doc;
    }

    KisView2* view() {
        return m_view;
    }

    KoMainWindow* shell() {
        return m_shell;
    }

    KisImageWSP image() {
        return m_image;
    }

    KisCanvas2* canvas() {
        return m_view->canvasBase();
    }

    QWidget* canvasWidget() {
        return m_view->canvasBase()->canvasWidget();
    }

    KoZoomController* zoomController() {
        return m_view->zoomController();
    }

    KisCanvasController* canvasController() {
        return dynamic_cast<KisCanvasController*>(m_view->canvasController());
    }

    const KisCoordinatesConverter* coordinatesConverter() {
        return m_view->canvasBase()->coordinatesConverter();
    }

private:
    KisSurrogateUndoStore *m_undoStore;
    KisImageSP m_image;
    KisPart2 *m_part;
    KisDoc2 *m_doc;
    KisView2 *m_view;
    KoMainWindow *m_shell;
};

template<class P, class T>
inline bool compareWithRounding(const P &pt0, const P &pt1, T tolerance)
{
    return qAbs(pt0.x() - pt1.x()) < tolerance &&
        qAbs(pt0.y() - pt1.y()) < tolerance;
}

bool verifyOffset(ZoomAndPanTester &t, const QPoint &offset) {

    if (t.coordinatesConverter()->documentOffset() != offset) {
            qDebug() << "########################";
            qDebug() << "Expected Offset:" << offset;
            qDebug() << "Actual values:";
            qDebug() << "Offset:" << t.coordinatesConverter()->documentOffset();
            qDebug() << "Origin:" << t.coordinatesConverter()->documentOrigin();
            qDebug() << "wsize:"  << t.canvasWidget()->size();
            qDebug() << "vport:"  << t.canvasController()->viewportSize();
            qDebug() << "pref:"  << t.canvasController()->preferredCenter();
            qDebug() << "########################";
    }

    return t.coordinatesConverter()->documentOffset() == offset;
}

bool KisZoomAndPanTest::checkPan(ZoomAndPanTester &t, QPoint shift)
{
    QPoint oldOffset = t.coordinatesConverter()->documentOffset();
    QPointF oldPrefCenter = t.canvasController()->preferredCenter();

    t.canvasController()->pan(shift);

    QPoint newOffset  = t.coordinatesConverter()->documentOffset();
    QPointF newPrefCenter = t.canvasController()->preferredCenter();
    QPointF newTopLeft = t.coordinatesConverter()->imageRectInWidgetPixels().topLeft();

    QPoint expectedOffset  = oldOffset + shift;
    QPointF expectedPrefCenter = oldPrefCenter + shift;


    // no tolerance accepted for pan
    bool offsetAsExpected = newOffset == expectedOffset;

    // rounding can happen due to the scroll bars being the main
    // source of the offset
    bool preferredCenterAsExpected =
        compareWithRounding(expectedPrefCenter, newPrefCenter, 1.0);

    bool topLeftAsExpected = newTopLeft.toPoint() == -newOffset;

    if (!offsetAsExpected ||
        !preferredCenterAsExpected ||
        !topLeftAsExpected) {

        qDebug() << "***** PAN *****************";

        if(!offsetAsExpected) {
            qDebug() << " ### Offset invariant broken";
        }

        if(!preferredCenterAsExpected) {
            qDebug() << " ### Preferred center invariant broken";
        }

        if(!topLeftAsExpected) {
            qDebug() << " ### TopLeft invariant broken";
        }

        qDebug() << ppVar(expectedOffset);
        qDebug() << ppVar(expectedPrefCenter);
        qDebug() << ppVar(oldOffset) << ppVar(newOffset);
        qDebug() << ppVar(oldPrefCenter) << ppVar(newPrefCenter);
        qDebug() << ppVar(newTopLeft);
        qDebug() << "***************************";
    }

    return offsetAsExpected && preferredCenterAsExpected && topLeftAsExpected;
}

bool KisZoomAndPanTest::checkInvariants(const QPointF &baseFlakePoint,
                                        const QPoint &oldOffset,
                                        const QPointF &oldPreferredCenter,
                                        qreal oldZoom,
                                        const QPoint &newOffset,
                                        const QPointF &newPreferredCenter,
                                        qreal newZoom,
                                        QPointF newTopLeft)
{
    qreal k = newZoom / oldZoom;

    QPoint expectedOffset = oldOffset + ((k - 1) * baseFlakePoint).toPoint();
    QPointF expectedPreferredCenter = oldPreferredCenter + (k - 1) * baseFlakePoint;

    /**
     * In the computation of the offset two roundings happen:
     * first for the computation of oldOffset and the second
     * for the computation of newOffset. So the maximum tolerance
     * should equal 2.
     */
    bool offsetAsExpected =
        compareWithRounding(expectedOffset, newOffset, 2);

    /**
     * Rounding for the preferred center happens due to the rounding
     * of the document size while zooming. The wider the step of the
     * zooming, the bigger tolerance should be
     */
    bool preferredCenterAsExpected =
        compareWithRounding(expectedPreferredCenter, newPreferredCenter,
                            qMax(1.0, k));

    bool topLeftAsExpected = newTopLeft.toPoint() == -newOffset;

    if (!offsetAsExpected ||
        !preferredCenterAsExpected ||
        !topLeftAsExpected) {

        qDebug() << "***** ZOOM ****************";

        if(!offsetAsExpected) {
            qDebug() << " ### Offset invariant broken";
        }

        if(!preferredCenterAsExpected) {
            qDebug() << " ### Preferred center invariant broken";
        }

        if(!topLeftAsExpected) {
            qDebug() << " ### TopLeft invariant broken";
        }

        qDebug() << ppVar(expectedOffset);
        qDebug() << ppVar(expectedPreferredCenter);
        qDebug() << ppVar(oldOffset) << ppVar(newOffset);
        qDebug() << ppVar(oldPreferredCenter) << ppVar(newPreferredCenter);
        qDebug() << ppVar(oldZoom) << ppVar(newZoom);
        qDebug() << ppVar(baseFlakePoint);
        qDebug() << ppVar(newTopLeft);
        qDebug() << "***************************";
    }

    return offsetAsExpected && preferredCenterAsExpected && topLeftAsExpected;
}

bool KisZoomAndPanTest::checkZoomWithAction(ZoomAndPanTester &t, qreal newZoom)
{
    QPoint oldOffset = t.coordinatesConverter()->documentOffset();
    QPointF oldPrefCenter = t.canvasController()->preferredCenter();
    qreal oldZoom = t.zoomController()->zoomAction()->effectiveZoom();

    t.zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, newZoom);

    QPointF newTopLeft = t.coordinatesConverter()->imageRectInWidgetPixels().topLeft();

    return checkInvariants(oldPrefCenter,
                           oldOffset,
                           oldPrefCenter,
                           oldZoom,
                           t.coordinatesConverter()->documentOffset(),
                           t.canvasController()->preferredCenter(),
                           newZoom,
                           newTopLeft);
}

bool KisZoomAndPanTest::checkZoomWithWheel(ZoomAndPanTester &t, const QPoint &widgetPoint, qreal zoomCoeff)
{
    QPoint oldOffset = t.coordinatesConverter()->documentOffset();
    QPointF oldPrefCenter = t.canvasController()->preferredCenter();
    qreal oldZoom = t.zoomController()->zoomAction()->effectiveZoom();

    t.canvasController()->zoomRelativeToPoint(widgetPoint, zoomCoeff);

    QPointF newTopLeft = t.coordinatesConverter()->imageRectInWidgetPixels().topLeft();

    return checkInvariants(oldOffset + widgetPoint,
                           oldOffset,
                           oldPrefCenter,
                           oldZoom,
                           t.coordinatesConverter()->documentOffset(),
                           t.canvasController()->preferredCenter(),
                           zoomCoeff * oldZoom,
                           newTopLeft);
}

void KisZoomAndPanTest::testZoom100ChangingWidgetSize()
{
    ZoomAndPanTester t;

    QCOMPARE(t.image()->size(), QSize(640,441));
    QCOMPARE(t.image()->xRes(), 1.0);
    QCOMPARE(t.image()->yRes(), 1.0);

    t.canvasController()->resize(QSize(1000,1000));
    t.zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, 1.0);
    t.canvasController()->setPreferredCenter(QPoint(320,220));

    QCOMPARE(t.canvasWidget()->size(), QSize(983,983));
    QCOMPARE(t.canvasWidget()->size(), t.canvasController()->viewportSize());
    QVERIFY(verifyOffset(t, QPoint(-171,-271)));

    t.canvasController()->resize(QSize(700,700));

    QCOMPARE(t.canvasWidget()->size(), QSize(683,683));
    QCOMPARE(t.canvasWidget()->size(), t.canvasController()->viewportSize());
    QVERIFY(verifyOffset(t, QPoint(-171,-271)));

    t.canvasController()->setPreferredCenter(QPoint(320,220));

    QVERIFY(verifyOffset(t, QPoint(-21,-121)));

    t.canvasController()->resize(QSize(400,400));

    QCOMPARE(t.canvasWidget()->size(), QSize(383,383));
    QCOMPARE(t.canvasWidget()->size(), t.canvasController()->viewportSize());
    QVERIFY(verifyOffset(t, QPoint(-21,-121)));

    t.canvasController()->setPreferredCenter(QPoint(320,220));

    QVERIFY(verifyOffset(t, QPoint(129,29)));

    t.canvasController()->pan(QPoint(100,100));

    QVERIFY(verifyOffset(t, QPoint(229,129)));
}

void KisZoomAndPanTest::initializeViewport(ZoomAndPanTester &t, bool fullscreenMode, bool rotate, bool mirror)
{
    QCOMPARE(t.image()->size(), QSize(640,441));
    QCOMPARE(t.image()->xRes(), 1.0);
    QCOMPARE(t.image()->yRes(), 1.0);

    t.canvasController()->resize(QSize(500,500));
    t.zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, 1.0);
    t.canvasController()->setPreferredCenter(QPoint(320,220));

    QCOMPARE(t.canvasWidget()->size(), QSize(483,483));
    QCOMPARE(t.canvasWidget()->size(), t.canvasController()->viewportSize());
    QVERIFY(verifyOffset(t, QPoint(79,-21)));

    if (fullscreenMode) {
        QCOMPARE(t.canvasController()->preferredCenter(), QPointF(320,220));

        QAction *action = t.view()->actionCollection()->action("view_show_just_the_canvas");
        action->setChecked(true);

        QVERIFY(verifyOffset(t, QPoint(79,-21)));
        QCOMPARE(t.canvasController()->preferredCenter(), QPointF(329,220));


        t.canvasController()->resize(QSize(483,483));
        QCOMPARE(t.canvasWidget()->size(), QSize(483,483));
        QCOMPARE(t.canvasWidget()->size(), t.canvasController()->viewportSize());
        QVERIFY(verifyOffset(t, QPoint(79,-21)));


        /**
         * FIXME: here is a small flaw in KoCanvasControllerWidget
         * We cannot set the center point explicitly, because it'll be rounded
         * up by recenterPreferred function, so real center point will be
         * different. Make the preferredCenter() return real center of the
         * image instead of the set value
         */
        QCOMPARE(t.canvasController()->preferredCenter(), QPointF(320.5,220));
    }

    if (rotate) {
        t.canvasController()->rotateCanvas(90);
        QVERIFY(verifyOffset(t, QPoint(-21,79)));
        QCOMPARE(t.canvasController()->preferredCenter(), QPointF(220.5,320.5));
        QCOMPARE(t.coordinatesConverter()->imageRectInWidgetPixels().topLeft().toPoint(), -t.coordinatesConverter()->documentOffset());
    }

    if (mirror) {
        t.canvasController()->mirrorCanvas(true);
        QVERIFY(verifyOffset(t, QPoint(78, -21)));
        QCOMPARE(t.canvasController()->preferredCenter(), QPointF(319.5,220));
        QCOMPARE(t.coordinatesConverter()->imageRectInWidgetPixels().topLeft().toPoint(), -t.coordinatesConverter()->documentOffset());
    }
}

void KisZoomAndPanTest::testSequentialActionZoomAndPan(bool fullscreenMode, bool rotate, bool mirror)
{
    ZoomAndPanTester t;
    initializeViewport(t, fullscreenMode, rotate, mirror);

    QVERIFY(checkZoomWithAction(t, 0.5));
    QVERIFY(checkPan(t, QPoint(100,100)));

    QVERIFY(checkZoomWithAction(t, 0.25));
    QVERIFY(checkPan(t, QPoint(-100,-100)));

    QVERIFY(checkZoomWithAction(t, 0.35));
    QVERIFY(checkPan(t, QPoint(100,100)));

    QVERIFY(checkZoomWithAction(t, 0.45));
    QVERIFY(checkPan(t, QPoint(100,100)));

    QVERIFY(checkZoomWithAction(t, 0.85));
    QVERIFY(checkPan(t, QPoint(-100,-100)));

    QVERIFY(checkZoomWithAction(t, 2.35));
    QVERIFY(checkPan(t, QPoint(100,100)));
}

void KisZoomAndPanTest::testSequentialWheelZoomAndPan(bool fullscreenMode, bool rotate, bool mirror)
{
    ZoomAndPanTester t;
    initializeViewport(t, fullscreenMode, rotate, mirror);

    QVERIFY(checkZoomWithWheel(t, QPoint(100,100), 0.5));
    QVERIFY(checkPan(t, QPoint(100,100)));

    QVERIFY(checkZoomWithWheel(t, QPoint(100,100), 0.5));
    QVERIFY(checkPan(t, QPoint(-100,-100)));

    QVERIFY(checkZoomWithWheel(t, QPoint(100,100), 1.25));
    QVERIFY(checkPan(t, QPoint(100,100)));

    QVERIFY(checkZoomWithWheel(t, QPoint(100,100), 1.5));
    QVERIFY(checkPan(t, QPoint(100,100)));

    QVERIFY(checkZoomWithWheel(t, QPoint(100,100), 2.5));
    QVERIFY(checkPan(t, QPoint(-100,-100)));

    // check one point which is outside the widget
    QVERIFY(checkZoomWithWheel(t, QPoint(-100,100), 2.5));
    QVERIFY(checkPan(t, QPoint(-100,-100)));

    QVERIFY(checkZoomWithWheel(t, QPoint(100,100), 0.5));
    QVERIFY(checkPan(t, QPoint(-100,-100)));
}

void KisZoomAndPanTest::testSequentialActionZoomAndPan()
{
    testSequentialActionZoomAndPan(false, false, false);
}

void KisZoomAndPanTest::testSequentialActionZoomAndPanFullscreen()
{
    testSequentialActionZoomAndPan(true, false, false);
}

void KisZoomAndPanTest::testSequentialActionZoomAndPanRotate()
{
    testSequentialActionZoomAndPan(false, true, false);
}

void KisZoomAndPanTest::testSequentialActionZoomAndPanRotateFullscreen()
{
    testSequentialActionZoomAndPan(true, true, false);
}

void KisZoomAndPanTest::testSequentialActionZoomAndPanMirror()
{
    testSequentialActionZoomAndPan(false, false, true);
}

void KisZoomAndPanTest::testSequentialWheelZoomAndPan()
{
    testSequentialWheelZoomAndPan(false, false, false);
}

void KisZoomAndPanTest::testSequentialWheelZoomAndPanFullscreen()
{
    testSequentialWheelZoomAndPan(true, false, false);
}

void KisZoomAndPanTest::testSequentialWheelZoomAndPanRotate()
{
    testSequentialWheelZoomAndPan(false, true, false);
}

void KisZoomAndPanTest::testSequentialWheelZoomAndPanRotateFullscreen()
{
    testSequentialWheelZoomAndPan(true, true, false);
}

void KisZoomAndPanTest::testSequentialWheelZoomAndPanMirror()
{
    testSequentialWheelZoomAndPan(false, false, true);
}



QTEST_KDEMAIN(KisZoomAndPanTest, GUI)
