/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDlgAnimationRenderer.h"

#include <QStandardPaths>
#include <QPluginLoader>
#include <QJsonObject>
#include <QMessageBox>
#include <QStringList>
#include <QProcess>

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <KoResourcePaths.h>
#include <kis_properties_configuration.h>
#include <kis_debug.h>
#include <KisMimeDatabase.h>
#include <KoJsonTrader.h>
#include <KisImportExportFilter.h>
#include <krita_container_utils.h>
#include <kis_image.h>
#include <kis_image_animation_interface.h>
#include <kis_time_span.h>
#include <KisImportExportManager.h>
#include <kis_config_widget.h>
#include <KisDocument.h>
#include <QHBoxLayout>
#include <kis_config.h>
#include <kis_file_name_requester.h>
#include <KoDialog.h>
#include "kis_slider_spin_box.h"
#include "kis_acyclic_signal_connector.h"
#include "KisVideoSaver.h"
#include "KisAnimationRenderingOptions.h"
#include "animation/KisFFMpegWrapper.h"
#include "VideoExportOptionsDialog.h"
#include "kis_image_config.h"


KisDlgAnimationRenderer::KisDlgAnimationRenderer(KisDocument *doc, QWidget *parent)
    : KoDialog(parent)
    , m_image(doc->image())
    , m_doc(doc)
{
    KisConfig cfg(true);

    setCaption(i18n("Render Animation"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    m_page = new WdgAnimationRenderer(this);
    m_page->layout()->setMargin(0);

    m_page->dirRequester->setMode(KoFileDialog::OpenDirectory);

    m_page->intStart->setMinimum(0);
    m_page->intStart->setMaximum(doc->image()->animationInterface()->fullClipRange().end());
    m_page->intEnd->setMinimum(doc->image()->animationInterface()->fullClipRange().start());

    m_page->intHeight->setMinimum(1);
    m_page->intHeight->setMaximum(100000);

    m_page->intWidth->setMinimum(1);
    m_page->intWidth->setMaximum(100000);

    QFileInfo audioFileInfo(doc->image()->animationInterface()->audioChannelFileName());
    const bool hasAudio = audioFileInfo.exists();
    m_page->chkIncludeAudio->setEnabled(hasAudio);

    QStringList mimes = KisImportExportManager::supportedMimeTypes(KisImportExportManager::Export);
    mimes.sort();
    filterSequenceMimeTypes(mimes);
    Q_FOREACH(const QString &mime, mimes) {
        QString description = KisMimeDatabase::descriptionForMimeType(mime);
        if (description.isEmpty()) {
            description = mime;
        }

        m_page->cmbMimetype->addItem(description, mime);

        if (mime == "image/png") {
            m_page->cmbMimetype->setCurrentIndex(m_page->cmbMimetype->count() - 1);
        }
    }

    QStringList supportedMimeType = makeVideoMimeTypesList();
    Q_FOREACH (const QString &mime, supportedMimeType) {
        QString description = KisMimeDatabase::descriptionForMimeType(mime);
        if (description.isEmpty()) {
            description = mime;
        }
        m_page->cmbRenderType->addItem(description, mime);
    }
    
    
    m_page->cmbScaleFilter->addItem(i18nc("bicubic filtering", "bicubic"), "bicubic");
    m_page->cmbScaleFilter->addItem(i18nc("bilinear filtering", "bilinear"), "bilinear");
    m_page->cmbScaleFilter->addItem(i18nc("lanczos3 filtering", "lanczos3"), "lanczos");
    m_page->cmbScaleFilter->addItem(i18nc("nearest neighbor filtering", "neighbor"), "neighbor");
    m_page->cmbScaleFilter->addItem(i18nc("spline filtering", "spline"), "spline");
    
    m_page->videoFilename->setMode(KoFileDialog::SaveFile);

    m_page->ffmpegLocation->setMode(KoFileDialog::OpenFile);

    m_page->cmbRenderType->setCurrentIndex(cfg.readEntry<int>("AnimationRenderer/render_type", 0));
    m_page->cmbRenderType->setCurrentIndex(cfg.readEntry<int>("AnimationRenderer/scale_mode", 0));

    connect(m_page->bnExportOptions, SIGNAL(clicked()), this, SLOT(sequenceMimeTypeOptionsClicked()));
    connect(m_page->bnRenderOptions, SIGNAL(clicked()), this, SLOT(selectRenderOptions()));

    connect(m_page->shouldExportOnlyImageSequence, SIGNAL(toggled(bool)), this, SLOT(slotExportTypeChanged()));
    connect(m_page->shouldExportOnlyVideo, SIGNAL(toggled(bool)), this, SLOT(slotExportTypeChanged()));
    connect(m_page->shouldExportAll, SIGNAL(toggled(bool)), this, SLOT(slotExportTypeChanged()));

    connect(m_page->intFramesPerSecond, SIGNAL(valueChanged(int)), SLOT(frameRateChanged(int)));

    // connect and cold init
    connect(m_page->cmbRenderType, SIGNAL(currentIndexChanged(int)), this, SLOT(selectRenderType(int)));
    selectRenderType(m_page->cmbRenderType->currentIndex());

    // try to lock the width and height being updated
    KisAcyclicSignalConnector *constrainsConnector = new KisAcyclicSignalConnector(this);
    constrainsConnector->createCoordinatedConnector()->connectBackwardInt(m_page->intWidth, SIGNAL(valueChanged(int)), this, SLOT(slotLockAspectRatioDimensionsWidth(int)));
    constrainsConnector->createCoordinatedConnector()->connectForwardInt(m_page->intHeight, SIGNAL(valueChanged(int)), this, SLOT(slotLockAspectRatioDimensionsHeight(int)));

    {
        KisPropertiesConfigurationSP settings = loadLastConfiguration("ANIMATION_EXPORT");
        KisAnimationRenderingOptions options;
        options.fromProperties(settings);
        initializeRenderSettings(*doc, options);
    }

    resize(m_page->sizeHint());
    setMainWidget(m_page);
    connect(this, SIGNAL(accepted()), SLOT(slotDialogAccepted()));
}

KisDlgAnimationRenderer::~KisDlgAnimationRenderer()
{
    delete m_page;
}

void KisDlgAnimationRenderer::getDefaultVideoEncoderOptions(const QString &mimeType,
                                                         KisPropertiesConfigurationSP cfg,
                                                         QString *customFFMpegOptionsString,
                                                         bool *renderHDR)
{
    const KisVideoExportOptionsDialog::ContainerType containerType =
            KisVideoExportOptionsDialog::mimeToContainer(mimeType);

    QScopedPointer<KisVideoExportOptionsDialog> encoderConfigWidget(
        new KisVideoExportOptionsDialog(containerType, 0));

    // we always enable HDR, letting the user to force it
    encoderConfigWidget->setSupportsHDR(true);
    encoderConfigWidget->setConfiguration(cfg);
    *customFFMpegOptionsString = encoderConfigWidget->customUserOptionsString();
    *renderHDR = encoderConfigWidget->videoConfiguredForHDR();
}

void KisDlgAnimationRenderer::filterSequenceMimeTypes(QStringList &mimeTypes)
{
    KritaUtils::filterContainer(mimeTypes, [](QString type) {
        return (type.startsWith("image/")
                || (type.startsWith("application/") &&
                    !type.startsWith("application/x-spriter")));
    });
}

QStringList KisDlgAnimationRenderer::makeVideoMimeTypesList()
{
    QStringList supportedMimeTypes = QStringList();
    supportedMimeTypes << "video/x-matroska";
    supportedMimeTypes << "image/gif";
    supportedMimeTypes << "image/apng";    
    supportedMimeTypes << "image/webp";       
    supportedMimeTypes << "video/ogg";
    supportedMimeTypes << "video/mp4";
    supportedMimeTypes << "video/webm";

    return supportedMimeTypes;
}

bool KisDlgAnimationRenderer::imageMimeSupportsHDR(QString &mime)
{
    return (mime == "image/png");
}

KisPropertiesConfigurationSP KisDlgAnimationRenderer::loadLastConfiguration(QString configurationID) {
    KisConfig globalConfig(true);
    return globalConfig.exportConfiguration(configurationID);
}

void KisDlgAnimationRenderer::saveLastUsedConfiguration(QString configurationID, KisPropertiesConfigurationSP config)
{
    KisConfig globalConfig(false);
    globalConfig.setExportConfiguration(configurationID, config);
}

void KisDlgAnimationRenderer::initializeRenderSettings(const KisDocument &doc, const KisAnimationRenderingOptions &lastUsedOptions)
{
    const QString documentPath = m_doc->localFilePath();

    // Initialize these settings based on last used configuration when possible..
    m_page->txtBasename->setText(lastUsedOptions.basename);

    if (!lastUsedOptions.lastDocuemntPath.isEmpty() &&
            lastUsedOptions.lastDocuemntPath == documentPath) {

        m_page->sequenceStart->setValue(lastUsedOptions.sequenceStart);
        m_page->intWidth->setValue(lastUsedOptions.width);
        m_page->intHeight->setValue(lastUsedOptions.height);

        m_page->videoFilename->setStartDir(lastUsedOptions.resolveAbsoluteDocumentFilePath(documentPath));
        m_page->videoFilename->setFileName(lastUsedOptions.videoFileName);

        m_page->dirRequester->setStartDir(lastUsedOptions.resolveAbsoluteDocumentFilePath(documentPath));
        m_page->dirRequester->setFileName(lastUsedOptions.directory);
    } else {
        m_page->sequenceStart->setValue(m_image->animationInterface()->playbackRange().start());
        m_page->intWidth->setValue(m_image->width());
        m_page->intHeight->setValue(m_image->height());

        m_page->videoFilename->setStartDir(lastUsedOptions.resolveAbsoluteDocumentFilePath(documentPath));
        m_page->videoFilename->setFileName(defaultVideoFileName(m_doc, lastUsedOptions.videoMimeType));

        m_page->dirRequester->setStartDir(lastUsedOptions.resolveAbsoluteDocumentFilePath(documentPath));
        m_page->dirRequester->setFileName(lastUsedOptions.directory);
    }

    for (int i = 0; i < m_page->cmbMimetype->count(); ++i) {
        if (m_page->cmbMimetype->itemData(i).toString() == lastUsedOptions.frameMimeType) {
            m_page->cmbMimetype->setCurrentIndex(i);
            break;
        }
    }

    for (int i = 0; i < m_page->cmbRenderType->count(); ++i) {
        if (m_page->cmbRenderType->itemData(i).toString() == lastUsedOptions.videoMimeType) {
            m_page->cmbRenderType->setCurrentIndex(i);
            break;
        }
    }

    for (int i = 0; i < m_page->cmbScaleFilter->count(); ++i) {
        if (m_page->cmbScaleFilter->itemData(i).toString() == lastUsedOptions.scaleFilter) {
            m_page->cmbScaleFilter->setCurrentIndex(i);
            break;
        }
    }    
    
    m_page->chkOnlyUniqueFrames->setChecked(lastUsedOptions.wantsOnlyUniqueFrameSequence);

    if (lastUsedOptions.shouldDeleteSequence) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(lastUsedOptions.shouldEncodeVideo);
        m_page->shouldExportOnlyVideo->setChecked(true);
    } else if (!lastUsedOptions.shouldEncodeVideo) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(!lastUsedOptions.shouldDeleteSequence);
        m_page->shouldExportOnlyImageSequence->setChecked(true);
    } else {
        m_page->shouldExportAll->setChecked(true); // export to both
    }


    {
        KisPropertiesConfigurationSP settings = loadLastConfiguration("VIDEO_ENCODER");

        getDefaultVideoEncoderOptions(lastUsedOptions.videoMimeType, settings,
                                      &m_customFFMpegOptionsString,
                                      &m_wantsRenderWithHDR);
    }

    {
        KisPropertiesConfigurationSP settings = loadLastConfiguration("img_sequence/" + lastUsedOptions.frameMimeType);
        m_wantsRenderWithHDR = settings->getPropertyLazy("saveAsHDR", m_wantsRenderWithHDR);
    }

    m_page->ffmpegLocation->setStartDir(QFileInfo(m_doc->localFilePath()).path());
    KisConfig cfg(false);
    QString ffmpegPath = cfg.ffmpegLocation();
    QJsonObject ffmpegJsonObj = KisFFMpegWrapper::findFFMpeg(ffmpegPath.isEmpty() ? lastUsedOptions.ffmpegPath:ffmpegPath);

    ffmpegPath = ffmpegJsonObj["path"].toString();
    ffmpegVersion = ffmpegJsonObj["enabled"].toBool() ? ffmpegJsonObj["version"].toString():"None";

    m_page->ffmpegLocation->setFileName(ffmpegPath);
    cfg.setFFMpegLocation(ffmpegPath);
    m_page->lblFFMpegVersion->setText("FFMpeg Version: " + ffmpegVersion + (ffmpegJsonObj["encoder"].toObject()["h264"].toBool() ? "":" (MP4/MKV UNSUPPORTED)"));
    
    ffmpegWarningCheck();

    connect(m_page->ffmpegLocation, SIGNAL(textChanged(QString)), SLOT(slotFFMpegChanged(QString)));

    // Initialize these settings based on the current document context..
    m_page->intStart->setValue(doc.image()->animationInterface()->playbackRange().start());
    m_page->intEnd->setValue(doc.image()->animationInterface()->playbackRange().end());
    m_page->intFramesPerSecond->setValue(doc.image()->animationInterface()->framerate());

    if (!doc.image()->animationInterface()->exportSequenceFilePath().isEmpty()
        && QDir(doc.image()->animationInterface()->exportSequenceFilePath()).exists() ) {
        m_page->dirRequester->setStartDir(doc.image()->animationInterface()->exportSequenceFilePath());
        m_page->dirRequester->setFileName(doc.image()->animationInterface()->exportSequenceFilePath());
    }

    if (!doc.image()->animationInterface()->exportSequenceBaseName().isEmpty()) {
        m_page->txtBasename->setText(doc.image()->animationInterface()->exportSequenceBaseName());
    }

    if (doc.image()->animationInterface()->exportInitialFrameNumber() != -1) {
        m_page->sequenceStart->setValue(doc.image()->animationInterface()->exportInitialFrameNumber());
    }

    m_page->chkIncludeAudio->setChecked(!doc.image()->animationInterface()->isAudioMuted());
}

