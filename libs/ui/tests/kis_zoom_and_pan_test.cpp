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

#include <cmath>
#include <QTest>

#include <kis_filter_configuration.h>
#include "testutil.h"
#include "qimage_based_test.h"

#include <kactioncollection.h>

#include "kis_config.h"

#include "KisMainWindow.h"
#include "KoZoomController.h"
#include "KisDocument.h"
#include "KisPart.h"
#include "KisViewManager.h"
#include "KisView.h"
#include "kis_canvas2.h"
#include "kis_canvas_controller.h"
#include "kis_coordinates_converter.h"
#include "kis_filter_strategy.h"

#include "kistest.h"

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
        QVERIFY(checkLayersInitial(m_image));

        m_doc = KisPart::instance()->createDocument();

        m_doc->setCurrentImage(m_image);

        m_mainWindow = KisPart::instance()->createMainWindow();
        m_view = new KisView(m_doc, m_mainWindow->resourceManager(), m_mainWindow->actionCollection(), m_mainWindow);

        m_image->refreshGraph();

        m_mainWindow->show();
    }

    ~ZoomAndPanTester() {
        m_image->waitForDone();
        QApplication::processEvents();

        delete m_mainWindow;
        delete m_doc;

        /**
         * The event queue may have up to 200k events
         * by the time all the tests are finished. Removing
         * all of them may last forever, so clear them after
         * every single test is finished
         */
        QApplication::removePostedEvents(0);
    }

    QPointer<KisView> view() {
        return m_view;
    }

    KisMainWindow* mainWindow() {
        return m_mainWindow;
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
    KisDocument *m_doc;
    QPointer<KisView>m_view;
    KisMainWindow *m_mainWindow;
};

template<class P, class T>
inline bool compareWithRounding(const P &pt0, const P &pt1, T tolerance)
{
    return qAbs(pt0.x() - pt1.x()) <= tolerance &&
        qAbs(pt0.y() - pt1.y()) <= tolerance;
}

