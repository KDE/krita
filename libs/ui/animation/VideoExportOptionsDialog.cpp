/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "VideoExportOptionsDialog.h"
#include "ui_video_export_options_dialog.h"

#include <KoID.h>

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include "VideoHDRMetadataOptionsDialog.h"
#include "KisHDRMetadataOptions.h"


struct KisVideoExportOptionsDialog::Private
{
    Private(ContainerType _containerType, const QStringList& validEncoders)
        : containerType(_containerType)
    {
        encoders = KisVideoExportOptionsDialog::encoderIdentifiers(containerType);

        // Cross check w/ arbitrary ffmpeg's list of present encoders if list is populated.
        if (validEncoders.size() > 0) {
            encoders = [validEncoders](const QVector<KoID>& input) -> QVector<KoID> {
                QVector<KoID> filtered;
                Q_FOREACH(const KoID& encoder, input) {
                    if (validEncoders.contains(encoder.id())) { 
                        filtered << encoder;
                    }
                }

                return filtered;
            }(encoders);
        }

        presetsXH264 << KoID("ultrafast", i18nc("h264 preset name, check simplescreenrecorder for standard translations", "ultrafast"));
        presetsXH264 << KoID("superfast", i18nc("h264 preset name, check simplescreenrecorder for standard translations", "superfast"));
        presetsXH264 << KoID("veryfast", i18nc("h264 preset name, check simplescreenrecorder for standard translations", "veryfast"));
        presetsXH264 << KoID("faster", i18nc("h264 preset name, check simplescreenrecorder for standard translations", "faster"));
        presetsXH264 << KoID("fast", i18nc("h264 preset name, check simplescreenrecorder for standard translations", "fast"));
        presetsXH264 << KoID("medium", i18nc("h264 preset name, check simplescreenrecorder for standard translations", "medium"));
        presetsXH264 << KoID("slow", i18nc("h264 preset name, check simplescreenrecorder for standard translations", "slow"));
        presetsXH264 << KoID("slower", i18nc("h264 preset name, check simplescreenrecorder for standard translations", "slower"));
        presetsXH264 << KoID("veryslow", i18nc("h264 preset name, check simplescreenrecorder for standard translations", "veryslow"));
        presetsXH264 << KoID("placebo", i18nc("h264 preset name, check simplescreenrecorder for standard translations", "placebo"));

        profilesXH264 << KoID("baseline", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "baseline"));
        profilesXH264 << KoID("main", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "main"));
        profilesXH264 << KoID("high", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "high"));
        profilesXH264 << KoID("high10", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "high10"));
        profilesXH264 << KoID("high422", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "high422"));
        profilesXH264 << KoID("high444", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "high444"));

        profilesXH265 << KoID("main", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "main"));
        profilesXH265 << KoID("main10", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "main10 (HDR)"));

        // TODO: add "none" tune option
        tuningXH264 << KoID("film", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "film"));
        tuningXH264 << KoID("animation", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "animation"));
        tuningXH264 << KoID("grain", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "grain"));
        tuningXH264 << KoID("stillimage", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "stillimage"));
        tuningXH264 << KoID("psnr", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "psnr"));
        tuningXH264 << KoID("ssim", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "ssim"));
        tuningXH264 << KoID("fastdecode", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "fastdecode"));
        tuningXH264 << KoID("zerolatency", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "zerolatency"));

        tuningXH265 << KoID("none", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "none"));
        tuningXH265 << KoID("animation", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "animation"));
        tuningXH265 << KoID("grain", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "grain"));
        tuningXH265 << KoID("psnr", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "psnr"));
        tuningXH265 << KoID("ssim", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "ssim"));
        tuningXH265 << KoID("fastdecode", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "fastdecode"));
        tuningXH265 << KoID("zero-latency", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "zero-latency"));
        
        predAPNG << KoID("none", i18nc("apng prediction option name", "none"));
        predAPNG << KoID("sub", i18nc("apng prediction option name", "sub"));
        predAPNG << KoID("up", i18nc("apng prediction option name", "up"));
        predAPNG << KoID("avg", i18nc("apng prediction option name", "avg"));
        predAPNG << KoID("paeth", i18nc("apng prediction option name", "paeth"));
        predAPNG << KoID("mixed", i18nc("apng prediction option name", "mixed"));

        presetsWEBP << KoID("default", i18nc("webp preset option name", "default"));        
        presetsWEBP << KoID("none", i18nc("webp preset option name", "none"));
        presetsWEBP << KoID("drawing", i18nc("webp preset option name", "drawing"));        
        presetsWEBP << KoID("icon", i18nc("webp preset option name", "icon"));
        presetsWEBP << KoID("photo", i18nc("webp preset option name", "photo"));
        presetsWEBP << KoID("picture", i18nc("webp preset option name", "picture"));
        presetsWEBP << KoID("text", i18nc("webp preset option name", "text"));
        
        paletteGenModeGIF << KoID("full", i18nc("palettegen status mode option name", "Global/Full"));
        paletteGenModeGIF << KoID("diff", i18nc("palettegen status mode option name", "Difference"));
        paletteGenModeGIF << KoID("single", i18nc("palettegen status mode option name", "Per Single Frame"));
    
        paletteDitherGIF << KoID("none", i18nc("paletteuse dither option name", "none"));
        paletteDitherGIF << KoID("bayer", i18nc("paletteuse dither option name", "bayer"));
        paletteDitherGIF << KoID("floyd_steinberg", i18nc("paletteuse dither option name", "floyd_steinberg"));
        paletteDitherGIF << KoID("heckbert", i18nc("paletteuse dither option name", "heckbert"));
        paletteDitherGIF << KoID("sierra2", i18nc("paletteuse dither option name", "sierra2"));
        paletteDitherGIF << KoID("sierra2_4a", i18nc("paletteuse dither option name", "sierra2_4a"));
        
        paletteDiffModeGIF << KoID("none", i18nc("paletteuse diff mode option name", "none"));
        paletteDiffModeGIF << KoID("rectangle", i18nc("paletteuse diff mode option name", "rectangle"));
    }

    QVector<KoID> encoders;
    QVector<KoID> presetsXH264;
    QVector<KoID> profilesXH264;
    QVector<KoID> profilesXH265;
    
    QVector<KoID> predAPNG;
    
    QVector<KoID> paletteGenModeGIF;
    QVector<KoID> paletteDitherGIF;
    QVector<KoID> paletteDiffModeGIF;

    QVector<KoID> tuningXH264;
    QVector<KoID> tuningXH265;
    QVector<KoID> presetsWEBP;

    QString currentCustomLine;

    KisHDRMetadataOptions hdrMetadataOptions;

    ContainerType containerType;
    bool supportsHDR = false;
};

