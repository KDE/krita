/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_brushop_test.h"

#include <QTest>
#include <qimage_based_test.h>
#include <stroke_testing_utils.h>
#include <brushengine/kis_paint_information.h>
#include <kis_canvas_resource_provider.h>
#include <brushengine/kis_paintop_preset.h>
#include <brushengine/kis_paintop_settings.h>
#include <kis_pressure_mirror_option.h>
#include <kis_pressure_rotation_option.h>

class TestBrushOp : public TestUtil::QImageBasedTest
{
public:
    TestBrushOp(const QString &presetFileName, const QString &prefix = "simple")
        : QImageBasedTest("brushop") {
        m_presetFileName = presetFileName;
        m_prefix = prefix;
    }

    virtual ~TestBrushOp() {}

    void test() {
        test(false, false,  0.0);
        test(false, false, 10.0);
        test(false, false, 20.0);

        test(true, false,  0.0);
        test(true, false, 10.0);
        test(true, false, 20.0);

        test(false, true,  0.0);
        test(false, true, 10.0);
        test(false, true, 20.0);

        test(true, true,  0.0);
        test(true, true, 10.0);
        test(true, true, 20.0);
    }

    void test(bool mirrorX, bool mirrorY, qreal rotation) {
        test(mirrorX, mirrorY, rotation, false, false);
        test(mirrorX, mirrorY, rotation, true,  false);
        test(mirrorX, mirrorY, rotation, false, true);
        test(mirrorX, mirrorY, rotation, true,  true);
    }

    void test(bool mirrorX, bool mirrorY, qreal rotation, bool mirrorDabX, bool mirrorDabY) {
        test(mirrorX, mirrorY, rotation, mirrorDabX, mirrorDabY, 0.0);
        test(mirrorX, mirrorY, rotation, mirrorDabX, mirrorDabY, 360.0 - 10.0);
        test(mirrorX, mirrorY, rotation, mirrorDabX, mirrorDabY, 360.0 - 20.0);
    }

    void test(bool mirrorX, bool mirrorY, qreal rotation, bool mirrorDabX, bool mirrorDabY, qreal dabRotation) {

        KisSurrogateUndoStore *undoStore = new KisSurrogateUndoStore();
        KisImageSP image = createTrivialImage(undoStore);
        image->initialRefreshGraph();

        KisNodeSP paint1 = findNode(image->root(), "paint1");

        QVERIFY(paint1->extent().isEmpty());

        KisPainter gc(paint1->paintDevice());

        QScopedPointer<KoCanvasResourceProvider> manager(
            utils::createResourceManager(image, 0, m_presetFileName));

        KisPaintOpPresetSP preset =
            manager->resource(KisCanvasResourceProvider::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();

        if (mirrorDabX || mirrorDabY) {
            KisPaintOpSettingsSP settings = preset->settings()->clone();

            KisPressureMirrorOption mirrorOption;
            mirrorOption.readOptionSetting(settings);

            mirrorOption.setChecked(true);
            mirrorOption.setCurveUsed(false);

            mirrorOption.enableHorizontalMirror(mirrorDabX);
            mirrorOption.enableVerticalMirror(mirrorDabY);

            mirrorOption.writeOptionSetting(settings.data());

            preset->setSettings(settings);
        }

        if (dabRotation != 0.0) {
            KisPaintOpSettingsSP settings = preset->settings()->clone();

            KisPressureRotationOption rotationOption;
            rotationOption.readOptionSetting(settings);

            rotationOption.setChecked(true);
            rotationOption.setCurveUsed(false);

            rotationOption.setValue(dabRotation / 360.0);

            rotationOption.writeOptionSetting(settings.data());

            preset->setSettings(settings);
        }


        QString testName =
            QString("%7_cmY_%1_cmX_%2_cR_%3_dmX_%4_dmY_%5_dR_%6")
            .arg(mirrorY)
            .arg(mirrorX)
            .arg(rotation)
            .arg(mirrorDabX)
            .arg(mirrorDabY)
            .arg(std::fmod(360.0 - dabRotation, 360.0))
            .arg(m_prefix);

        KisResourcesSnapshotSP resources =
            new KisResourcesSnapshot(image,
                                     paint1,
                                     manager.data());

        resources->setupPainter(&gc);

        doPaint(gc, rotation, mirrorX, mirrorY);

        checkOneLayer(image, paint1, testName);
    }

    virtual void doPaint(KisPainter &gc, qreal rotation, bool mirrorX, bool mirrorY) {
        KisPaintInformation pi(QPointF(100, 100), 1.0);
        pi.setCanvasRotation(rotation);
        pi.setCanvasMirroredH(mirrorX);
        pi.setCanvasMirroredV(mirrorY);

        KisDistanceInformation dist;
        gc.paintAt(pi, &dist);
    }

    QString m_presetFileName;
    QString m_prefix;
};

class TestBrushOpLines : public TestBrushOp
{
public:
    TestBrushOpLines(const QString &presetFileName)
        : TestBrushOp(presetFileName) {
    }