void KisDlgAnimationRenderer::slotFFMpegChanged(const QString& path) {
    KisConfig cfg(false);
    QJsonObject ffmpegChangeJsonObj = KisFFMpegWrapper::findFFMpeg(path);
    ffmpegVersion = ffmpegChangeJsonObj["enabled"].toBool() ? ffmpegChangeJsonObj["version"].toString():"None";

    cfg.setFFMpegLocation(ffmpegChangeJsonObj["path"].toString());
    m_page->lblFFMpegVersion->setText("FFMpeg Version: " + ffmpegVersion + (ffmpegChangeJsonObj["encoder"].toObject()["h264"].toBool() ? "":" (MP4/MKV UNSUPPORTED)"));

    ffmpegWarningCheck();
}

void KisDlgAnimationRenderer::ffmpegWarningCheck() {
    const QString mimeType = m_page->cmbRenderType->itemData(m_page->cmbRenderType->currentIndex()).toString();

    QRegularExpression minVerFFMpegRX("^n{0,1}(?:[0-3]|4\\.[01])[\\.\\-]");
    QRegularExpressionMatch minVerFFMpegMatch = minVerFFMpegRX.match(ffmpegVersion);

    m_page->lblGifWarningFFMpeg->setVisible((mimeType == "image/gif" && minVerFFMpegMatch.hasMatch() ));
}