void populateComboWithKoIds(QComboBox *combo, const QVector<KoID> &ids, int defaultIndex)
{
    Q_FOREACH (const KoID &id, ids) {
        combo->insertItem(combo->count(), id.name());
    }
    combo->setCurrentIndex(defaultIndex);
    combo->setEnabled(combo->count() > 1);
}

KisVideoExportOptionsDialog::KisVideoExportOptionsDialog(ContainerType containerType, const QStringList& validEncoders, QWidget *parent)
    : KisConfigWidget(parent),
      ui(new Ui::VideoExportOptionsDialog),
      m_d(new Private(containerType, validEncoders))
{
    ui->setupUi(this);

    ui->intOpenH264bitrate->setRange(100, 10000);
    ui->intOpenH264bitrate->setValue(5000);
    ui->intOpenH264bitrate->setSuffix(i18nc("kilo-bits-per-second, video bitrate suffix", "kbps"));

    ui->intCRFH264->setRange(0, 51);
    ui->intCRFH264->setValue(28);

    ui->intCRFH265->setRange(0, 51);
    ui->intCRFH265->setValue(28);

    populateComboWithKoIds(ui->cmbPresetH264, m_d->presetsXH264, 5);
    populateComboWithKoIds(ui->cmbPresetH265, m_d->presetsXH264, 5);

    populateComboWithKoIds(ui->cmbProfileH264, m_d->profilesXH264, 0);
    populateComboWithKoIds(ui->cmbProfileH265, m_d->profilesXH265, 0);

    populateComboWithKoIds(ui->cmbTuneH264, m_d->tuningXH264, 0);
    populateComboWithKoIds(ui->cmbTuneH265, m_d->tuningXH265, 0);

    ui->intBitrate->setRange(10, 50000);
    ui->intBitrate->setValue(5000);
    ui->intBitrate->setSuffix(i18nc("kilo-bits-per-second, video bitrate suffix", "kbps"));

    ui->gifReserveTransparent->setChecked(true);
    ui->gifLoop->setChecked(true);
    ui->gifTransDiff->setChecked(true);
    
    populateComboWithKoIds(ui->cmbPalettegenStatsModeGIF, m_d->paletteGenModeGIF, 0);
    populateComboWithKoIds(ui->cmbPaletteuseDitherGIF, m_d->paletteDitherGIF, 5);
    populateComboWithKoIds(ui->cmbPaletteuseDiffModeGIF, m_d->paletteDiffModeGIF, 0);
 
    ui->intPaletteuseBayerScaleGIF->setRange(0, 5);
    ui->intPaletteuseBayerScaleGIF->setValue(2);
    
    ui->apngLoop->setChecked(true);
    
    populateComboWithKoIds(ui->cmbPredAPNG, m_d->predAPNG, 0);
    
    ui->intCompressWEBP->setRange(0, 6);
    ui->intCompressWEBP->setValue(4);

    ui->intQscaleWEBP->setRange(0, 100);
    ui->intQscaleWEBP->setValue(75);
    
    populateComboWithKoIds(ui->cmbPresetWEBP, m_d->presetsWEBP, 0);
    
    ui->webpLoop->setChecked(true);
    
    populateComboWithKoIds(ui->cmbCodec, m_d->encoders, 0);
    connect(ui->cmbCodec, SIGNAL(currentIndexChanged(int)), SLOT(slotCodecSelected(int)));
    slotCodecSelected(0);

    // TODO: temporarily hidden! Some combinations of 'tune' and
    //       'profile' options make ffmpeg generate empty file.
    //       We should not let the user shoot into his own foot!
    ui->cmbTuneH264->setVisible(false);
    ui->lblTuneH264->setVisible(false);

    ui->cmbTuneH265->setVisible(false);
    ui->lblTuneH265->setVisible(false);

    slotCustomLineToggled(false);
    connect(ui->chkCustomLine, SIGNAL(toggled(bool)), SLOT(slotCustomLineToggled(bool)));
    connect(ui->txtCustomLine, SIGNAL(editingFinished()), SLOT(slotSaveCustomLine()));
    connect(ui->btnResetCustomLine, SIGNAL(clicked()), SLOT(slotResetCustomLine()));

    connect(ui->chkUseHDRMetadata, SIGNAL(toggled(bool)),
            ui->btnHdrMetadata, SLOT(setEnabled(bool)));
    connect(ui->cmbProfileH265,
            SIGNAL(currentIndexChanged(int)),
            SLOT(slotH265ProfileChanged(int)));
    slotH265ProfileChanged(ui->cmbProfileH265->currentIndex());

    connect(ui->btnHdrMetadata, SIGNAL(clicked()), SLOT(slotEditHDRMetadata()));

    connect(ui->cmbPaletteuseDitherGIF, 
            SIGNAL(currentIndexChanged(int)),
            SLOT(slotBayerFilterSelected(int)));
    
    slotBayerFilterSelected(ui->cmbPaletteuseDitherGIF->currentIndex());
    
    setSupportsHDR(false);
}

