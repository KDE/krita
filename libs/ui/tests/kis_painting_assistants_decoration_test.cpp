/*
 * This file contains tests for:
 *
 * - KisPaintingAssistantsDecoration
 * - ParallelRulerAssistant
 * - PerspectiveAssistant
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <testutil.h>
#include "qimage_based_test.h"

#include <KoCanvasResourceProvider.h>
#include "kis_canvas_resource_provider.h"
#include <util.h>
#include <KisMainWindow.h>
#include <KisDocument.h>
#include <KisPart.h>
#include <KisView.h>
#include <KisViewManager.h>
#include <KisDecorationsManager.h>

#include "kis_painting_assistants_decoration.h"
#include "KisPart.h"
#include "testui.h"
#include <kis_config.h>
#include <testutil.h>
#include <KoResourcePaths.h>
#include "opengl/kis_opengl.h"
#include "kis_painting_assistants_decoration_test.h"

void KisPaintingAssistantsDecorationTest::initTestCase()
{
    /* A bit of a heavy handed way to force the plugings to be loaded and get
     * access to the paintingAssistantsDecoration but can't find another way
     * at the moment
     */
    m_document = createEmptyDocument();
    m_mainWindow = KisPart::instance()->createMainWindow();
    m_view = new KisView(m_document, m_mainWindow->viewManager(), m_mainWindow);
    m_viewManager = new KisViewManager(m_mainWindow, m_mainWindow->actionCollection());
    QApplication::processEvents();
}


void KisPaintingAssistantsDecorationTest::cleanupTestCase()
{
    /* Clean-up properly otherwise there will be an
     * assert during application exit.
     */
    m_mainWindow->hide();
    QApplication::processEvents();
    delete m_view;
    delete m_document;
    delete m_mainWindow;
}

/*
 * Test Perspective Assistant construction and using it for brush position
 * Adjustment
 */
void KisPaintingAssistantsDecorationTest::testPerspectiveAssistant()
{
    KisPaintingAssistantsDecorationSP paintingAssistantsDecoration =
        m_view->canvasBase()->paintingAssistantsDecoration();

    const KisPaintingAssistantFactoryRegistry* r = KisPaintingAssistantFactoryRegistry::instance();
    QVERIFY(r);

    KisPaintingAssistantFactory* factory = r->get("perspective");
    QVERIFY(factory);

    KisPaintingAssistantSP assistant = toQShared(factory->createPaintingAssistant());

    QVERIFY(assistant->isAssistantComplete() == false);

    KisPaintingAssistantHandleSP h1 = new KisPaintingAssistantHandle(10.0, 10.0);
    KisPaintingAssistantHandleSP h2 = new KisPaintingAssistantHandle(20.0, 90.0);
    KisPaintingAssistantHandleSP h3 = new KisPaintingAssistantHandle(60.0, 90.0);
    KisPaintingAssistantHandleSP h4 = new KisPaintingAssistantHandle(70.0, 10.0);

    assistant->addHandle(h1, NORMAL);
    assistant->addHandle(h2, NORMAL);
    assistant->addHandle(h3, NORMAL);
    assistant->addHandle(h4, NORMAL);

    QVERIFY(assistant->isAssistantComplete() == true);

    // Move straight up.
    QPointF startPosition(14.0, 14.0);
    QPointF p2(14.0, 90.0);
    QPointF adjustedPosition;
    // Compute twice to make sure state variables get taken into account
    adjustedPosition = assistant->adjustPosition(p2, startPosition);
    adjustedPosition = assistant->adjustPosition(p2, startPosition);
    // Should lead to a adjustment
    QVERIFY(adjustedPosition != p2);
}


/*
 * Test adjusting the brush position based on one or more Parallel rulers via
 * the PaintingAssistantDecoration.
 */