QString KisDlgAnimationRenderer::defaultVideoFileName(KisDocument *doc, const QString &mimeType)
{
    const QString docFileName = !doc->localFilePath().isEmpty() ?
        doc->localFilePath() : i18n("Untitled");

    return
        QString("%1.%2")
            .arg(QFileInfo(docFileName).completeBaseName())
            .arg(KisMimeDatabase::suffixesForMimeType( mimeType == "image/apng" ? "image/png":mimeType ).first());
}

void KisDlgAnimationRenderer::selectRenderType(int index)
{
    const QString mimeType = m_page->cmbRenderType->itemData(index).toString();

    // m_page->bnRenderOptions->setEnabled(mimeType != "image/gif" && mimeType != "image/webp" && mimeType != "image/png" );
    m_page->lblGifWarningFPS->setVisible((mimeType == "image/gif" && m_page->intFramesPerSecond->value() > 50));

    ffmpegWarningCheck();

    QString videoFileName = defaultVideoFileName(m_doc, mimeType);

    if (!m_page->videoFilename->fileName().isEmpty()) {
        const QFileInfo info = QFileInfo(m_page->videoFilename->fileName());
        const QString baseName = info.completeBaseName();
        const QString path = info.path();

        videoFileName =
            QString("%1%2%3.%4").arg(path).arg('/').arg(baseName).arg(KisMimeDatabase::suffixesForMimeType( mimeType == "image/apng" ? "image/png":mimeType ).first());

    }
    m_page->videoFilename->setMimeTypeFilters(QStringList() << mimeType, mimeType);
    m_page->videoFilename->setFileName(videoFileName);

    m_wantsRenderWithHDR = (mimeType == "video/mp4") ? m_wantsRenderWithHDR : false;

    {   // We've got to reload the render settings to account for the user changing render type without configuration.
        // If this is removed from the configuration, ogg vorbis can fail to render on first attempt. BUG:421658
        // This should be revisited at some point, too much configuration juggling in this class makes it error-prone...

        KisPropertiesConfigurationSP settings = loadLastConfiguration("VIDEO_ENCODER");
        getDefaultVideoEncoderOptions(mimeType, settings,
                                      &m_customFFMpegOptionsString,
                                      &m_wantsRenderWithHDR);
    }
}