KisVideoExportOptionsDialog::~KisVideoExportOptionsDialog()
{
    delete ui;
}

void KisVideoExportOptionsDialog::setSupportsHDR(bool value)
{
    m_d->supportsHDR = value;
    slotH265ProfileChanged(ui->cmbProfileH265->currentIndex());
}

KisPropertiesConfigurationSP KisVideoExportOptionsDialog::configuration() const
{
    KisPropertiesConfigurationSP cfg(new KisPropertiesConfiguration());

    cfg->setProperty("CodecId", currentCodecId());

    cfg->setProperty("Openh264Bitrate", ui->intOpenH264bitrate->value());

    cfg->setProperty("h264PresetIndex", ui->cmbPresetH264->currentIndex());
    cfg->setProperty("h264ConstantRateFactor", ui->intCRFH264->value());
    cfg->setProperty("h264ProfileIndex", ui->cmbProfileH264->currentIndex());
    cfg->setProperty("h264TuneIndex", ui->cmbTuneH264->currentIndex());

    cfg->setProperty("h265PresetIndex", ui->cmbPresetH265->currentIndex());
    cfg->setProperty("h265ConstantRateFactor", ui->intCRFH265->value());
    cfg->setProperty("h265ProfileIndex", ui->cmbProfileH265->currentIndex());
    cfg->setProperty("h265TuneIndex", ui->cmbTuneH265->currentIndex());
    cfg->setProperty("h265UseHDRMetadata", ui->chkUseHDRMetadata->isChecked());

    cfg->setProperty("TheoraBitrate", ui->intBitrate->value());
    cfg->setProperty("CustomLineValue", ui->txtCustomLine->text());
    cfg->setProperty("customUserOptions", customUserOptions().join(' '));

    cfg->setPrefixedProperties("hdrMetadata/", m_d->hdrMetadataOptions.toProperties());

    return cfg;
}