    void doPaint(KisPainter &gc, qreal rotation, bool mirrorX, bool mirrorY) override {

        QVector<KisPaintInformation> vector;

        vector << KisPaintInformation(QPointF(100, 100));
        vector << KisPaintInformation(QPointF(200, 150));
        vector << KisPaintInformation(QPointF(100, 350));


        for (auto pi : vector) {
            pi.setCanvasRotation(rotation);
            pi.setCanvasMirroredH(mirrorX);
            pi.setCanvasMirroredV(mirrorY);
        }

        KisDistanceInformation dist;

        for (int i = 1; i < vector.size(); i++) {
            gc.paintLine(vector[i - 1], vector[i], &dist);
        }
    }
};

class TestBrushOpPressureLines : public TestBrushOp
{
public:
    TestBrushOpPressureLines(const QString &presetFileName, const QString &prefix)
        : TestBrushOp(presetFileName, prefix) {
    }

    void doPaint(KisPainter &gc, qreal rotation, bool mirrorX, bool mirrorY) override {

        QVector<KisPaintInformation> vector;

        vector << KisPaintInformation(QPointF(0, 0), 0.2);
        vector << KisPaintInformation(QPointF(200, 50), 1.0);
        vector << KisPaintInformation(QPointF(100, 250), 0.0);
        vector << KisPaintInformation(QPointF(200, 150), 1.0);
        vector << KisPaintInformation(QPointF(100, 350), 1.0);

        for (auto pi : vector) {
            pi.setCanvasRotation(rotation);
            pi.setCanvasMirroredH(mirrorX);
            pi.setCanvasMirroredV(mirrorY);
        }

        KisDistanceInformation dist;

        for (int i = 1; i < vector.size(); i++) {
            gc.paintLine(vector[i - 1], vector[i], &dist);
        }
    }
};

#include <KoResourcePaths.h>
void KisBrushOpTest::initTestCase()
{
    KoResourcePaths::addResourceDir(ResourceType::Brushes, QString(SYSTEM_RESOURCES_DATA_DIR) + "/brushes");
}

void KisBrushOpTest::testRotationMirroring()
{
    TestBrushOp t("LR_simple.kpp");
    t.test();
}

void KisBrushOpTest::testRotationMirroringDrawingAngle()
{
    TestBrushOpLines t("LR_drawing_angle.kpp");
    t.test();
}

void KisBrushOpTest::testMagicSeven()
{
    /**
     * A special preset that forces Qt to bug:
     *     mask size: 56
     *     brush size: 7
     *     therefore scale is: 0.125
     *     which causes QTransform work as a pure Translate in the mipmap
     */

    TestBrushOpPressureLines t("magic_seven.kpp", "magicseven");
    t.test();
}

QTEST_MAIN(KisBrushOpTest)