void KisDlgAnimationRenderer::selectRenderOptions()
{
    const int index = m_page->cmbRenderType->currentIndex();
    const QString mimetype = m_page->cmbRenderType->itemData(index).toString();

    const KisVideoExportOptionsDialog::ContainerType containerType =
        KisVideoExportOptionsDialog::mimeToContainer(mimetype);

    KisVideoExportOptionsDialog *encoderConfigWidget =
        new KisVideoExportOptionsDialog(containerType, this);

    // we always enable HDR, letting the user to force it
    encoderConfigWidget->setSupportsHDR(true);

    {
        KisPropertiesConfigurationSP settings = loadLastConfiguration("VIDEO_ENCODER");
        encoderConfigWidget->setConfiguration(settings);
        encoderConfigWidget->setHDRConfiguration(m_wantsRenderWithHDR);
    }

    KoDialog dlg(this);
    dlg.setMainWidget(encoderConfigWidget);
    dlg.setButtons(KoDialog::Ok | KoDialog::Cancel);
    if (dlg.exec() == QDialog::Accepted) {
        saveLastUsedConfiguration("VIDEO_ENCODER", encoderConfigWidget->configuration());
        m_customFFMpegOptionsString = encoderConfigWidget->customUserOptionsString();
        m_wantsRenderWithHDR = encoderConfigWidget->videoConfiguredForHDR();
    }

    dlg.setMainWidget(0);
    encoderConfigWidget->deleteLater();
}

