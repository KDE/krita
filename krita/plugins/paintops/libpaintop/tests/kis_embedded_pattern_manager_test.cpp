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

#include "kis_embedded_pattern_manager_test.h"

#include <QTest>

#include "kis_embedded_pattern_manager.h"

#include <QPainter>

#include <KoResourceServerProvider.h>
#include <KoPattern.h>

#include <kis_properties_configuration.h>
#include <kis_resource_server_provider.h>

KoPattern *createPattern()
{
    QImage image(512, 512, QImage::Format_ARGB32);
    image.fill(255);

    QPainter gc(&image);
    gc.fillRect(100, 100, 312, 312, Qt::red);

    return new KoPattern(image, "__test_pattern", KoResourceServerProvider::instance()->patternServer()->saveLocation());
}

void KisEmbeddedPatternManagerTest::initTestCase()
{
    // touch all the barriers
    KisResourceServerProvider::instance()->paintOpPresetServer();
    KoResourceServerProvider::instance()->patternServer();
    KisResourceServerProvider::instance()->workspaceServer();
}

void KisEmbeddedPatternManagerTest::init()
{
    cleanUp();
}

void KisEmbeddedPatternManagerTest::cleanUp()
{
    Q_FOREACH (KoPattern * p, KoResourceServerProvider::instance()->patternServer()->resources()) {
        if (p->filename().contains("__test_pattern")) {
            QFile file(p->filename());
            file.remove();
        }
        QVERIFY(KoResourceServerProvider::instance()->patternServer()->removeResourceFromServer(p));
    }
}

void KisEmbeddedPatternManagerTest::testRoundTrip()
{
    KoPattern *pattern = createPattern();

    KisPropertiesConfiguration config;

    KisEmbeddedPatternManager::saveEmbeddedPattern(&config, pattern);

    KoPattern *newPattern = KisEmbeddedPatternManager::loadEmbeddedPattern(&config);

    QCOMPARE(newPattern->pattern(), pattern->pattern());
    QCOMPARE(newPattern->name(), pattern->name());
    QCOMPARE(QFileInfo(newPattern->filename()).fileName(),
             QFileInfo(pattern->filename()).fileName());

    delete pattern;
    // will be deleted by the server
    // delete newPattern;
}

enum NameStatus {
    VALID,
    PATH,
    EMPTY
};

KisPropertiesConfiguration createXML(NameStatus nameStatus,
                                     bool hasMd5)
{
    QString fileName("./__test_pattern_path.pat");

    QString name;
    switch (nameStatus) {
    case VALID:
        name = "__test_pattern_name";
        break;
    case PATH:
        name = "./path/some_weird_path.pat";
        break;
    case EMPTY:
        name = "";
        break;
    }

    KisPropertiesConfiguration setting;

    {
        KoPattern *pattern = createPattern();

        if (hasMd5) {
            QByteArray patternMD5 = pattern->md5();
            setting.setProperty("Texture/Pattern/PatternMD5", patternMD5.toBase64());
        }

        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        pattern->pattern().save(&buffer, "PNG");
        setting.setProperty("Texture/Pattern/Pattern", ba.toBase64());
        delete pattern;
    }

    setting.setProperty("Texture/Pattern/PatternFileName", fileName);
    setting.setProperty("Texture/Pattern/Name", name);

    return setting;
}

KoPattern* findOnServer(QByteArray md5)
{
    KoPattern *pattern = 0;

    if (!md5.isEmpty()) {
        Q_FOREACH (KoResource * res, KoResourceServerProvider::instance()->patternServer()->resources()) {
            KoPattern *pat = static_cast<KoPattern *>(res);
            if (pat->md5() == md5) {
                pattern = pat;
                break;
            }
        }
    }

    return pattern;
}

void checkOneConfig(NameStatus nameStatus, bool hasMd5,
                    QString expectedName, bool isOnServer)
{
    KoPattern *basePattern = createPattern();

    KoPattern *initialPattern = findOnServer(basePattern->md5());
    QCOMPARE((bool)initialPattern, isOnServer);

    KisPropertiesConfiguration setting = createXML(nameStatus, hasMd5);
    KoPattern *pattern = KisEmbeddedPatternManager::loadEmbeddedPattern(&setting);

    QVERIFY(pattern);
    QCOMPARE(pattern->pattern(), basePattern->pattern());
    QCOMPARE(pattern->name(), QString(expectedName));

    QFileInfo info(pattern->filename());
    QVERIFY(info.baseName().startsWith(expectedName));
    QCOMPARE(info.dir().path(), QDir(KoResourceServerProvider::instance()->patternServer()->saveLocation()).path());

    if (isOnServer) {
        QCOMPARE(initialPattern, pattern);
    }

    // will be deleted by the server
    // delete pattern;
    delete basePattern;
}


void KisEmbeddedPatternManagerTest::testLoadingNoOnServerValidName()
{
    checkOneConfig(VALID, false, "__test_pattern_name", false);
}

void KisEmbeddedPatternManagerTest::testLoadingNoOnServerEmptyName()
{
    checkOneConfig(EMPTY, false, "__test_pattern_path", false);
}

void KisEmbeddedPatternManagerTest::testLoadingNoOnServerPathName()
{
    checkOneConfig(PATH, false, "__test_pattern_path", false);
}

void KisEmbeddedPatternManagerTest::testLoadingOnServerValidName()
{
    KoResourceServerProvider::instance()->patternServer()->addResource(createPattern(), false);
    checkOneConfig(VALID, false, "__test_pattern", true);
}

void KisEmbeddedPatternManagerTest::testLoadingOnServerEmptyName()
{
    KoResourceServerProvider::instance()->patternServer()->addResource(createPattern(), false);
    checkOneConfig(EMPTY, false, "__test_pattern", true);
}

void KisEmbeddedPatternManagerTest::testLoadingOnServerPathName()
{
    KoResourceServerProvider::instance()->patternServer()->addResource(createPattern(), false);
    checkOneConfig(PATH, false, "__test_pattern", true);
}

void KisEmbeddedPatternManagerTest::testLoadingOnServerValidNameMd5()
{
    KoResourceServerProvider::instance()->patternServer()->addResource(createPattern(), false);
    checkOneConfig(VALID, true, "__test_pattern", true);
}

void KisEmbeddedPatternManagerTest::testLoadingOnServerEmptyNameMd5()
{
    KoResourceServerProvider::instance()->patternServer()->addResource(createPattern(), false);
    checkOneConfig(EMPTY, true, "__test_pattern", true);
}

void KisEmbeddedPatternManagerTest::testLoadingOnServerPathNameMd5()
{
    KoResourceServerProvider::instance()->patternServer()->addResource(createPattern(), false);
    checkOneConfig(PATH, true, "__test_pattern", true);
}

QTEST_MAIN(KisEmbeddedPatternManagerTest)