KisVideoExportOptionsDialog::ContainerType KisVideoExportOptionsDialog::mimeToContainer(const QString &mimeType)
{
    if (mimeType == "video/webm") {
        return ContainerType::WEBM;
    } else if (mimeType == "video/ogg") {
        return ContainerType::OGV;
    } else if (mimeType == "image/gif") {
        return ContainerType::GIF;
    } else if (mimeType == "image/apng") {
        return ContainerType::APNG;
    } else if (mimeType == "image/webp") {
        return ContainerType::WEBP;
    }

    return ContainerType::DEFAULT;
}

QVector<KoID> KisVideoExportOptionsDialog::encoderIdentifiers(ContainerType type)
{
    KIS_ASSERT(type < ContainerType::NUM_CONTAINER_TYPE && type >= ContainerType::DEFAULT);
    QVector<KoID> encoders;
    QVector<KoID> h264Encoders = {
                                KoID("libopenh264", i18nc("openh264 codec name", "OpenH264")),
                                KoID("libx264", i18nc("h264 codec name, check simplescreenrecorder for standard translations", "H.264, MPEG-4 Part 10")),
                                KoID("libx265", i18nc("h265 codec name, check simplescreenrecorder for standard translations", "H.265, MPEG-H Part 2 (HEVC)"))
                                };
    
    KoID vp9Encoder =  KoID("libvpx-vp9", i18nc("VP9 codec name", "VP9"));
    

    switch (type) {
        case ContainerType::OGV:
            encoders << KoID("libtheora", i18nc("theora codec name, check simplescreenrecorder for standard translations", "Theora"));
            break;
        case ContainerType::WEBP:
            encoders << KoID("libwebp", i18nc("WEBP codec name", "WEBP"));
            break;
        case ContainerType::APNG:
            encoders << KoID("apng", i18nc("APNG codec name", "APNG"));
            break;
        case ContainerType::GIF:
            encoders << KoID("gif", i18nc("GIF codec name", "GIF"));
            break;
        case ContainerType::WEBM:
            encoders << vp9Encoder;
            break;
        case ContainerType::MKV:
        case ContainerType::MP4:
        default:
            encoders << h264Encoders;
            encoders << vp9Encoder;
            break;
    }
    
    return encoders;
}

void KisVideoExportOptionsDialog::slotCustomLineToggled(bool value)
{
    QString customLine = m_d->currentCustomLine;

    if (m_d->currentCustomLine.isEmpty() && value) {
        customLine = generateCustomLine().join(" ");
    } else if (!value) {
        customLine = QString();
        m_d->currentCustomLine = QString();
    }

    ui->txtCustomLine->setText(customLine);

    ui->stackedWidget->setEnabled(!value);
    ui->txtCustomLine->setEnabled(value);
    ui->btnResetCustomLine->setEnabled(value);
}