void KisDlgAnimationRenderer::sequenceMimeTypeOptionsClicked()
{
    int index = m_page->cmbMimetype->currentIndex();

    KisConfigWidget *frameExportConfigWidget = 0;

    QString mimetype = m_page->cmbMimetype->itemData(index).toString();
    QSharedPointer<KisImportExportFilter> filter(KisImportExportManager::filterForMimeType(mimetype, KisImportExportManager::Export));
    if (filter) {
        frameExportConfigWidget = filter->createConfigurationWidget(0, KisDocument::nativeFormatMimeType(), mimetype.toLatin1());

        if (frameExportConfigWidget) {

            KisPropertiesConfigurationSP exportConfig = loadLastConfiguration("img_sequence/" + mimetype);
            if (exportConfig) {
                KisImportExportManager::fillStaticExportConfigurationProperties(exportConfig, m_image);
            }

            //Important -- m_useHDR allows the synchronization of both the video and image render settings.
            if(imageMimeSupportsHDR(mimetype)) {
                exportConfig->setProperty("saveAsHDR", m_wantsRenderWithHDR);
                if (m_wantsRenderWithHDR) {
                    exportConfig->setProperty("forceSRGB", false);
                }
            }

            frameExportConfigWidget->setConfiguration(exportConfig);
            KoDialog dlg(this);
            dlg.setMainWidget(frameExportConfigWidget);
            dlg.setButtons(KoDialog::Ok | KoDialog::Cancel);
            if (dlg.exec() == QDialog::Accepted) {
                m_wantsRenderWithHDR = frameExportConfigWidget->configuration()->getPropertyLazy("saveAsHDR", false);
                saveLastUsedConfiguration("img_sequence/" + mimetype, frameExportConfigWidget->configuration());
            }

            frameExportConfigWidget->hide();
            dlg.setMainWidget(0);
            frameExportConfigWidget->setParent(0);
            frameExportConfigWidget->deleteLater();

        }
    }
}