bool verifyOffset(ZoomAndPanTester &t, const QPoint &offset) {

    if (t.coordinatesConverter()->documentOffset() != offset) {
            dbgKrita << "########################";
            dbgKrita << "Expected Offset:" << offset;
            dbgKrita << "Actual values:";
            dbgKrita << "Offset:" << t.coordinatesConverter()->documentOffset();
            dbgKrita << "wsize:"  << t.canvasWidget()->size();
            dbgKrita << "vport:"  << t.canvasController()->viewportSize();
            dbgKrita << "pref:"  << t.canvasController()->preferredCenter();
            dbgKrita << "########################";
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

        dbgKrita << "***** PAN *****************";

        if(!offsetAsExpected) {
            dbgKrita << " ### Offset invariant broken";
        }

        if(!preferredCenterAsExpected) {
            dbgKrita << " ### Preferred center invariant broken";
        }

        if(!topLeftAsExpected) {
            dbgKrita << " ### TopLeft invariant broken";
        }

        dbgKrita << ppVar(expectedOffset);
        dbgKrita << ppVar(expectedPrefCenter);
        dbgKrita << ppVar(oldOffset) << ppVar(newOffset);
        dbgKrita << ppVar(oldPrefCenter) << ppVar(newPrefCenter);
        dbgKrita << ppVar(newTopLeft);
        dbgKrita << "***************************";
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
                                        const QPointF &newTopLeft,
                                        const QSize &oldDocumentSize)
{
    qreal k = newZoom / oldZoom;

    QPointF expectedOffset = oldOffset + (k - 1) * baseFlakePoint;
    QPointF expectedPreferredCenter = oldPreferredCenter + (k - 1) * baseFlakePoint;

    qreal oldPreferredCenterFractionX = 1.0 * oldPreferredCenter.x() / oldDocumentSize.width();
    qreal oldPreferredCenterFractionY = 1.0 * oldPreferredCenter.y() / oldDocumentSize.height();

    qreal roundingTolerance =
        qMax(qreal(1.0), qMax(oldPreferredCenterFractionX, oldPreferredCenterFractionY) / k);

    /**
     * In the computation of the offset two roundings happen:
     * first for the computation of oldOffset and the second
     * for the computation of newOffset. So the maximum tolerance
     * should equal 2.
     */
    bool offsetAsExpected =
        compareWithRounding(expectedOffset, QPointF(newOffset), 2 * roundingTolerance);

    /**
     * Rounding for the preferred center happens due to the rounding
     * of the document size while zooming. The wider the step of the
     * zooming, the bigger tolerance should be
     */
    bool preferredCenterAsExpected =
        compareWithRounding(expectedPreferredCenter, newPreferredCenter,
                            roundingTolerance);

    bool topLeftAsExpected = newTopLeft.toPoint() == -newOffset;

    if (!offsetAsExpected ||
        !preferredCenterAsExpected ||
        !topLeftAsExpected) {

        dbgKrita << "***** ZOOM ****************";

        if(!offsetAsExpected) {
            dbgKrita << " ### Offset invariant broken";
        }

        if(!preferredCenterAsExpected) {
            dbgKrita << " ### Preferred center invariant broken";
        }

        if(!topLeftAsExpected) {
            dbgKrita << " ### TopLeft invariant broken";
        }

        dbgKrita << ppVar(expectedOffset);
        dbgKrita << ppVar(expectedPreferredCenter);
        dbgKrita << ppVar(oldOffset) << ppVar(newOffset);
        dbgKrita << ppVar(oldPreferredCenter) << ppVar(newPreferredCenter);
        dbgKrita << ppVar(oldPreferredCenterFractionX);
        dbgKrita << ppVar(oldPreferredCenterFractionY);
        dbgKrita << ppVar(oldZoom) << ppVar(newZoom);
        dbgKrita << ppVar(baseFlakePoint);
        dbgKrita << ppVar(newTopLeft);
        dbgKrita << ppVar(roundingTolerance);
        dbgKrita << "***************************";
    }

    return offsetAsExpected && preferredCenterAsExpected && topLeftAsExpected;
}

bool KisZoomAndPanTest::checkZoomWithAction(ZoomAndPanTester &t, qreal newZoom, bool limitedZoom)
{
    QPoint oldOffset = t.coordinatesConverter()->documentOffset();
    QPointF oldPrefCenter = t.canvasController()->preferredCenter();
    qreal oldZoom = t.zoomController()->zoomAction()->effectiveZoom();
    QSize oldDocumentSize = t.canvasController()->documentSize().toSize();

    t.zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, newZoom);

    QPointF newTopLeft = t.coordinatesConverter()->imageRectInWidgetPixels().topLeft();

    return checkInvariants(oldPrefCenter,
                           oldOffset,
                           oldPrefCenter,
                           oldZoom,
                           t.coordinatesConverter()->documentOffset(),
                           t.canvasController()->preferredCenter(),
                           limitedZoom ? oldZoom : newZoom,
                           newTopLeft,
                           oldDocumentSize);
}