void KisPaintingAssistantsDecorationTest::testParallelRulerAdjustPosition()
{
    KisPaintingAssistantsDecorationSP paintingAssistantsDecoration =
        m_view->canvasBase()->paintingAssistantsDecoration();

    QVERIFY(paintingAssistantsDecoration->isEditingAssistants() == false);

    const KisPaintingAssistantFactoryRegistry* r = KisPaintingAssistantFactoryRegistry::instance();
    QVERIFY(r);

    KisPaintingAssistantFactory* factory = r->get("parallel ruler");
    QVERIFY(factory);

    KisPaintingAssistantSP assistant = toQShared(factory->createPaintingAssistant());

    QVERIFY(assistant->isAssistantComplete() == false);

    // A horizontal parallel ruler
    KisPaintingAssistantHandleSP h1 = new KisPaintingAssistantHandle(10.0, 10.0);
    KisPaintingAssistantHandleSP h2 = new KisPaintingAssistantHandle(90.0, 10.0);
    assistant->addHandle(h1, NORMAL);
    assistant->addHandle(h2, NORMAL);
    // A parallel ruler assistant needs two handles so now it should be
    // complete
    QVERIFY(assistant->numHandles() == 2);
    QVERIFY(assistant->isAssistantComplete() == true);
    // The assistant is now complete and can be used for computing brush
    // position adjustments


    // Move in a slanted direction
    QPointF startPosition(10.0, 50.0);
    QPointF p2(90.0, 90.0);
    QPointF adjustedPosition;

    // When the paintingAssistantsDecoration does not contain any assistants
    // there should be no position adjustments.
    // (Test twice to make sure state variables get taken into account).
    adjustedPosition = paintingAssistantsDecoration->adjustPosition(p2, startPosition);
    QVERIFY(adjustedPosition == p2);
    adjustedPosition = paintingAssistantsDecoration->adjustPosition(p2, startPosition);
    QVERIFY(adjustedPosition == p2);

    // Add an assistant
    paintingAssistantsDecoration->addAssistant(assistant);

    // First compute directly via the assisant
    adjustedPosition = assistant->adjustPosition(p2, startPosition);
    // The position should be adjusted to the horizontal direction
    QVERIFY(adjustedPosition.x() == 90.0);
    QVERIFY(adjustedPosition.y() == 50.0);
    // Compute the same adjustment via the PaintingAssistantDecoration
    QPointF adjustedPosition2 = paintingAssistantsDecoration->adjustPosition(p2, startPosition);
    // Resulting positions should match
    QVERIFY(adjustedPosition == adjustedPosition2);

    // Add two more assistants

    // (1) A vertical parallel ruler
    KisPaintingAssistantSP assistant_2 = toQShared(factory->createPaintingAssistant());
    KisPaintingAssistantHandleSP h3 = new KisPaintingAssistantHandle(10.0, 10.0);
    KisPaintingAssistantHandleSP h4 = new KisPaintingAssistantHandle(10.0, 90.0);
    assistant_2->addHandle(h3, NORMAL);
    assistant_2->addHandle(h4, NORMAL);

    // (2) A 45 degree parallel ruler
    KisPaintingAssistantSP assistant_3 = toQShared(factory->createPaintingAssistant());
    KisPaintingAssistantHandleSP h5 = new KisPaintingAssistantHandle(10.0, 10.0);
    KisPaintingAssistantHandleSP h6 = new KisPaintingAssistantHandle(90.0, 90.0);
    assistant_3->addHandle(h5, NORMAL);
    assistant_3->addHandle(h6, NORMAL);

    paintingAssistantsDecoration->addAssistant(assistant_2);
    paintingAssistantsDecoration->addAssistant(assistant_3);

    // startPosition == 10.0, 50.0
    QPointF p3(11.0, 50.1); // Less than |4| from the start position
    QPointF p4(15.0, 50.1); // More than |4| from the start position
    QPointF p5(18.0, 51.1); // Almost horizontal line
    QPointF p6(11.0, 65.1); // Almost vertical line

    // Switch to the more were we continuously snap to the best matching
    // assistant
    paintingAssistantsDecoration->setOnlyOneAssistantSnap(false);
    adjustedPosition = paintingAssistantsDecoration->adjustPosition(p3, startPosition);

    adjustedPosition = paintingAssistantsDecoration->adjustPosition(p4, startPosition);
    adjustedPosition = paintingAssistantsDecoration->adjustPosition(p5, startPosition);
    // Horizontal ruler should be the best match
    QVERIFY(adjustedPosition.y() == 50.0);
    adjustedPosition = paintingAssistantsDecoration->adjustPosition(p6, startPosition);
    // Vertical ruler should be the best match
    QVERIFY(adjustedPosition.x() == 10.0);
    paintingAssistantsDecoration->endStroke();

    // Switch to the more were first find the best matching
    // assistant and then keep using it
    paintingAssistantsDecoration->setOnlyOneAssistantSnap(true);
    adjustedPosition = paintingAssistantsDecoration->adjustPosition(p3, startPosition);
    adjustedPosition = paintingAssistantsDecoration->adjustPosition(p4, startPosition);
    adjustedPosition = paintingAssistantsDecoration->adjustPosition(p5, startPosition);
    // Horizontal ruler should be the best match
    QVERIFY(adjustedPosition.y() == 50.0);
    adjustedPosition = paintingAssistantsDecoration->adjustPosition(p6, startPosition);
    // Horizontal ruler should still be the best match, even though we moved
    // vertically
    QVERIFY(adjustedPosition.y() == 50.0);
    paintingAssistantsDecoration->endStroke();

}


KISTEST_MAIN(KisPaintingAssistantsDecorationTest)

