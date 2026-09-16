/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <simpletest.h>
#include <QCheckBox>
#include <KoID.h>
#include "animation/VideoExportOptionsDialog.h"

class KisVideoExportOptionsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testProRes4444()
    {
        using Dialog = KisVideoExportOptionsDialog;
        QCOMPARE(Dialog::mimeToContainer("video/quicktime"), Dialog::MOV);
        const auto encoders = Dialog::encoderIdentifiers(Dialog::MOV);
        QCOMPARE(encoders.size(), 1);
        QCOMPARE(encoders.first().id(), QString("prores_ks"));

        Dialog dialog(Dialog::MOV, {"prores_ks"}, KisHDRMetadataOptions());
        dialog.setConfiguration(new KisPropertiesConfiguration());
        const QStringList expected {"-c:v", "prores_ks", "-profile:v", "4", "-pix_fmt", "yuva444p10le"};
        QCOMPARE(dialog.customUserOptions(), expected);

        // Switching from an existing VP9 preset must select the MOV encoder.
        KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
        cfg->setProperty("CodecId", "libvpx-vp9");
        dialog.setConfiguration(cfg);
        QCOMPARE(dialog.customUserOptions(), expected);
        Dialog restored(Dialog::MOV, {"prores_ks"}, KisHDRMetadataOptions());
        restored.setConfiguration(dialog.configuration());
        QCOMPARE(restored.customUserOptions(), expected);
    }

    void testTransparencyContainers()
    {
        using Dialog = KisVideoExportOptionsDialog;
        QCOMPARE(Dialog::mimeToContainer("video/x-matroska"), Dialog::MKV);
        const QList<Dialog::ContainerType> containers {Dialog::WEBM, Dialog::MKV, Dialog::MP4};
        for (const auto container : containers) {
            Dialog dialog(container, {"libvpx-vp9"}, KisHDRMetadataOptions());
            KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
            cfg->setProperty("CodecId", "libvpx-vp9");
            cfg->setProperty("vp9PreserveTransparency", true);
            cfg->setProperty("vp9Lossless", true);
            dialog.setConfiguration(cfg);
            auto *checkbox = dialog.findChild<QCheckBox *>("vp9PreserveTransparency");
            QVERIFY(checkbox);
            QCOMPARE(checkbox->isEnabled(), container != Dialog::MP4);
            const QStringList args = dialog.customUserOptions();
            QCOMPARE(args.value(args.indexOf("-pix_fmt") + 1),
                     container == Dialog::MP4 ? QString("yuv420p") : QString("yuva420p"));
            QVERIFY(dialog.configuration()->getBool("vp9PreserveTransparency"));
            QVERIFY(args.contains("-lossless"));

            checkbox->setChecked(false);
            const QStringList opaqueArgs = dialog.customUserOptions();
            QVERIFY(!opaqueArgs.contains("yuva420p"));
        }
    }

    void testSettingsRoundTripAndCustomOptions()
    {
        using Dialog = KisVideoExportOptionsDialog;
        Dialog dialog(Dialog::WEBM, {"libvpx-vp9"}, KisHDRMetadataOptions());
        KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
        cfg->setProperty("CodecId", "libvpx-vp9");
        cfg->setProperty("vp9PreserveTransparency", true);
        cfg->setProperty("vp9Mbits", 7);
        dialog.setConfiguration(cfg);
        Dialog restored(Dialog::WEBM, {"libvpx-vp9"}, KisHDRMetadataOptions());
        restored.setConfiguration(dialog.configuration());
        QCOMPARE(restored.customUserOptions(), dialog.customUserOptions());
        QVERIFY(restored.customUserOptions().contains("7M"));

        cfg->setProperty("CustomLineValue", "-c:v libvpx-vp9 -pix_fmt yuv420p -b:v 3M");
        restored.setConfiguration(cfg);
        QCOMPARE(restored.customUserOptionsString(), cfg->getString("CustomLineValue"));
        QVERIFY(QMetaObject::invokeMethod(&restored, "slotResetCustomLine", Qt::DirectConnection));
        QVERIFY(restored.customUserOptions().contains("yuva420p"));
    }
};

SIMPLE_TEST_MAIN(KisVideoExportOptionsTest)
#include "KisVideoExportOptionsTest.moc"