bool KisZoomAndPanTest::checkZoomWithWheel(ZoomAndPanTester &t, const QPoint &widgetPoint, qreal zoomCoeff, bool limitedZoom)
{
    QPoint oldOffset = t.coordinatesConverter()->documentOffset();
    QPointF oldPrefCenter = t.canvasController()->preferredCenter();
    qreal oldZoom = t.zoomController()->zoomAction()->effectiveZoom();
    QSize oldDocumentSize = t.canvasController()->documentSize().toSize();

    t.canvasController()->zoomRelativeToPoint(widgetPoint, zoomCoeff);

    QPointF newTopLeft = t.coordinatesConverter()->imageRectInWidgetPixels().topLeft();

    return checkInvariants(oldOffset + widgetPoint,
                           oldOffset,
                           oldPrefCenter,
                           oldZoom,
                           t.coordinatesConverter()->documentOffset(),
                           t.canvasController()->preferredCenter(),
                           limitedZoom ? oldZoom : zoomCoeff * oldZoom,
                           newTopLeft,
                           oldDocumentSize);
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
    QCOMPARE(t.canvasWidget()->size(), t.canvasController()->viewportSize().toSize());
    QVERIFY(verifyOffset(t, QPoint(-171,-271)));

    t.canvasController()->resize(QSize(700,700));

    QCOMPARE(t.canvasWidget()->size(), QSize(683,683));
    QCOMPARE(t.canvasWidget()->size(), t.canvasController()->viewportSize().toSize());
    QVERIFY(verifyOffset(t, QPoint(-171,-271)));

    t.canvasController()->setPreferredCenter(QPoint(320,220));

    QVERIFY(verifyOffset(t, QPoint(-21,-121)));

    t.canvasController()->resize(QSize(400,400));

    QCOMPARE(t.canvasWidget()->size(), QSize(383,383));
    QCOMPARE(t.canvasWidget()->size(), t.canvasController()->viewportSize().toSize());
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
    QCOMPARE(t.canvasWidget()->size(), t.canvasController()->viewportSize().toSize());
    QVERIFY(verifyOffset(t, QPoint(79,-21)));

    if (fullscreenMode) {
        QCOMPARE(t.canvasController()->preferredCenter(), QPointF(320,220));

        QAction *action = t.view()->viewManager()->actionCollection()->action("view_show_canvas_only");
        action->setChecked(true);

        QVERIFY(verifyOffset(t, QPoint(79,-21)));
        QCOMPARE(t.canvasController()->preferredCenter(), QPointF(329,220));


        t.canvasController()->resize(QSize(483,483));
        QCOMPARE(t.canvasWidget()->size(), QSize(483,483));
        QCOMPARE(t.canvasWidget()->size(), t.canvasController()->viewportSize().toSize());
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
        QVERIFY(compareWithRounding(QPointF(220,320), t.canvasController()->preferredCenter(), 2));
        QCOMPARE(t.coordinatesConverter()->imageRectInWidgetPixels().topLeft().toPoint(), -t.coordinatesConverter()->documentOffset());
    }

    if (mirror) {
        t.canvasController()->mirrorCanvas(true);
        QVERIFY(verifyOffset(t, QPoint(78, -21)));
        QVERIFY(compareWithRounding(QPointF(320,220), t.canvasController()->preferredCenter(), 2));
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

void KisZoomAndPanTest::testZoomOnBorderZoomLevels()
{
    ZoomAndPanTester t;
    initializeViewport(t, false, false, false);

//    QPoint widgetPoint(100,100);

    warnKrita << "WARNING: testZoomOnBorderZoomLevels() is disabled due to some changes in KoZoomMode::minimum/maximumZoom()";
    return;

    // test min zoom level
    t.zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, KoZoomMode::minimumZoom());
    QVERIFY(checkZoomWithWheel(t, QPoint(100,100), 0.5, true));
    QVERIFY(checkZoomWithAction(t, KoZoomMode::minimumZoom() * 0.5, true));

    // test max zoom level
    t.zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, KoZoomMode::maximumZoom());
    QVERIFY(checkZoomWithWheel(t, QPoint(100,100), 2.0, true));
    QVERIFY(checkZoomWithAction(t, KoZoomMode::maximumZoom() * 2.0, true));
}

inline QTransform correctionMatrix(qreal angle)
{
    return QTransform(0,0,0,sin(M_PI * angle / 180),0,0,0,0,1);
}