KisAnimationRenderingOptions KisDlgAnimationRenderer::getEncoderOptions() const
{
    KisAnimationRenderingOptions options;

    options.lastDocuemntPath = m_doc->localFilePath();
    options.videoMimeType = m_page->cmbRenderType->currentData().toString();
    options.frameMimeType = m_page->cmbMimetype->currentData().toString();
    options.scaleFilter = m_page->cmbScaleFilter->currentData().toString();

    options.basename = m_page->txtBasename->text();
    options.directory = m_page->dirRequester->fileName();
    options.firstFrame = m_page->intStart->value();
    options.lastFrame = m_page->intEnd->value();
    options.sequenceStart = m_page->sequenceStart->value();

    options.shouldEncodeVideo = !m_page->shouldExportOnlyImageSequence->isChecked();
    options.shouldDeleteSequence = m_page->shouldExportOnlyVideo->isChecked();
    options.includeAudio = m_page->chkIncludeAudio->isChecked();
    options.wantsOnlyUniqueFrameSequence = m_page->chkOnlyUniqueFrames->isChecked();

    options.ffmpegPath = m_page->ffmpegLocation->fileName();
    options.frameRate = m_page->intFramesPerSecond->value();

    if (options.frameRate > 50 && options.videoMimeType == "image/gif") {
        options.frameRate = 50;
    }

    options.width = m_page->intWidth->value();
    options.height = m_page->intHeight->value();
    options.videoFileName = m_page->videoFilename->fileName();

    options.customFFMpegOptions = m_customFFMpegOptionsString;

    {
        KisPropertiesConfigurationSP cfg = loadLastConfiguration("img_sequence/" + options.frameMimeType);
        if (cfg) {
            KisImportExportManager::fillStaticExportConfigurationProperties(cfg, m_image);
        }

        const bool forceNecessaryHDRSettings = m_wantsRenderWithHDR && imageMimeSupportsHDR(options.frameMimeType);
        if (forceNecessaryHDRSettings) {
            KIS_SAFE_ASSERT_RECOVER_NOOP(options.frameMimeType == "image/png");
            cfg->setProperty("forceSRGB", false);
            cfg->setProperty("saveAsHDR", true);
        }

        options.frameExportConfig = cfg;
    }

    return options;
}

void KisDlgAnimationRenderer::slotButtonClicked(int button)
{
    if (button == KoDialog::Ok && !m_page->shouldExportOnlyImageSequence->isChecked()) {
        QString ffmpeg = m_page->ffmpegLocation->fileName();
        if (m_page->videoFilename->fileName().isEmpty()) {
            QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("Please enter a file name to render to."));
            return;
        }
        else if (ffmpeg.isEmpty()) {
            QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("<b>Krita can't find FFmpeg!</b><br> \
               <i>Krita depends on another free program called FFmpeg to turn frame-by-frame animations into video files. (<a href=\"https://www.ffmpeg.org\">www.ffmpeg.org</a>)</i><br><br>\
               To learn more about setting up Krita for rendering animations, <a href=\"https://docs.krita.org/en/reference_manual/render_animation.html#setting-up-krita-for-exporting-animations\">please visit this section of our User Manual.</a>"));
            return;
        }
        else {
            QFileInfo fi(ffmpeg);
            if (!fi.exists()) {
                QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("The location of FFmpeg is invalid. Please select the correct location of the FFmpeg executable on your system."));
                return;
            }
            if (fi.fileName().endsWith("zip")) {
                QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("Please extract ffmpeg from the archive first."));
                return;
            }

            if (fi.fileName().endsWith("tar.bz2")) {
#ifdef Q_OS_WIN
                QMessageBox::warning(this, i18nc("@title:window", "Krita"),
                                     i18n("This is a source code archive. Please download the application archive ('Windows Builds'), unpack it and then provide the path to the extracted ffmpeg file."));
#else
                QMessageBox::warning(this, i18nc("@title:window", "Krita"),
                                     i18n("This is a source code archive. Please install ffmpeg instead."));
#endif
                return;
            }

            if (!fi.isExecutable()) {
                QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("The location of FFmpeg is invalid. Please select the correct location of the FFmpeg executable on your system."));
                return;
            }


        }
    }
    KoDialog::slotButtonClicked(button);
}

