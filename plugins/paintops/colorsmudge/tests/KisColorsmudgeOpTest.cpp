/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisColorsmudgeOpTest.h"

#include "kistest.h"

#include <qimage_based_test.h>
#include <stroke_testing_utils.h>
#include <brushengine/kis_paint_information.h>
#include <kis_canvas_resource_provider.h>
#include <brushengine/kis_paintop_preset.h>
#include <brushengine/kis_paintop_settings.h>
#include <KoCanvasResourcesIds.h>

class TestColorsmudgeOp : public TestUtil::QImageBasedTest
{
public:
    TestColorsmudgeOp(const QString &prefix = "simple")
        : QImageBasedTest("colorsmudgeop") {
        m_prefix = prefix;
    }

    void test(const QString &testName, const QString &presetFileName) {
        KisSurrogateUndoStore *undoStore = new KisSurrogateUndoStore();
        KisImageSP image = createTrivialImage(undoStore);
        image->initialRefreshGraph();
        image->resizeImage(QRect(0,0,200,200));
        image->waitForDone();

        KisNodeSP paint1 = findNode(image->root(), "paint1");

        QVERIFY(paint1->extent().isEmpty());

        paint1->paintDevice()->fill(QRect(80, 5, 50, 190), KoColor(Qt::red, image->colorSpace()));

        KisPainter gc(paint1->paintDevice());

        QScopedPointer<KoCanvasResourceProvider> manager(
            utils::createResourceManager(image, 0, presetFileName));

        manager->setResource(KoCanvasResource::ForegroundColor, KoColor(Qt::green, image->colorSpace()));

        KisPaintOpPresetSP preset =
            manager->resource(KoCanvasResource::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();

        QString testPrefix =
            QString("%1_%2")
            .arg(m_prefix)
            .arg(testName);

        KisResourcesSnapshotSP resources =
            new KisResourcesSnapshot(image,
                                     paint1,
                                     manager.data());

        resources->setupPainter(&gc);

        doPaint(gc);

        checkOneLayer(image, paint1, testPrefix);
    }

    void doPaint(KisPainter &gc) {

        const QVector<qreal> pressureLevels = {1.0, 0.8, 0.5};

        int yOffset = 20;
        Q_FOREACH (qreal pressure, pressureLevels) {

            {
                KisDistanceInformation dist;
                KisPaintInformation p1(QPointF(20, yOffset), pressure);
                KisPaintInformation p2(QPointF(180, yOffset), pressure);

                gc.paintLine(p1, p2, &dist);
            }

            {
                KisDistanceInformation dist;
                KisPaintInformation p1(QPointF(100, yOffset + 30), pressure);
                KisPaintInformation p2(QPointF(180, yOffset + 30), pressure);

                gc.paintLine(p1, p2, &dist);
            }

            yOffset += 60;
        }
    }

    QString m_presetFileName;
    QString m_prefix;
};

void KisColorsmudgeOpTest::test_data()
{
    QTest::addColumn<QString>("testName");
    QTest::addColumn<QString>("preset");

    QStringList files = {
        "test_smudge_20px_dul_nsa_new.0001.kpp",
        "test_smudge_20px_dul_nsa_old.0001.kpp",
        "test_smudge_20px_dul_sa_new.0001.kpp",
        "test_smudge_20px_dul_sa_old.0001.kpp",
        "test_smudge_20px_sme_nsa_new.0001.kpp",
        "test_smudge_20px_sme_nsa_old.0001.kpp",
        "test_smudge_20px_sme_sa_new.0001.kpp",
        "test_smudge_20px_sme_sa_old.0001.kpp"
    };

    Q_FOREACH (const QString &file, files) {
        QRegularExpression re("test_smudge_(.+).0001.kpp");
        const QString name = re.match(file).captured(1);
        const QByteArray nameLatin = name.toLatin1();
        QTest::addRow("%s", nameLatin.data()) << name << file;
    }
}

void KisColorsmudgeOpTest::test()
{
    QFETCH(QString, testName);
    QFETCH(QString, preset);

    TestColorsmudgeOp t;
    t.test(testName, preset);
}

KISTEST_MAIN(KisColorsmudgeOpTest)