bool KisZoomAndPanTest::checkRotation(ZoomAndPanTester &t, qreal angle)
{
    // save old values
    QPoint oldOffset = t.coordinatesConverter()->documentOffset();
    QPointF oldCenteringCorrection = t.coordinatesConverter()->centeringCorrection();
    QPointF oldPreferredCenter = t.canvasController()->preferredCenter();
    QPointF oldRealCenterPoint = t.coordinatesConverter()->widgetToImage(t.coordinatesConverter()->widgetCenterPoint());
    QSize oldDocumentSize = t.canvasController()->documentSize().toSize();

    qreal baseAngle = t.coordinatesConverter()->rotationAngle();
    t.canvasController()->rotateCanvas(angle);

    // save result values
    QPoint newOffset = t.coordinatesConverter()->documentOffset();
    QPointF newCenteringCorrection = t.coordinatesConverter()->centeringCorrection();
    QPointF newPreferredCenter = t.canvasController()->preferredCenter();
    QPointF newRealCenterPoint = t.coordinatesConverter()->widgetToImage(t.coordinatesConverter()->widgetCenterPoint());
    QSize newDocumentSize = t.canvasController()->documentSize().toSize();


    // calculate theoretical preferred center
    QTransform rot;
    rot.rotate(angle);

    QSizeF dSize = t.coordinatesConverter()->imageSizeInFlakePixels();
    QPointF dPoint(dSize.width(), dSize.height());

    QPointF expectedPreferredCenter =
        (oldPreferredCenter - dPoint * correctionMatrix(baseAngle)) * rot +
         dPoint * correctionMatrix(baseAngle + angle);

    // calculate theoretical offset based on the real preferred center
    QPointF wPoint(t.canvasWidget()->size().width(), t.canvasWidget()->size().height());
    QPointF expectedOldOffset = oldPreferredCenter - 0.5 * wPoint;
    QPointF expectedNewOffset = newPreferredCenter - 0.5 * wPoint;

    bool preferredCenterAsExpected =
        compareWithRounding(expectedPreferredCenter, newPreferredCenter, 2);
    bool oldOffsetAsExpected =
        compareWithRounding(expectedOldOffset + oldCenteringCorrection, QPointF(oldOffset), 2);
    bool newOffsetAsExpected =
        compareWithRounding(expectedNewOffset + newCenteringCorrection, QPointF(newOffset), 3);

    qreal zoom = t.zoomController()->zoomAction()->effectiveZoom();
    bool realCenterPointAsExpected =
        compareWithRounding(oldRealCenterPoint, newRealCenterPoint, 2/zoom);


    if (!oldOffsetAsExpected ||
        !newOffsetAsExpected ||
        !preferredCenterAsExpected ||
        !realCenterPointAsExpected) {

        dbgKrita << "***** ROTATE **************";

        if(!oldOffsetAsExpected) {
            dbgKrita << " ### Old offset invariant broken";
        }

        if(!newOffsetAsExpected) {
            dbgKrita << " ### New offset invariant broken";
        }

        if(!preferredCenterAsExpected) {
            dbgKrita << " ### Preferred center invariant broken";
        }

        if(!realCenterPointAsExpected) {
            dbgKrita << " ### *Real* center invariant broken";
        }

        dbgKrita << ppVar(expectedOldOffset);
        dbgKrita << ppVar(expectedNewOffset);
        dbgKrita << ppVar(expectedPreferredCenter);
        dbgKrita << ppVar(oldOffset) << ppVar(newOffset);
        dbgKrita << ppVar(oldCenteringCorrection) << ppVar(newCenteringCorrection);
        dbgKrita << ppVar(oldPreferredCenter) << ppVar(newPreferredCenter);
        dbgKrita << ppVar(oldRealCenterPoint) << ppVar(newRealCenterPoint);
        dbgKrita << ppVar(oldDocumentSize) << ppVar(newDocumentSize);
        dbgKrita << ppVar(baseAngle) << "deg";
        dbgKrita << ppVar(angle) << "deg";
        dbgKrita << "***************************";
    }

    return preferredCenterAsExpected && oldOffsetAsExpected && newOffsetAsExpected && realCenterPointAsExpected;
}

