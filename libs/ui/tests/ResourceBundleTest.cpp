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

#include <KisResourceServerProvider.h>
#include "kis_workspace_resource.h"
#include <brushengine/kis_paintop_preset.h>
#include "kis_brush_server.h"

#include "ResourceBundleTest.h"
#include "KisResourceBundle.h"

void ResourceBundleTest::testCreateBundle()
{
    QString testbundle = QString(FILES_DATA_DIR) + "/" + "testcreatebundle.bundle";
    KisResourceBundle bundle(testbundle);
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


void ResourceBundleTest::testLoadSave()
{
    KisResourceBundle bundle(QString(FILES_OUTPUT_DIR) + "/" + "testloadsavebundle.bundle");
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
    QVERIFY(gradientServer->resourceCount() > 0);
    Q_FOREACH (KoAbstractGradient* gradient, gradientServer->resources()) {
        if (gradient->name() == "Foreground to Transparent" || gradient->name() == "Foreground to Background") continue;
        gradientServer->addTag(gradient, QString("testtag: %1").arg(tagCount));
        tagCount++;
        bundle.addResource(gradientServer->type(), gradient->filename(), gradientServer->assignedTagsList(gradient), gradient->md5());
    }

    KoResourceServer<KoPattern>* patternServer = KoResourceServerProvider::instance()->patternServer();
    QVERIFY(patternServer->resourceCount() > 0);
    Q_FOREACH (KoPattern* pattern, patternServer->resources()) {
        patternServer->addTag(pattern, QString("testtag: %1").arg(tagCount));
        tagCount++;
        bundle.addResource(patternServer->type(), pattern->filename(), patternServer->assignedTagsList(pattern), pattern->md5());
    }

    KisBrushResourceServer* brushServer = KisBrushServer::instance()->brushServer();

    QVERIFY(brushServer->resourceCount() > 0);

    Q_FOREACH (KisBrushSP brush, brushServer->resources()) {
        brushServer->addTag(brush.data(), QString("testtag: %1").arg(tagCount));
        tagCount++;
        bundle.addResource(brushServer->type(), brush->filename(), brushServer->assignedTagsList(brush.data()), brush->md5());
    }


    KoResourceServer<KoColorSet>* paletteServer = KoResourceServerProvider::instance()->paletteServer();
    QVERIFY(paletteServer->resourceCount() > 0);
    Q_FOREACH (KoColorSet* palette, paletteServer->resources()) {
        paletteServer->addTag(palette, QString("testtag: %1").arg(tagCount));
        tagCount++;
        bundle.addResource(paletteServer->type(), palette->filename(), paletteServer->assignedTagsList(palette), palette->md5());
    }


    KoResourceServer< KisWorkspaceResource >* workspaceServer = KisResourceServerProvider::instance()->workspaceServer();
    QVERIFY(workspaceServer->resourceCount() > 0);
    Q_FOREACH (KisWorkspaceResource* workspace, workspaceServer->resources()) {
        workspaceServer->addTag(workspace, QString("testtag: %1").arg(tagCount));
        tagCount++;
        bundle.addResource(workspaceServer->type(), workspace->filename(), workspaceServer->assignedTagsList(workspace), workspace->md5());
    }

    KisPaintOpPresetResourceServer * paintopServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    QVERIFY(paintopServer->resourceCount() > 0);
    Q_FOREACH (KisPaintOpPresetSP preset, paintopServer->resources()) {
        paintopServer->addTag(preset.data(), QString("testtag: %1").arg(tagCount));
        tagCount++;
        bundle.addResource(paintopServer->type(), preset->filename(), paintopServer->assignedTagsList(preset.data()), preset->md5());
    }

    //QCOMPARE(bundle.getTagsList(), );

    bool res = bundle.save();
    QCOMPARE(bundle.getMeta("updated"), QDate::currentDate().toString("dd/MM/yyyy"));
    QVERIFY(res);

    KisResourceBundle bundle2(QString(FILES_OUTPUT_DIR) + "/" + "testloadsavebundle.bundle");
    res = bundle2.load();

    QVERIFY(res);

    // load sets installed to true
    QVERIFY(bundle2.isInstalled());
    QVERIFY(bundle2.valid());
    //QCOMPARE(bundle2.getTagsList().size(), tagCount);
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
    QVERIFY(!img.isNull());
}

void ResourceBundleTest::testInstallUninstall()
{

}


QTEST_MAIN(ResourceBundleTest)