void KisVideoExportOptionsDialog::slotResetCustomLine()
{
    ui->txtCustomLine->setText(generateCustomLine().join(" "));
    slotSaveCustomLine();
}

void KisVideoExportOptionsDialog::slotCodecSelected(int index)
{
    const QString codec = m_d->encoders[index].id();
    if (codec == "libopenh264") {
        ui->stackedWidget->setCurrentIndex(CODEC_OPENH264);
    } else if (codec == "libx264") {
        ui->stackedWidget->setCurrentIndex(CODEC_H264);
    } else if (codec == "libx265") {
        ui->stackedWidget->setCurrentIndex(CODEC_H265);
    } else if (codec == "libtheora") {
        ui->stackedWidget->setCurrentIndex(CODEC_THEORA);
    } else if (codec == "libvpx-vp9") {
        ui->stackedWidget->setCurrentIndex(CODEC_VP9);
    } else if (codec == "gif") {
        ui->stackedWidget->setCurrentIndex(CODEC_GIF);
    } else if (codec == "apng") {
        ui->stackedWidget->setCurrentIndex(CODEC_APNG);
    } else if (codec == "libwebp") {
        ui->stackedWidget->setCurrentIndex(CODEC_WEBP);
    }
}

void KisVideoExportOptionsDialog::slotSaveCustomLine()
{
    m_d->currentCustomLine = ui->txtCustomLine->text();
}

QStringList KisVideoExportOptionsDialog::customUserOptions() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    return ui->chkCustomLine->isChecked() ?
        ui->txtCustomLine->text().split(" ", Qt::SkipEmptyParts) :
                generateCustomLine();
#else
    return ui->chkCustomLine->isChecked() ?
        ui->txtCustomLine->text().split(" ", QString::SkipEmptyParts) :
                generateCustomLine();
#endif
}

QString KisVideoExportOptionsDialog::customUserOptionsString() const
{
    return customUserOptions().join(' ');
}

bool KisVideoExportOptionsDialog::videoConfiguredForHDR() const
{
    return currentCodecId() == "libx265" &&
        ui->chkUseHDRMetadata->isEnabled() &&
            ui->chkUseHDRMetadata->isChecked();
}

void KisVideoExportOptionsDialog::setHDRConfiguration(bool value) {
    if (value && currentCodecId() != "libx265") {
        ui->cmbCodec->setCurrentIndex(m_d->encoders.indexOf(KoID("libx265")));
        ui->chkUseHDRMetadata->setEnabled(true);
    }

    //If HDR is enabled && the codec id is correct, we need to use main10.
    if (value && currentCodecId() == "libx265") {
        ui->cmbProfileH265->setCurrentIndex(m_d->profilesXH265.indexOf(KoID("main10")));
    }

    ui->chkUseHDRMetadata->setChecked(value);
}

int findIndexById(const QString &id, const QVector<KoID> &ids)
{
    int index = -1;
    auto it = std::find_if(ids.begin(), ids.end(), kismpl::mem_equal_to(&KoID::id, id));
    if (it != ids.end()) {
        index = std::distance(ids.begin(), it);
    }

    return index;
}

