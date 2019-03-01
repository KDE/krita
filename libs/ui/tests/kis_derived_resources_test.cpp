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

#include <KoCanvasResourceProvider.h>

#include "kis_canvas_resource_provider.h"
#include <util.h>
#include <KisMainWindow.h>
#include <KisDocument.h>
#include <KisPart.h>
#include <KisView.h>
#include <KisViewManager.h>
#include <kis_paintop_settings.h>
#include <KoResourcePaths.h>
#include <kis_config.h>
#include "testutil.h"
#include "opengl/kis_opengl.h"


void addResourceTypes()
{
    // All Krita's resource types
    KoResourcePaths::addResourceType("gmic_definitions", "data", "/gmic/");
    KoResourcePaths::addResourceType("icc_profiles", "data", "/color/icc");
    KoResourcePaths::addResourceType("icc_profiles", "data", "/profiles/");
    KoResourcePaths::addResourceType("kis_actions", "data", "/actions");
    KoResourcePaths::addResourceType("kis_brushes", "data", "/brushes/");
    KoResourcePaths::addResourceType("kis_defaultpresets", "data", "/defaultpresets/");
    KoResourcePaths::addResourceType("kis_images", "data", "/images/");
    KoResourcePaths::addResourceType("kis_paintoppresets", "data", "/paintoppresets/");
    KoResourcePaths::addResourceType("kis_pics", "data", "/pics/");
    KoResourcePaths::addResourceType("kis_resourcebundles", "data", "/bundles/");
    KoResourcePaths::addResourceType("kis_shortcuts", "data", "/shortcuts/");
    KoResourcePaths::addResourceType("kis_taskset", "data", "/taskset/");
    KoResourcePaths::addResourceType("kis_taskset", "data", "/taskset/");
    KoResourcePaths::addResourceType("kis_windowlayouts", "data", "/windowlayouts/");
    KoResourcePaths::addResourceType("kis_workspaces", "data", "/workspaces/");
    KoResourcePaths::addResourceType("ko_effects", "data", "/effects/");
    KoResourcePaths::addResourceType("ko_gradients", "data", "/gradients/");
    KoResourcePaths::addResourceType("ko_palettes", "data", "/palettes/");
    KoResourcePaths::addResourceType("ko_patterns", "data", "/patterns/");
    KoResourcePaths::addResourceType("metadata_schema", "data", "/metadata/schemas/");
    KoResourcePaths::addResourceType("psd_layer_style_collections", "data", "/asl");
    KoResourcePaths::addResourceType("tags", "data", "/tags/");

    KisOpenGL::testingInitializeDefaultSurfaceFormat();

    KisConfig cfg(false);
    cfg.setUseOpenGL(false);
}

void KisDerivedResourcesTest::test()
{
    addResourceTypes();

    KisDocument* doc = createEmptyDocument();



    KisMainWindow* mainWindow = KisPart::instance()->createMainWindow();
    QPointer<KisView> view = new KisView(doc, mainWindow->resourceManager(), mainWindow->actionCollection(), mainWindow);
    KisViewManager *viewManager = new KisViewManager(mainWindow, mainWindow->actionCollection());
    KoCanvasResourceProvider *manager = viewManager->canvasResourceProvider()->resourceManager();

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

    QSignalSpy spy(manager, SIGNAL(canvasResourceChanged(int,QVariant)));

    manager->setResource(KisCanvasResourceProvider::CurrentPaintOpPreset, i);

    QMap<int, QVariant> expectedSignals;
    expectedSignals[KisCanvasResourceProvider::CurrentPaintOpPreset] = QVariant::fromValue(preset);
    expectedSignals[KisCanvasResourceProvider::EraserMode] = false;
    expectedSignals[KisCanvasResourceProvider::LodSizeThresholdSupported] = true;
    expectedSignals[KisCanvasResourceProvider::EffectiveLodAvailablility] = true;
    expectedSignals[KisCanvasResourceProvider::LodSizeThreshold] = 100;
    expectedSignals[KisCanvasResourceProvider::LodAvailability] = true;
    expectedSignals[KisCanvasResourceProvider::Opacity] = 1.0;
    expectedSignals[KisCanvasResourceProvider::Size] = 300.0;
    expectedSignals[KisCanvasResourceProvider::Flow] = 1.0;
    expectedSignals[KisCanvasResourceProvider::CurrentEffectiveCompositeOp] = COMPOSITE_OVER;
    expectedSignals[KisCanvasResourceProvider::CurrentCompositeOp] = COMPOSITE_OVER;

    auto it = spy.begin();
    for (; it != spy.end(); ++it) {
        const int id = (*it)[0].toInt();
        const QVariant value = (*it)[1];

        if (!expectedSignals.contains(id)) {
            qDebug() << ppVar(id) << ppVar(value);
            QFAIL("Unexpected signal!");
        } else {
            if (expectedSignals[id] != value) {
                qDebug() << ppVar(id) << ppVar(value) << ppVar(expectedSignals[id]);
                QFAIL("Unexpected value!");
            }
        }
    }

    QCOMPARE(spy.size(), expectedSignals.size());

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

KISTEST_MAIN(KisDerivedResourcesTest)