void KisZoomAndPanTest::testRotation(qreal vastScrolling, qreal zoom)
{
    KisConfig cfg(false);
    cfg.setVastScrolling(vastScrolling);

    ZoomAndPanTester t;

    QCOMPARE(t.image()->size(), QSize(640,441));
    QCOMPARE(t.image()->xRes(), 1.0);
    QCOMPARE(t.image()->yRes(), 1.0);

    QPointF preferredCenter = zoom * t.image()->bounds().center();

    t.canvasController()->resize(QSize(500,500));
    t.zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, zoom);
    t.canvasController()->setPreferredCenter(preferredCenter.toPoint());

    QCOMPARE(t.canvasWidget()->size(), QSize(483,483));
    QCOMPARE(t.canvasWidget()->size(), t.canvasController()->viewportSize().toSize());

    QPointF realCenterPoint = t.coordinatesConverter()->widgetToImage(t.coordinatesConverter()->widgetCenterPoint());
    QPointF expectedCenterPoint = QPointF(t.image()->bounds().center());

    if(!compareWithRounding(realCenterPoint, expectedCenterPoint, 2/zoom)) {
        dbgKrita << "Failed to set initial center point";
        dbgKrita << ppVar(expectedCenterPoint) << ppVar(realCenterPoint);
        QFAIL("FAIL: Failed to set initial center point");
    }

    QVERIFY(checkRotation(t, 30));
    QVERIFY(checkRotation(t, 20));
    QVERIFY(checkRotation(t, 10));
    QVERIFY(checkRotation(t, 5));
    QVERIFY(checkRotation(t, 5));
    QVERIFY(checkRotation(t, 5));

    if(vastScrolling < 0.5 && zoom < 1) {
        warnKrita << "Disabling a few tests for vast scrolling ="
                   << vastScrolling << ". See comment for more";
        /**
         * We have to disable a couple of tests here for the case when
         * vastScrolling value is 0.2. The problem is that the centering
         * correction applied  to the offset in
         * KisCanvasController::rotateCanvas pollutes the preferredCenter
         * value, because KoCnvasControllerWidget has no access to this
         * correction and cannot calculate the real value of the center of
         * the image. To fix this bug the calculation of correction
         * (aka "origin") should be moved to the KoCanvasControllerWidget
         * itself which would cause quite huge changes (including the change
         * of the external interface of it). Namely, we would have to
         * *calculate* offset from the value of the scroll bars, but not
         * use their values directly:
         *
         * offset = scrollBarValue - origin
         *
         * So now we just disable these unittests and allow a couple
         * of "jumping" bugs appear in vastScrolling < 0.5 modes, which
         * is, actually, not the default case.
         */

    } else {
        QVERIFY(checkRotation(t, 5));
        QVERIFY(checkRotation(t, 5));
        QVERIFY(checkRotation(t, 5));
    }
}

void KisZoomAndPanTest::testRotation_VastScrolling_1_0()
{
    testRotation(0.9, 1.0);
}

void KisZoomAndPanTest::testRotation_VastScrolling_0_5()
{
    testRotation(0.9, 0.5);
}

void KisZoomAndPanTest::testRotation_NoVastScrolling_1_0()
{
    testRotation(0.2, 1.0);
}

void KisZoomAndPanTest::testRotation_NoVastScrolling_0_5()
{
    testRotation(0.2, 0.5);
}

void KisZoomAndPanTest::testImageRescaled_0_5()
{
    ZoomAndPanTester t;
    QApplication::processEvents();
    initializeViewport(t, false, false, false);
    QApplication::processEvents();
    QVERIFY(checkPan(t, QPoint(200,200)));
    QApplication::processEvents();

    QPointF oldStillPoint =
        t.coordinatesConverter()->imageRectInWidgetPixels().center();

    KisFilterStrategy *strategy = new KisBilinearFilterStrategy();
    t.image()->scaleImage(QSize(320, 220), t.image()->xRes(), t.image()->yRes(), strategy);
    t.image()->waitForDone();
    QApplication::processEvents();
    delete strategy;

    QPointF newStillPoint =
        t.coordinatesConverter()->imageRectInWidgetPixels().center();

    QVERIFY(compareWithRounding(oldStillPoint, newStillPoint, 1.0));
}

void KisZoomAndPanTest::testImageCropped()
{
    ZoomAndPanTester t;
    QApplication::processEvents();
    initializeViewport(t, false, false, false);
    QApplication::processEvents();
    QVERIFY(checkPan(t, QPoint(-150,-150)));
    QApplication::processEvents();

    QPointF oldStillPoint =
        t.coordinatesConverter()->imageToWidget(QPointF(150,150));

    t.image()->cropImage(QRect(100,100,100,100));
    t.image()->waitForDone();
    QApplication::processEvents();

    QPointF newStillPoint =
        t.coordinatesConverter()->imageToWidget(QPointF(50,50));

    QVERIFY(compareWithRounding(oldStillPoint, newStillPoint, 1.0));
}

KISTEST_MAIN(KisZoomAndPanTest)