void KisVideoExportOptionsDialog::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    ui->intOpenH264bitrate->setValue(cfg->getInt("Openh264Bitrate", 5000));

    ui->cmbPresetH264->setCurrentIndex(cfg->getInt("h264PresetIndex", 5));
    ui->intCRFH264->setValue(cfg->getInt("h264ConstantRateFactor", 23));
    ui->cmbProfileH264->setCurrentIndex(cfg->getInt("h264ProfileIndex", 0));
    ui->cmbTuneH264->setCurrentIndex(cfg->getInt("h264TuneIndex", 1));

    ui->cmbPresetH265->setCurrentIndex(cfg->getInt("h265PresetIndex", 5));
    ui->intCRFH265->setValue(cfg->getInt("h265ConstantRateFactor", 23));
    ui->cmbProfileH265->setCurrentIndex(cfg->getInt("h265ProfileIndex", 0));
    ui->cmbTuneH265->setCurrentIndex(cfg->getInt("h265TuneIndex", 1));
    ui->chkUseHDRMetadata->setChecked(cfg->getBool("h265UseHDRMetadata", false));

    ui->intBitrate->setValue(cfg->getInt("TheoraBitrate", 5000));

    m_d->currentCustomLine = cfg->getString("CustomLineValue", QString());
    ui->chkCustomLine->setChecked(!m_d->currentCustomLine.isEmpty());
    slotCustomLineToggled(ui->chkCustomLine->isChecked());

    const QString codecId = cfg->getString("CodecId", "");

    const int index = qMax(0, findIndexById(codecId, m_d->encoders));
    ui->cmbCodec->setCurrentIndex(index);
    slotCodecSelected(index);

    slotH265ProfileChanged(ui->cmbProfileH265->currentIndex());

    KisPropertiesConfigurationSP metadataProperties = new KisPropertiesConfiguration();
    cfg->getPrefixedProperties("hdrMetadata/", metadataProperties);
    m_d->hdrMetadataOptions.fromProperties(metadataProperties);
}

QStringList KisVideoExportOptionsDialog::generateCustomLine() const
{
    QStringList options;

    if (currentCodecId() == "libopenh264") {
        options << "-c:v" << "libopenh264";
        options << "-b:v" << QString::number(ui->intOpenH264bitrate->value()) + "k";
    } else if (currentCodecId() == "libx264") {
        options << "-crf" << QString::number(ui->intCRFH264->value());

        const int presetIndex = ui->cmbPresetH264->currentIndex();
        options << "-preset" << m_d->presetsXH264[presetIndex].id();

        const int profileIndex = ui->cmbProfileH264->currentIndex();
        options << "-profile:v" << m_d->profilesXH264[profileIndex].id();

        if (m_d->profilesXH264[profileIndex].id() == "high422") {
            options << "-pix_fmt" << "yuv422p";
        } else if (m_d->profilesXH264[profileIndex].id() == "high444") {
            options << "-pix_fmt" << "yuv444p";
        } else {
            options << "-pix_fmt" << "yuv420p";
        }

        // Disabled! see the comment in c-tor!
        //const int tuneIndex = ui->cmbTune->currentIndex();
        //options << "-tune" << m_d->tunes[tuneIndex].id();

    } else if (currentCodecId() == "libx265") {
        const bool enableHDR =
            ui->chkUseHDRMetadata->isEnabled() &&
            ui->chkUseHDRMetadata->isChecked();

        if (enableHDR) {
            options << "-colorspace" << "bt2020c"
                    << "-color_trc" << "smpte2084"
                    << "-color_primaries" << "bt2020";
        }

        options << "-c:v" << "libx265";
        options << "-crf" << QString::number(ui->intCRFH265->value());

        const int presetIndex = ui->cmbPresetH265->currentIndex();
        options << "-preset" << m_d->presetsXH264[presetIndex].id();

        const int profileIndex = ui->cmbProfileH265->currentIndex();
        options << "-profile:v" << m_d->profilesXH265[profileIndex].id();

        if (m_d->profilesXH265[profileIndex].id() == "main") {
            options << "-pix_fmt" << "yuv420p";
        } else if (m_d->profilesXH265[profileIndex].id() == "main10") {
            options << "-pix_fmt" << "yuv420p10le";
        } else {
            KIS_SAFE_ASSERT_RECOVER_NOOP(0 && "Unknown profile selected for h265 encoder");
        }

        if (enableHDR) {
            const QString metadataLine = m_d->hdrMetadataOptions.generateFFMpegOptions();
            options << metadataLine.split(" ");
        }

    } else if (currentCodecId() == "libtheora") {
        options << "-b" << QString::number(ui->intBitrate->value()) + "k";
    } else if (currentCodecId() == "libvpx-vp9") {
        options << "-c:v" << currentCodecId();
        if (ui->vp9Lossless->isChecked()) {
            options << "-lossless" <<  "1";
        } else {
            options << "-b:v" << QString::number(ui->vp9Mbits->value()) + "M";
        }
    } else if (currentCodecId() == "gif") {
        const QString ditherFilterString = m_d->paletteDitherGIF[ ui->cmbPaletteuseDitherGIF->currentIndex() ].id();

        options << "-f" << "gif"
                << "-loop" << ( ui->gifLoop->isChecked() ? "0":"-1" )
                << "-gifflags" << ( ui->gifTransDiff->isChecked() ? "+transdiff":"-transdiff" )
                << "-palettegen" << QString("palettegen=stats_mode=%1%2") 
                                            .arg(m_d->paletteGenModeGIF[ ui->cmbPalettegenStatsModeGIF->currentIndex() ].id())
                                            .arg(":reserve_transparent=" + QString(ui->gifReserveTransparent->isChecked() ? "1":"0"))
                << "-lavfi" << QString("[0:v][1:v]paletteuse=dither=%1%2%3")
                                            .arg(ditherFilterString)
                                            .arg(ditherFilterString == "bayer" ? (QString(":bayer_scale=%1").arg(ui->intPaletteuseBayerScaleGIF->value()) ):"" )
                                            .arg(":diff_mode=" + m_d->paletteDiffModeGIF[ ui->cmbPaletteuseDiffModeGIF->currentIndex() ].id() );
                
    } else if (currentCodecId() == "apng") {
        const int predIndex = ui->cmbPredAPNG->currentIndex();
        
        options << "-f" << "apng"
                << "-pred" << m_d->predAPNG[predIndex].id()
                << "-plays" << ( ui->apngLoop->isChecked() ? "0":"1" );
                
    } else if (currentCodecId() == "libwebp") {
        const int presetIndex = ui->cmbPresetWEBP->currentIndex();
        
        options << "-f" << "webp"
                << "-lossless" << ( ui->webpLossless->isChecked() ? "1":"0" )
                << "-compression_level" << QString::number(ui->intCompressWEBP->value())
                << "-q:v" << QString::number(ui->intQscaleWEBP->value())
                << "-preset" << m_d->presetsWEBP[presetIndex].id()
                << "-loop" << ( ui->webpLoop->isChecked() ? "0":"1" );
        
    }

    return options;
}

