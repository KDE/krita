/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_linked_pattern_manager_test.h"

#include <simpletest.h>
#include <QPainter>

#include <KoResourceServerProvider.h>
#include <resources/KoPattern.h>

#include "kis_linked_pattern_manager.h"

#include <kis_properties_configuration.h>
#include <KisResourceServerProvider.h>
#include <KisGlobalResourcesInterface.h>
#include <KoResourceServerProvider.h>

#include "sdk/tests/testresources.h"

KoPatternSP KisLinkedPatternManagerTest::createPattern()
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

void KisLinkedPatternManagerTest::testRoundTrip()
{
    KoPatternSP pattern = createPattern();

    KoResourceServer<KoPattern> *resourceServer = KoResourceServerProvider::instance()->patternServer();
    resourceServer->addResource(pattern, false);

    KisPropertiesConfigurationSP config(new KisPropertiesConfiguration);

    KisLinkedPatternManager::saveLinkedPattern(config, pattern);

    KoPatternSP newPattern = KisLinkedPatternManager::loadLinkedPattern(config, KisGlobalResourcesInterface::instance());

    QCOMPARE(newPattern->pattern(), pattern->pattern());
    QCOMPARE(newPattern->name(), pattern->name());
    QCOMPARE(QFileInfo(newPattern->filename()).fileName(),
             QFileInfo(pattern->filename()).fileName());

}

void KisLinkedPatternManagerTest::init()
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

KisPropertiesConfigurationSP KisLinkedPatternManagerTest::createXML(NameStatus nameStatus, bool hasMd5)
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
            QString patternMD5 = pattern->md5Sum();
            Q_ASSERT(!patternMD5.isEmpty());
            setting->setProperty("Texture/Pattern/PatternMD5", patternMD5);
        }

        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        pattern->pattern().save(&buffer, "PNG");
        setting->setProperty("Texture/Pattern/Pattern", ba.toBase64());
    }


    return setting;
}

KoPatternSP findOnServer(const QString &md5)
{
    KoPatternSP pattern;

    if (!md5.isEmpty()) {
        return KoResourceServerProvider::instance()->patternServer()->resource(md5, "", "");
    }

    return pattern;
}

void KisLinkedPatternManagerTest::checkOneConfig(NameStatus nameStatus, bool hasMd5, QString expectedName, bool isOnServer)
{
    QSharedPointer<KoPattern> basePattern(createPattern());

    KoPatternSP initialPattern = findOnServer(basePattern->md5Sum());
    QCOMPARE((bool)initialPattern, isOnServer);

    KisPropertiesConfigurationSP setting = createXML(nameStatus, hasMd5);


    KoPatternSP pattern = KisLinkedPatternManager::loadLinkedPattern(setting, KisGlobalResourcesInterface::instance());

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

void KisLinkedPatternManagerTest::testLoadingNotOnServerValidName()
{
    checkOneConfig(VALID, false, "__test_pattern", false);
}

void KisLinkedPatternManagerTest::testLoadingNotOnServerEmptyName()
{
    checkOneConfig(EMPTY, false, "__test_pattern_path", false);
}

void KisLinkedPatternManagerTest::testLoadingNotOnServerPathName()
{
    checkOneConfig(PATH, false, "__test_pattern", false);
}

void KisLinkedPatternManagerTest::testLoadingOnServerValidName()
{
    KoResourceServerProvider::instance()->patternServer()->addResource(createPattern(), false);
    checkOneConfig(VALID, false, "__test_pattern", true);
}

void KisLinkedPatternManagerTest::testLoadingOnServerEmptyName()
{
    KoResourceServerProvider::instance()->patternServer()->addResource(createPattern(), false);
    checkOneConfig(EMPTY, false, "__test_pattern_path", true);
}

void KisLinkedPatternManagerTest::testLoadingOnServerPathName()
{
    KoResourceServerProvider::instance()->patternServer()->addResource(createPattern(), false);
    checkOneConfig(PATH, false, "__test_pattern", true);
}

void KisLinkedPatternManagerTest::testLoadingOnServerValidNameMd5()
{
    KoResourceServerProvider::instance()->patternServer()->addResource(createPattern(), true);
    checkOneConfig(VALID, true, "__test_pattern", true);
}

void KisLinkedPatternManagerTest::testLoadingOnServerEmptyNameMd5()
{
    KoResourceServerProvider::instance()->patternServer()->addResource(createPattern(), true);
    checkOneConfig(EMPTY, true, "__test_pattern", true);
}

void KisLinkedPatternManagerTest::testLoadingOnServerPathNameMd5()
{
    KoResourceServerProvider::instance()->patternServer()->addResource(createPattern(), false);
    checkOneConfig(PATH, true, "__test_pattern", true);
}

KISTEST_MAIN(KisLinkedPatternManagerTest)