void KisDlgAnimationRenderer::slotDialogAccepted()
{
    KisConfig cfg(false);
    KisAnimationRenderingOptions options = getEncoderOptions();
    saveLastUsedConfiguration("ANIMATION_EXPORT", options.toProperties());

    m_image->animationInterface()->setExportSequenceBaseName(options.basename);
    m_image->animationInterface()->setExportSequenceFilePath(options.directory);
    m_image->animationInterface()->setExportInitialFrameNumber(options.sequenceStart);
}

void KisDlgAnimationRenderer::slotExportTypeChanged()
{
    KisConfig cfg(false);

    bool willEncodeVideo =
        m_page->shouldExportAll->isChecked() || m_page->shouldExportOnlyVideo->isChecked();

    // if a video format needs to be outputted
    if (willEncodeVideo) {
         // videos always uses PNG for creating video, so disable the ability to change the format
         m_page->cmbMimetype->setEnabled(false);
         for (int i = 0; i < m_page->cmbMimetype->count(); ++i) {
             if (m_page->cmbMimetype->itemData(i).toString() == "image/png") {
                 m_page->cmbMimetype->setCurrentIndex(i);
                 break;
             }
         }
    }

    m_page->intWidth->setVisible(willEncodeVideo);
    m_page->intHeight->setVisible(willEncodeVideo);
    m_page->intFramesPerSecond->setVisible(willEncodeVideo);
    m_page->fpsLabel->setVisible(willEncodeVideo);
    m_page->cmbScaleFilter->setVisible(willEncodeVideo);
    m_page->scaleFilterLabel->setVisible(willEncodeVideo);
    m_page->lblWidth->setVisible(willEncodeVideo);
    m_page->lblHeight->setVisible(willEncodeVideo);

    // if only exporting video
    if (m_page->shouldExportOnlyVideo->isChecked()) {
        m_page->cmbMimetype->setEnabled(false); // allow to change image format
        m_page->imageSequenceOptionsGroup->setVisible(false);
        m_page->videoOptionsGroup->setVisible(false); //shrinks the horizontal space temporarily to help resize() work
        m_page->videoOptionsGroup->setVisible(true);
    }


    // if only an image sequence needs to be output
    if (m_page->shouldExportOnlyImageSequence->isChecked()) {
        m_page->cmbMimetype->setEnabled(true); // allow to change image format
        m_page->videoOptionsGroup->setVisible(false);
        m_page->imageSequenceOptionsGroup->setVisible(false);
        m_page->imageSequenceOptionsGroup->setVisible(true);
    }

    // show all options
     if (m_page->shouldExportAll->isChecked() ) {
         m_page->imageSequenceOptionsGroup->setVisible(true);
         m_page->videoOptionsGroup->setVisible(true);
     }


     // for the resize to work as expected, try to hide elements first before displaying other ones.
     // if the widget gets bigger at any point, the resize will use that, even if elements are hidden later to make it smaller
     resize(m_page->sizeHint());
}

void KisDlgAnimationRenderer::frameRateChanged(int framerate)
{
    const QString mimeType = m_page->cmbRenderType->itemData(m_page->cmbRenderType->currentIndex()).toString();
    m_page->lblGifWarningFPS->setVisible((mimeType == "image/gif" && framerate > 50));
}

void KisDlgAnimationRenderer::slotLockAspectRatioDimensionsWidth(int width)
{
    Q_UNUSED(width);

    float aspectRatio = (float)m_image->width() / (float)m_image->height();

    // update height here
    float newHeight = m_page->intWidth->value() / aspectRatio  ;

    m_page->intHeight->setValue(newHeight);

}

void KisDlgAnimationRenderer::slotLockAspectRatioDimensionsHeight(int height)
{
    Q_UNUSED(height);

    float aspectRatio = (float)m_image->width() / (float)m_image->height();

    // update width here
     float newWidth = aspectRatio *  m_page->intHeight->value();

     m_page->intWidth->setValue(newWidth);
}
