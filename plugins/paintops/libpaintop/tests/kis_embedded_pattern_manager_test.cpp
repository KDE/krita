/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_embedded_pattern_manager_test.h"

#include <QTest>
#include <QPainter>

#include <KoResourceServerProvider.h>
#include <resources/KoPattern.h>

#include "kis_embedded_pattern_manager.h"

#include <kis_properties_configuration.h>
#include <KisResourceServerProvider.h>
#include <KisGlobalResourcesInterface.h>

#include "sdk/tests/testresources.h"

KoPatternSP KisEmbeddedPatternManagerTest::createPattern()
{
    QImage image(512, 512, QImage::Format_ARGB32);
    image.fill(255);

    QPainter gc(&image);
    gc.fillRect(100, 100, 312, 312, Qt::red);

    KoPatternSP pattern (new KoPattern(image,
                                       "__test_pattern",
                                       KoResourceServerProvider::instance()->patternServer()->saveLocation()));
    return pattern;
}

void KisEmbeddedPatternManagerTest::testRoundTrip()
{
    KoPatternSP pattern = createPattern();

    KisPropertiesConfigurationSP config(new KisPropertiesConfiguration);

    KisEmbeddedPatternManager::saveEmbeddedPattern(config, pattern);

    KoPatternSP newPattern = KisEmbeddedPatternManager::loadEmbeddedPattern(config, KisGlobalResourcesInterface::instance());

    QCOMPARE(newPattern->pattern(), pattern->pattern());
    QCOMPARE(newPattern->name(), pattern->name());
    QCOMPARE(QFileInfo(newPattern->filename()).fileName(),
             QFileInfo(pattern->filename()).fileName());

}

void KisEmbeddedPatternManagerTest::init()
{
    QList<KoResourceSP> resourceList;
    KisResourceModel *resourceModel = KoResourceServerProvider::instance()->patternServer()->resourceModel();
    for (int row = 0; row < resourceModel->rowCount(); ++row) {
        resourceList << resourceModel->resourceForIndex(resourceModel->index(row, 0));
    }

    Q_FOREACH(KoResourceSP pa, resourceList) {
        if (pa) {
            KoResourceServerProvider::instance()->patternServer()->removeResourceFile(pa->filename());
        }
    }
}

KisPropertiesConfigurationSP KisEmbeddedPatternManagerTest::createXML(NameStatus nameStatus, bool hasMd5)
{

    KisPropertiesConfigurationSP setting(new KisPropertiesConfiguration);

    switch (nameStatus) {
    case VALID: {
        setting->setProperty("Texture/Pattern/PatternFileName", "./__test_pattern_path.pat");
        setting->setProperty("Texture/Pattern/Name", "__test_pattern");
        break;
    }
    case PATH: {
        QString path = KoResourceServerProvider::instance()->patternServer()->saveLocation() + "/__test_pattern.pat";
        setting->setProperty("Texture/Pattern/PatternFileName", path);
        setting->setProperty("Texture/Pattern/Name", "__test_pattern");
        break;
    }
    case EMPTY: {
        setting->setProperty("Texture/Pattern/PatternFileName", "./__test_pattern_path.pat");
        setting->setProperty("Texture/Pattern/Name", "");
        break;
    }
    }

    {
        KoPatternSP pattern = createPattern();

        if (hasMd5) {
            QByteArray patternMD5 = pattern->md5();
            Q_ASSERT(!patternMD5.isEmpty());
            setting->setProperty("Texture/Pattern/PatternMD5", patternMD5.toBase64());
        }

        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        pattern->pattern().save(&buffer, "PNG");
        setting->setProperty("Texture/Pattern/Pattern", ba.toBase64());
    }


    return setting;
}

KoPatternSP findOnServer(QByteArray md5)
{
    KoPatternSP pattern;

    if (!md5.isEmpty()) {
        return KoResourceServerProvider::instance()->patternServer()->resourceByMD5(md5);
    }

    return pattern;
}

void KisEmbeddedPatternManagerTest::checkOneConfig(NameStatus nameStatus, bool hasMd5, QString expectedName, bool isOnServer)
{
    QSharedPointer<KoPattern> basePattern(createPattern());

    KoPatternSP initialPattern = findOnServer(basePattern->md5());
    QCOMPARE((bool)initialPattern, isOnServer);

    KisPropertiesConfigurationSP setting = createXML(nameStatus, hasMd5);


    KoPatternSP pattern = KisEmbeddedPatternManager::loadEmbeddedPattern(setting, KisGlobalResourcesInterface::instance());

    QVERIFY(pattern);
    QCOMPARE(pattern->pattern(), basePattern->pattern());
    QVERIFY(pattern->name().startsWith(expectedName));

    QFileInfo info(pattern->filename());
    QVERIFY(info.completeBaseName().startsWith(expectedName));
    QCOMPARE(info.dir().path(), QDir(KoResourceServerProvider::instance()->patternServer()->saveLocation()).path());

    // We can only find things on the server by name or by md5; the file path as an identifier does not work.
    if (isOnServer && nameStatus != EMPTY && !hasMd5) {
        QCOMPARE(initialPattern, pattern);
    }

    // will be deleted by the server
    // delete pattern;
}

void KisEmbeddedPatternManagerTest::testLoadingNotOnServerValidName()
{
    checkOneConfig(VALID, false, "__test_pattern", false);
}

void KisEmbeddedPatternManagerTest::testLoadingNotOnServerEmptyName()
{
    checkOneConfig(EMPTY, false, "__test_pattern_path", false);
}

void KisEmbeddedPatternManagerTest::testLoadingNotOnServerPathName()
{
    checkOneConfig(PATH, false, "__test_pattern", false);
}

void KisEmbeddedPatternManagerTest::testLoadingOnServerValidName()
{
    KoResourceServerProvider::instance()->patternServer()->addResource(createPattern(), false);
    checkOneConfig(VALID, false, "__test_pattern", true);
}

void KisEmbeddedPatternManagerTest::testLoadingOnServerEmptyName()
{
    KoResourceServerProvider::instance()->patternServer()->addResource(createPattern(), false);
    checkOneConfig(EMPTY, false, "__test_pattern_path", true);
}

void KisEmbeddedPatternManagerTest::testLoadingOnServerPathName()
{
    KoResourceServerProvider::instance()->patternServer()->addResource(createPattern(), false);
    checkOneConfig(PATH, false, "__test_pattern", true);
}

void KisEmbeddedPatternManagerTest::testLoadingOnServerValidNameMd5()
{
    KoResourceServerProvider::instance()->patternServer()->addResource(createPattern(), true);
    checkOneConfig(VALID, true, "__test_pattern", true);
}

void KisEmbeddedPatternManagerTest::testLoadingOnServerEmptyNameMd5()
{
    KoResourceServerProvider::instance()->patternServer()->addResource(createPattern(), true);
    checkOneConfig(EMPTY, true, "__test_pattern", true);
}

void KisEmbeddedPatternManagerTest::testLoadingOnServerPathNameMd5()
{
    KoResourceServerProvider::instance()->patternServer()->addResource(createPattern(), false);
    checkOneConfig(PATH, true, "__test_pattern", true);
}

KISTEST_MAIN(KisEmbeddedPatternManagerTest)
