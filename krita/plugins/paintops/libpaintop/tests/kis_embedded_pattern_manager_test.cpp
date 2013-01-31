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

#include <qtest_kde.h>

#include "kis_embedded_pattern_manager.h"

#include <QPainter>

#include <kis_pattern.h>
#include <kis_properties_configuration.h>
#include <kis_resource_server_provider.h>


KisPattern *createPattern() {
    QImage image(512, 512, QImage::Format_ARGB32);
    image.fill(255);

    QPainter gc(&image);
    gc.fillRect(100, 100, 312, 312, Qt::red);

    return new KisPattern(image, "__test_pattern", KisResourceServerProvider::instance()->patternServer()->saveLocation());
}

void KisEmbeddedPatternManagerTest::initTestCase()
{
    // touch all the barriers
    KisResourceServerProvider::instance()->paintOpPresetServer();
    KisResourceServerProvider::instance()->patternServer();
    KisResourceServerProvider::instance()->workspaceServer();
}

void KisEmbeddedPatternManagerTest::init()
{
    cleanUp();
}

void KisEmbeddedPatternManagerTest::cleanUp()
{
    foreach(KisPattern *p, KisResourceServerProvider::instance()->patternServer()->resources()) {
        if (p->filename().contains("__test_pattern")) {
            QFile file(p->filename());
            file.remove();
        }
        QVERIFY(KisResourceServerProvider::instance()->patternServer()->removeResourceFromServer(p));
    }
}

void KisEmbeddedPatternManagerTest::testRoundTrip()
{
    KisPattern *pattern = createPattern();

    KisPropertiesConfiguration config;

    KisEmbeddedPatternManager::saveEmbeddedPattern(&config, pattern);

    KisPattern *newPattern = KisEmbeddedPatternManager::loadEmbeddedPattern(&config);

    QCOMPARE(newPattern->image(), pattern->image());
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
    switch(nameStatus) {
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
        KisPattern *pattern = createPattern();

        if (hasMd5) {
            QByteArray patternMD5 = pattern->md5();
            setting.setProperty("Texture/Pattern/PatternMD5", patternMD5.toBase64());
        }

        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        pattern->image().save(&buffer, "PNG");
        setting.setProperty("Texture/Pattern/Pattern", ba.toBase64());
        delete pattern;
    }

    setting.setProperty("Texture/Pattern/PatternFileName", fileName);
    setting.setProperty("Texture/Pattern/Name", name);

    return setting;
}

KisPattern* findOnServer(QByteArray md5)
{
    KisPattern *pattern = 0;

    if (!md5.isEmpty()) {
        foreach(KoResource *res, KisResourceServerProvider::instance()->patternServer()->resources()) {
            KisPattern *pat = static_cast<KisPattern *>(res);
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
    KisPattern *basePattern = createPattern();

    KisPattern *initialPattern = findOnServer(basePattern->md5());
    QCOMPARE((bool)initialPattern, isOnServer);

    KisPropertiesConfiguration setting = createXML(nameStatus, hasMd5);
    KisPattern *pattern = KisEmbeddedPatternManager::loadEmbeddedPattern(&setting);

    QVERIFY(pattern);
    QCOMPARE(pattern->image(), basePattern->image());
    QCOMPARE(pattern->name(), QString(expectedName));

    QFileInfo info(pattern->filename());
    QVERIFY(info.baseName().startsWith(expectedName));
    QCOMPARE(info.dir().path(), QDir(KisResourceServerProvider::instance()->patternServer()->saveLocation()).path());

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
    KisResourceServerProvider::instance()->patternServer()->addResource(createPattern(), false);
    checkOneConfig(VALID, false, "__test_pattern", true);
}

void KisEmbeddedPatternManagerTest::testLoadingOnServerEmptyName()
{
    KisResourceServerProvider::instance()->patternServer()->addResource(createPattern(), false);
    checkOneConfig(EMPTY, false, "__test_pattern", true);
}

void KisEmbeddedPatternManagerTest::testLoadingOnServerPathName()
{
    KisResourceServerProvider::instance()->patternServer()->addResource(createPattern(), false);
    checkOneConfig(PATH, false, "__test_pattern", true);
}

void KisEmbeddedPatternManagerTest::testLoadingOnServerValidNameMd5()
{
    KisResourceServerProvider::instance()->patternServer()->addResource(createPattern(), false);
    checkOneConfig(VALID, true, "__test_pattern", true);
}

void KisEmbeddedPatternManagerTest::testLoadingOnServerEmptyNameMd5()
{
    KisResourceServerProvider::instance()->patternServer()->addResource(createPattern(), false);
    checkOneConfig(EMPTY, true, "__test_pattern", true);
}

void KisEmbeddedPatternManagerTest::testLoadingOnServerPathNameMd5()
{
    KisResourceServerProvider::instance()->patternServer()->addResource(createPattern(), false);
    checkOneConfig(PATH, true, "__test_pattern", true);
}

QTEST_KDEMAIN(KisEmbeddedPatternManagerTest, GUI)
