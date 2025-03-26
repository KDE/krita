/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_linked_pattern_manager_test.h"

#include <testresources.h>

#include <QPainter>

#include <KoResourceServerProvider.h>
#include <resources/KoPattern.h>

#include "KisEmbeddedTextureData.h"

#include <kis_properties_configuration.h>
#include <KisResourceServerProvider.h>
#include <KisGlobalResourcesInterface.h>
#include <KoResourceServerProvider.h>
#include <KoResourceLoadResult.h>

#include <KoMD5Generator.h>

KoPatternSP createPattern(const QString &name, const QString &fileName)
{
    QImage image(512, 512, QImage::Format_ARGB32);
    image.fill(255);

    QPainter gc(&image);

    /**
     * Make sure that MD5 of every generated resource is different
     */
    const QString hash = KoMD5Generator::generateHash(fileName.toLatin1());
    const QColor fillColor(
            hash.toLatin1()[0],
            hash.toLatin1()[1],
            hash.toLatin1()[2],
            hash.toLatin1()[3]);

    gc.fillRect(100, 100, 312, 312, fillColor);

    KoPatternSP pattern (new KoPattern(image,
                                       name,
                                       fileName));
    return pattern;
}


void KisLinkedPatternManagerTest::testRoundTrip_data()
{
    QTest::addColumn<QString>("loadMode");

    QTest::newRow("old-md5") << "old-md5";
    QTest::newRow("new-md5") << "new-md5";
    QTest::newRow("name") << "name";
    QTest::newRow("filename") << "filename";
    QTest::newRow("filename-with-path") << "filename-with-path";
}

