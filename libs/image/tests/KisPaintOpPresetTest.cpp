/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisPaintOpPresetTest.h"

#include "simpletest.h"

#include "kis_paintop_preset.h"
#include "KisLocalStrokeResources.h"

#include <QFileInfo>

namespace {
template <typename C, typename T = typename C::value_type>
QSet<T> toSet(const C &container) {
    return QSet<T>(container.begin(), container.end());
}
}

void KisPaintOpPresetTest::testLoadingEmbeddedResources_data()
{
    QTest::addColumn<QString>("testFileName");
    QTest::addColumn<QStringList>("expectedSideLoadedResources");
    QTest::addColumn<QStringList>("expectedLinkedResources");
    QTest::addColumn<QStringList>("expectedEmbeddedResources");

    QTest::newRow("ver-2.2")
        << "test-embedded-resources-2.2.kpp"
        << QStringList{}
        << QStringList{"test_brush.png"}
        << QStringList{"6d8b596cfb5220b06174145fb3fbbaed"};

    QTest::newRow("ver-5.0")
        << "test-embedded-resources-5.0.kpp"
        << QStringList{"9a90b42a7bb2e7cef22689bf3abcdba6", "dc4e9099acb7c3cd33293a48f75c6ff7"}
        << QStringList{"9a90b42a7bb2e7cef22689bf3abcdba6", "dc4e9099acb7c3cd33293a48f75c6ff7"}
        << QStringList{};
}

void KisPaintOpPresetTest::testLoadingEmbeddedResources()
{
    QFETCH(QString, testFileName);
    QFETCH(QStringList, expectedSideLoadedResources);
    QFETCH(QStringList, expectedLinkedResources);
    QFETCH(QStringList, expectedEmbeddedResources);

    const QString fileName = QString(FILES_DATA_DIR) + QDir::separator() + testFileName;

    QVERIFY(QFileInfo(fileName).exists());

    QSharedPointer<KisLocalStrokeResources> linkedResources(new KisLocalStrokeResources());

    KisPaintOpPresetSP preset(new KisPaintOpPreset(fileName));
    preset->load(linkedResources);

    QVERIFY(preset->valid());

    QSet<QString> realSideLoadedSignatures;
    QSet<QString> realLinkedSignatures;
    QSet<QString> realEmbeddedSignatures;

    Q_FOREACH (const KoResourceLoadResult &result, preset->sideLoadedResources(linkedResources)) {
        //qDebug() << "side-loaded" << result.type() << result.signature();
        QCOMPARE(result.type(), KoResourceLoadResult::EmbeddedResource);
        QVERIFY(result.embeddedResource().isValid());
        QVERIFY(result.embeddedResource().sanityCheckMd5());

        realSideLoadedSignatures << result.signature().md5sum;
    }

    // check if clearing the side-loaded resources actually clears them
    preset->clearSideLoadedResources();
    QCOMPARE(preset->sideLoadedResources(linkedResources).size(), 0);

    Q_FOREACH (const KoResourceLoadResult &result, preset->linkedResources(linkedResources)) {
        //qDebug() << "linked" << result.type() << result.signature();

        /**
         * The side-loaded resources are not yet loaded into linkedResources,
         * so the call should return fail-link resources.
         */
        QCOMPARE(result.type(), KoResourceLoadResult::FailedLink);

        /**
         * Linked resources in older versions may miss the MD5 sum, in which case
         * just fall back to a filename
         */
        realLinkedSignatures <<
            (!result.signature().md5sum.isEmpty() ?
                result.signature().md5sum :
                result.signature().filename);
    }

    Q_FOREACH (const KoResourceLoadResult &result, preset->embeddedResources(linkedResources)) {
        //qDebug() << "embedded" << result.type() << result.signature();
        QCOMPARE(result.type(), KoResourceLoadResult::EmbeddedResource);
        realEmbeddedSignatures << result.signature().md5sum;
    }

    QCOMPARE(realSideLoadedSignatures, toSet(expectedSideLoadedResources));
    QCOMPARE(realLinkedSignatures, toSet(expectedLinkedResources));
    QCOMPARE(realEmbeddedSignatures, toSet(expectedEmbeddedResources));
}

SIMPLE_TEST_MAIN(KisPaintOpPresetTest)
