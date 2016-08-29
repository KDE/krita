/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_derived_resources_test.h"

#include <QTest>
#include <brushengine/kis_paintop_preset.h>

#include <QApplication>

#include <KoCanvasResourceManager.h>

#include "kis_canvas_resource_provider.h"
#include <util.h>
#include <KisMainWindow.h>
#include <KisDocument.h>
#include <KisPart.h>
#include <KisView.h>
#include <KisViewManager.h>
#include <kis_paintop_settings.h>
#include <KoResourcePaths.h>

#include "testutil.h"

void addResourceTypes()
{
    // All Krita's resource types
    KoResourcePaths::addResourceType("kis_pics", "data", "/pics/");
    KoResourcePaths::addResourceType("kis_images", "data", "/images/");
    KoResourcePaths::addResourceType("icc_profiles", "data", "/profiles/");
    KoResourcePaths::addResourceType("metadata_schema", "data", "/metadata/schemas/");
    KoResourcePaths::addResourceType("kis_brushes", "data", "/brushes/");
    KoResourcePaths::addResourceType("kis_taskset", "data", "/taskset/");
    KoResourcePaths::addResourceType("kis_taskset", "data", "/taskset/");
    KoResourcePaths::addResourceType("gmic_definitions", "data", "/gmic/");
    KoResourcePaths::addResourceType("kis_resourcebundles", "data", "/bundles/");
    KoResourcePaths::addResourceType("kis_defaultpresets", "data", "/defaultpresets/");
    KoResourcePaths::addResourceType("kis_paintoppresets", "data", "/paintoppresets/");
    KoResourcePaths::addResourceType("kis_workspaces", "data", "/workspaces/");
    KoResourcePaths::addResourceType("psd_layer_style_collections", "data", "/asl");
    KoResourcePaths::addResourceType("ko_patterns", "data", "/patterns/", true);
    KoResourcePaths::addResourceType("ko_gradients", "data", "/gradients/");
    KoResourcePaths::addResourceType("ko_gradients", "data", "/gradients/", true);
    KoResourcePaths::addResourceType("ko_palettes", "data", "/palettes/", true);
    KoResourcePaths::addResourceType("kis_shortcuts", "data", "/shortcuts/");
    KoResourcePaths::addResourceType("kis_actions", "data", "/actions");
    KoResourcePaths::addResourceType("icc_profiles", "data", "/color/icc");
    KoResourcePaths::addResourceType("ko_effects", "data", "/effects/");
    KoResourcePaths::addResourceType("tags", "data", "/tags/");

}



void KisDerivedResourcesTest::test()
{
    KisDocument* doc = createEmptyDocument();

    addResourceTypes();

    KisMainWindow* mainWindow = KisPart::instance()->createMainWindow();
    QPointer<KisView> view = new KisView(doc, mainWindow->resourceManager(), mainWindow->actionCollection(), mainWindow);
    KisViewManager *viewManager = new KisViewManager(mainWindow, mainWindow->actionCollection());
    KoCanvasResourceManager *manager = viewManager->resourceProvider()->resourceManager();

    QApplication::processEvents();

    QString presetFileName = "autobrush_300px.kpp";

    QVariant i;

    KisPaintOpPresetSP preset;
    if (!presetFileName.isEmpty()) {
        QString fullFileName = TestUtil::fetchDataFileLazy(presetFileName);
        preset = new KisPaintOpPreset(fullFileName);
        bool presetValid = preset->load();
        Q_ASSERT(presetValid); Q_UNUSED(presetValid);

        i.setValue(preset);

    }

    QVERIFY(i.isValid());

    QSignalSpy spy(manager, SIGNAL(canvasResourceChanged(int, const QVariant &)));

    manager->setResource(KisCanvasResourceProvider::CurrentPaintOpPreset, i);

    QCOMPARE(spy[0][0].toInt(), (int)KisCanvasResourceProvider::CurrentPaintOpPreset);
    QCOMPARE(spy[0][1].value<KisPaintOpPresetSP>(), preset);

    QCOMPARE(spy[1][0].toInt(), (int)KisCanvasResourceProvider::EraserMode);
    QCOMPARE(spy[1][1].toBool(), false);

    QCOMPARE(spy[2][0].toInt(), (int)KisCanvasResourceProvider::LodAvailability);
    QCOMPARE(spy[2][1].toBool(), true);

    QCOMPARE(spy[3][0].toInt(), (int)KisCanvasResourceProvider::Size);
    QCOMPARE(spy[3][1].toDouble(), 1.0);

    QCOMPARE(spy[4][0].toInt(), (int)KisCanvasResourceProvider::Flow);
    QCOMPARE(spy[4][1].toDouble(), 1.0);

    QCOMPARE(spy[5][0].toInt(), (int)KisCanvasResourceProvider::Opacity);
    QCOMPARE(spy[5][1].toDouble(), 1.0);

    QCOMPARE(spy[6][0].toInt(), (int)KisCanvasResourceProvider::CurrentEffectiveCompositeOp);
    QCOMPARE(spy[6][1].toString(), COMPOSITE_OVER);

    spy.clear();

    preset->settings()->setPaintOpOpacity(0.8);

    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy[0][0].toInt(), (int)KisCanvasResourceProvider::Opacity);
    QCOMPARE(spy[0][1].toDouble(), 0.8);
    spy.clear();


    mainWindow->hide();
    QApplication::processEvents();

    delete view;
    delete doc;
    delete mainWindow;
}

QTEST_MAIN(KisDerivedResourcesTest)