QString KisVideoExportOptionsDialog::currentCodecId() const
{
    return m_d->encoders[ui->cmbCodec->currentIndex()].id();
}

void KisVideoExportOptionsDialog::slotH265ProfileChanged(int index)
{
    const bool enableHDR =
        m_d->supportsHDR &&
        index >= 0 &&
        m_d->profilesXH265[index].id() == "main10";

    ui->chkUseHDRMetadata->setEnabled(enableHDR);
    ui->btnHdrMetadata->setEnabled(enableHDR && ui->chkUseHDRMetadata->isChecked());

    QString hdrToolTip;

    if (!m_d->supportsHDR) {
        hdrToolTip = i18nc("@info:tooltip", "Exported animation format does not support HDR");
    } else if (!enableHDR) {
        hdrToolTip = i18nc("@info:tooltip", "HDR metadata available only with \"main10\" profile");
    }

    ui->chkUseHDRMetadata->setToolTip(hdrToolTip);
    ui->btnHdrMetadata->setToolTip(hdrToolTip);
}

void KisVideoExportOptionsDialog::slotEditHDRMetadata()
{
    VideoHDRMetadataOptionsDialog dlg(this);
    dlg.setHDRMetadataOptions(m_d->hdrMetadataOptions);

    if (dlg.exec() == QDialog::Accepted) {
        m_d->hdrMetadataOptions = dlg.hdrMetadataOptions();
    }
}

void KisVideoExportOptionsDialog::slotBayerFilterSelected(int index)
{
    const bool enableBayer = m_d->paletteDitherGIF[ index ].id() == "bayer";
    ui->lblPaletteuseBayerScaleGIF->setEnabled( enableBayer );
    ui->intPaletteuseBayerScaleGIF->setEnabled( enableBayer );
}