void KisLinkedPatternManagerTest::testRoundTrip()
{
    QFETCH(QString, loadMode);

    QLatin1String tagName(QTest::currentDataTag());
    const QString fileName = QLatin1String(QTest::currentTestFunction()) + "_" + tagName + "_pattern.pat";
    const QString name = QLatin1String(QTest::currentTestFunction()) + "_" + tagName + "_pattern";

    KoPatternSP pattern = createPattern(name, fileName);

    KoResourceServer<KoPattern> *resourceServer = KoResourceServerProvider::instance()->patternServer();
    resourceServer->addResource(pattern);

    KisPropertiesConfigurationSP config(new KisPropertiesConfiguration);

    KisEmbeddedTextureData data1 = KisEmbeddedTextureData::fromPattern(pattern);
    data1.write(config.data());

    /**
     * Test if each of the four tags alone is enough to load the pattern.
     * Basically, we test if each tag is round-tripped correctly
     */

    if (loadMode != "old-md5") {
        config->removeProperty("Texture/Pattern/PatternMD5");
    }

    if (loadMode != "new-md5") {
        config->removeProperty("Texture/Pattern/PatternMD5Sum");
    }

    if (loadMode != "name") {
        config->removeProperty("Texture/Pattern/Name");
    }

    if (loadMode != "filename") {
        config->removeProperty("Texture/Pattern/PatternFileName");
    }

    if (loadMode == "filename-with-path") {
        QString path = KoResourceServerProvider::instance()->patternServer()->saveLocation() + "/" + fileName;
        config->setProperty("Texture/Pattern/PatternFileName", path);
    }

    KisEmbeddedTextureData data2;
    data2.read(config.data());

    KoResourceLoadResult result = data2.loadLinkedPattern(KisGlobalResourcesInterface::instance());

    QCOMPARE(result.type(), KoResourceLoadResult::ExistingResource);
    KoPatternSP newPattern = result.resource<KoPattern>();
    QVERIFY(newPattern);

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

KisPropertiesConfigurationSP KisLinkedPatternManagerTest::createXML(SaveDataFlags flags,
                                                                    KoPatternSP pattern)
{

    KisPropertiesConfigurationSP setting(new KisPropertiesConfiguration);

    if (flags.testFlag(SaveFileName)) {
        setting->setProperty("Texture/Pattern/PatternFileName", pattern->filename());
    }

    if (flags.testFlag(SaveFileNameWithPath)) {
        QString path = KoResourceServerProvider::instance()->patternServer()->saveLocation() + "/" + pattern->filename();
        setting->setProperty("Texture/Pattern/PatternFileName", path);
    }

    if (flags.testFlag(SaveName)) {
        setting->setProperty("Texture/Pattern/Name", pattern->name());
    }

    if (flags.testFlag(SaveOldMd5Base64)) {
        QString patternMD5 = pattern->md5Sum();
        KIS_ASSERT(!patternMD5.isEmpty());

        /// WARNING: KisPropertiesConfiguration saved QByteArray as a base64 string!
        ///          We don't do this conversion manually here!
        setting->setProperty("Texture/Pattern/PatternMD5",
                             QString(QByteArray::fromHex(patternMD5.toLatin1()).toBase64()));
    }

    if (flags.testFlag(SaveEmbeddedData)) {
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        pattern->saveToDevice(&buffer);
        setting->setProperty("Texture/Pattern/Pattern", buffer.data().toBase64());
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

void KisLinkedPatternManagerTest::testLoadingLegacyXML_data()
{
    QTest::addColumn<bool>("isOnServer");
    QTest::addColumn<SaveDataFlags>("saveDataFlags");


    QTest::newRow("lnk-filename") << true << (SaveFileName | SaveEmbeddedData);
    QTest::newRow("lnk-filename-with-path") << true << (SaveFileNameWithPath | SaveEmbeddedData);
    QTest::newRow("lnk-name") << true << (SaveName | SaveEmbeddedData);
    QTest::newRow("lnk-old-md5base64") << true << (SaveOldMd5Base64 | SaveEmbeddedData);

    QTest::newRow("emb-filename") << false << (SaveFileName | SaveEmbeddedData);
    QTest::newRow("emb-filename-with-path") << false << (SaveFileNameWithPath | SaveEmbeddedData);
    QTest::newRow("emb-name") << false << (SaveName | SaveEmbeddedData);
    QTest::newRow("emb-old-md5base64") << false << (SaveOldMd5Base64 | SaveEmbeddedData);

}

void KisLinkedPatternManagerTest::testLoadingLegacyXML()
{
    QFETCH(SaveDataFlags, saveDataFlags);
    QFETCH(bool, isOnServer);

    QLatin1String tagName(QTest::currentDataTag());
    const QString fileName = QLatin1String(QTest::currentTestFunction()) + "_" + tagName + "_pattern.pat";
    const QString name = QLatin1String(QTest::currentTestFunction()) + "_" + tagName + "_pattern";
    QSharedPointer<KoPattern> basePattern(createPattern(name, fileName));

    if (isOnServer) {
        // upload the resource to the server if requested
        KoResourceServerProvider::instance()->patternServer()->addResource(basePattern);
        QVERIFY(findOnServer(basePattern->md5Sum()));
    }

    KisPropertiesConfigurationSP setting = createXML(saveDataFlags, basePattern);

    KisEmbeddedTextureData data2;
    data2.read(setting.data());

    KoResourceLoadResult result = data2.loadLinkedPattern(KisGlobalResourcesInterface::instance());

    if (isOnServer) {
        KoPatternSP pattern = result.resource<KoPattern>();

        QVERIFY(pattern);
        QCOMPARE(pattern->pattern(), basePattern->pattern());
        QCOMPARE(pattern->name(), basePattern->name());
        QCOMPARE(pattern->filename(), basePattern->filename());
    } else {
        QCOMPARE(result.type(), KoResourceLoadResult::EmbeddedResource);
        KoEmbeddedResource embeddedResource = result.embeddedResource();
        QVERIFY(embeddedResource.sanityCheckMd5());

        if (saveDataFlags.testFlag(SaveOldMd5Base64)) {
            QCOMPARE(embeddedResource.signature().md5sum, basePattern->md5Sum());
        }

        /// WARNING: it seems like mimeTypeForFile doesn't handle it gracefully
        /// when the filename is empty. This code does **not** test handling of
        /// this issue in the real code.
        const QString effectiveFileName =
                !embeddedResource.signature().filename.isEmpty() ?
                    embeddedResource.signature().filename :
                    basePattern->filename();

        KisResourceLoaderBase *loader =
            KisResourceLoaderRegistry::instance()->loader(
                embeddedResource.signature().type,
                KisMimeDatabase::mimeTypeForFile(effectiveFileName));

        QByteArray ba = embeddedResource.data();
        QBuffer buf(&ba);
        buf.open(QBuffer::ReadOnly);

        KoResourceSP resource = loader->load(effectiveFileName, buf, KisGlobalResourcesInterface::instance());

        QVERIFY(resource);
        QCOMPARE(resource->name(), basePattern->name());
        QCOMPARE(resource->filename(), basePattern->filename());
        // NOTE: md5 is explicitly set by the loading code, we cannot
        //       verify it, since it can change after loading

        KoPatternSP loadedPattern = resource.dynamicCast<KoPattern>();
        QVERIFY(loadedPattern);
        QCOMPARE(basePattern->pattern(), loadedPattern->pattern());
    }
}

KISTEST_MAIN(KisLinkedPatternManagerTest)
