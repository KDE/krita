/*
 *  Copyright (c) 2014 Victor Lafon metabolic.ewilan@hotmail.fr
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QTest>
#include <QFile>
#include <QDate>

#include <KoResourceServerProvider.h>
#include <KoResourceTagStore.h>

#include <kis_resource_server_provider.h>
#include "kis_workspace_resource.h"
#include "kis_paintop_preset.h"
#include "kis_brush_server.h"

#include "KoResourceBundleTest.h"
#include "KoResourceBundle.h"

#include <qtest_kde.h>

void KoResourceBundleTest::testCreateBundle()
{
    QString testbundle = QString(FILES_DATA_DIR) + "/" + "testcreatebundle.bundle";
    KoResourceBundle bundle(testbundle);
    QVERIFY(!bundle.isInstalled());
    QVERIFY(!bundle.valid());
    QVERIFY(bundle.getTagsList().isEmpty());
    QVERIFY(bundle.filename() == QString(FILES_DATA_DIR) + "/" + "testcreatebundle.bundle");
    QCOMPARE(bundle.shortFilename(), QString("testcreatebundle.bundle"));
    QCOMPARE(bundle.name(), QString("testcreatebundle"));
    QCOMPARE(bundle.defaultFileExtension(), QString(".bundle"));

    bundle.addMeta("author", "TestAuthor");
    QCOMPARE(bundle.getMeta("author"), QString("TestAuthor"));

    bundle.addMeta("email", "TestEmail");
    QCOMPARE(bundle.getMeta("email"), QString("TestEmail"));

    bundle.addMeta("license", "TestLicense");
    QCOMPARE(bundle.getMeta("license"), QString("TestLicense"));

    bundle.addMeta("website", "TestWebsite");
    QCOMPARE(bundle.getMeta("website"), QString("TestWebsite"));

    bundle.addMeta("created", "TestCreated");
    QCOMPARE(bundle.getMeta("created"), QString("TestCreated"));

    bundle.addMeta("updated", "TestUpdated");
    QCOMPARE(bundle.getMeta("updated"), QString("TestUpdated"));

    bundle.addMeta("description", "Test Description");
    QCOMPARE(bundle.getMeta("description"), QString("Test Description"));

    QVERIFY(bundle.getTagsList().isEmpty());
}


void KoResourceBundleTest::testLoadSave()
{
    KoResourceBundle bundle(QString(FILES_OUTPUT_DIR) + "/" + "testloadsavebundle.bundle");
    bundle.addMeta("author", "TestAuthor");
    bundle.addMeta("email", "TestEmail");
    bundle.addMeta("license", "TestLicense");
    bundle.addMeta("website", "TestWebsite");
    bundle.addMeta("created", "TestCreated");
    bundle.addMeta("updated", "TestUpdated");
    bundle.addMeta("description", "Test Description");
    bundle.setThumbnail(QString(FILES_DATA_DIR) + "/" + "thumb.png");

    int tagCount = 0;

    KoResourceServer<KoAbstractGradient>* gradientServer = KoResourceServerProvider::instance()->gradientServer();
    gradientServer->loadResources(gradientServer->fileNames());
    QVERIFY(gradientServer->resoureCount() > 0);
    foreach(KoAbstractGradient* gradient, gradientServer->resources()) {
        if (gradient->name() == "Foreground to Transparent" || gradient->name() == "Foreground to Background") continue;
        gradientServer->addTag(gradient, QString("testtag: %1").arg(tagCount));
        tagCount++;
        bundle.addFile(gradientServer->type(), gradient->filename(), gradientServer->tagObject()->assignedTagsList(gradient));
    }

    KoResourceServer<KoPattern>* patternServer = KoResourceServerProvider::instance()->patternServer();
    QVERIFY(patternServer->resoureCount() > 0);
    foreach(KoPattern* pattern, patternServer->resources()) {
        patternServer->addTag(pattern, QString("testtag: %1").arg(tagCount));
        tagCount++;
        bundle.addFile(patternServer->type(), pattern->filename(), patternServer->tagObject()->assignedTagsList(pattern));
    }

    KoResourceServer<KisBrush>* brushServer = KisBrushServer::instance()->brushServer();
    QVERIFY(brushServer->resoureCount() > 0);
    foreach(KisBrush* brush, brushServer->resources()) {
        brushServer->addTag(brush, QString("testtag: %1").arg(tagCount));
        tagCount++;
        bundle.addFile(brushServer->type(), brush->filename(), brushServer->tagObject()->assignedTagsList(brush));
    }


    KoResourceServer<KoColorSet>* paletteServer = KoResourceServerProvider::instance()->paletteServer();
    QVERIFY(paletteServer->resoureCount() > 0);
    foreach(KoColorSet* palette, paletteServer->resources()) {
        paletteServer->addTag(palette, QString("testtag: %1").arg(tagCount));
        tagCount++;
        bundle.addFile(paletteServer->type(), palette->filename(), paletteServer->tagObject()->assignedTagsList(palette));
    }


    KoResourceServer< KisWorkspaceResource >* workspaceServer = KisResourceServerProvider::instance()->workspaceServer();
    QVERIFY(workspaceServer->resoureCount() > 0);
    foreach(KisWorkspaceResource* workspace, workspaceServer->resources()) {
        workspaceServer->addTag(workspace, QString("testtag: %1").arg(tagCount));
        tagCount++;
        bundle.addFile(workspaceServer->type(), workspace->filename(), workspaceServer->tagObject()->assignedTagsList(workspace));
    }

    KoResourceServer<KisPaintOpPreset>* paintopServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    QVERIFY(paintopServer->resoureCount() > 0);
    foreach(KisPaintOpPreset* preset, paintopServer->resources()) {
        paintopServer->addTag(preset, QString("testtag: %1").arg(tagCount));
        tagCount++;
        bundle.addFile(paintopServer->type(), preset->filename(), paintopServer->tagObject()->assignedTagsList(preset));
    }

    QCOMPARE(bundle.getTagsList().size(), tagCount);

    bool res = bundle.save();
    QCOMPARE(bundle.getMeta("updated"), QDate::currentDate().toString("dd/MM/yyyy"));
    QVERIFY(res);

    KoResourceBundle bundle2(QString(FILES_OUTPUT_DIR) + "/" + "testloadsavebundle.bundle");
    res = bundle2.load();
    QVERIFY(res);

    QVERIFY(!bundle2.isInstalled());
    QVERIFY(bundle2.valid());
    QCOMPARE(bundle2.getTagsList().size(), tagCount);
    QVERIFY(bundle2.filename() == QString(FILES_OUTPUT_DIR) + "/" + "testloadsavebundle.bundle");
    QCOMPARE(bundle2.shortFilename(), QString("testloadsavebundle.bundle"));
    QCOMPARE(bundle2.name(), QString("testloadsavebundle"));
    QCOMPARE(bundle2.defaultFileExtension(), QString(".bundle"));
    QCOMPARE(bundle2.getMeta("author"), QString("TestAuthor"));
    QCOMPARE(bundle2.getMeta("email"), QString("TestEmail"));
    QCOMPARE(bundle2.getMeta("license"), QString("TestLicense"));
    QCOMPARE(bundle2.getMeta("website"), QString("TestWebsite"));
    QCOMPARE(bundle2.getMeta("created"), QString("TestCreated"));
    QCOMPARE(bundle2.getMeta("updated"), QDate::currentDate().toString("dd/MM/yyyy"));
    QCOMPARE(bundle2.getMeta("description"), QString("Test Description"));

    QImage img = bundle2.image();
    QImage thumb = QImage(QString(FILES_DATA_DIR) + "/" + "thumb.png");

    QCOMPARE(img, thumb);
}

void KoResourceBundleTest::testInstallUninstall()
{

}


QTEST_KDEMAIN(KoResourceBundleTest, GUI)

