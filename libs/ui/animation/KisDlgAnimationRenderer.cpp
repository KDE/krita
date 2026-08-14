/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDlgAnimationRenderer.h"

#include <QStandardPaths>
#include <QPluginLoader>
#include <QJsonObject>
#include <QJsonArray>
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
#include <kis_icon_utils.h>
#include <kis_image.h>
#include <kis_image_animation_interface.h>
#include <kis_time_span.h>
#include <KisImportExportManager.h>
#include <kis_config_widget.h>
#include <kis_signals_blocker.h>
#include <KisDocument.h>
#include <QHBoxLayout>
#include <kis_config.h>
#include <kis_file_name_requester.h>
#include <KoDialog.h>
#include "animation/KisHDRMetadataOptions.h"
#include "kis_slider_spin_box.h"
#include "kis_acyclic_signal_connector.h"
#include "KisVideoSaver.h"
#include "KisAnimationRenderingOptions.h"
#include "kis_image_config.h"

#ifdef Q_OS_ANDROID
#include <QButtonGroup>
#include <QJsonDocument>
#include "animation/KisMediaEncoderFormatPreferencesDialog.h"
#include "animation/KisMediaEncoderWrapper.h"
#else
#include "VideoExportOptionsDialog.h"
#include "animation/KisFFMpegWrapper.h"
#endif


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
    m_page->layout()->setContentsMargins(0, 0, 0, 0);

    {
        QIcon editIcon = KisIconUtils::loadIcon("document-edit");
        m_page->bnExportOptions->setIcon(editIcon);
        m_page->bnRenderOptions->setIcon(editIcon);
    }

    m_page->lblWarnings->hide();
    m_page->lblWarnings->setPixmap(KisIconUtils::loadIcon(QStringLiteral("dialog-warning")).pixmap(32, 32));

#ifdef Q_OS_ANDROID
    m_exportButtonGroup = new QButtonGroup(this);
    m_exportButtonGroup->addButton(m_page->bnExportImages);
    m_exportButtonGroup->addButton(m_page->bnExportVideo);
    m_page->shouldExportOnlyImageSequence->setChecked(false);
    m_page->shouldExportOnlyVideo->setChecked(false);
    m_page->shouldExportOnlyImageSequence->setCheckable(false);
    m_page->shouldExportOnlyVideo->setCheckable(false);
    m_page->pgImages->layout()->addWidget(m_page->shouldExportOnlyImageSequence);
    m_page->pgVideo->layout()->addWidget(m_page->shouldExportOnlyVideo);
    m_page->lblVideoFilenameTitle->hide();
    m_page->videoFilename->hide();
    m_page->lblDirRequester->hide();
    m_page->dirRequester->hide();
    m_page->lblFFMpegLocationTitle->hide();
    m_page->ffmpegLocation->hide();
    m_page->lblFFMpegVersionTitle->hide();
    m_page->lblFFMpegVersion->hide();
#else
    m_page->wdgExportButtons->hide();
    m_page->stkExport->hide();
#endif

    m_page->dirRequester->setMode(KoFileDialog::OpenDirectory);

    m_page->intStart->setMinimum(0);
    m_page->intStart->setMaximum(doc->image()->animationInterface()->documentPlaybackRange().end());
    m_page->intEnd->setMinimum(doc->image()->animationInterface()->documentPlaybackRange().start());

    m_page->intHeight->setMinimum(1);
    m_page->intHeight->setMaximum(100000);

    m_page->intWidth->setMinimum(1);
    m_page->intWidth->setMaximum(100000);

    // Setup audio...
    QVector<QFileInfo> audioFiles = doc->getAudioTracks();
    const bool hasAudio = audioFiles.count() > 0;
    m_page->chkIncludeAudio->setEnabled(hasAudio);

    // Setup image mimeTypes...
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

#ifdef Q_OS_ANDROID
    // Set up video export formats on Android. No ffmpeg here.
    m_videoFormatPreferences = loadVideoFormatPreferences();
    const QVector<KisMediaEncoderFormat *> videoFormats = KisMediaEncoderWrapper::getSupportedFormats();
    for (KisMediaEncoderFormat *videoFormat : videoFormats) {
        m_page->cmbRenderType->addItem(videoFormat->title(), QVariant(videoFormat->key()));
    }
#endif

    m_page->cmbScaleFilter->addItem(i18nc("bicubic filtering", "bicubic"), "bicubic");
    m_page->cmbScaleFilter->addItem(i18nc("bilinear filtering", "bilinear"), "bilinear");
    m_page->cmbScaleFilter->addItem(i18nc("lanczos3 filtering", "lanczos3"), "lanczos");
    m_page->cmbScaleFilter->addItem(i18nc("nearest neighbor filtering", "neighbor"), "neighbor");
    m_page->cmbScaleFilter->addItem(i18nc("spline filtering", "spline"), "spline");

#ifndef Q_OS_ANDROID
    m_page->videoFilename->setMode(KoFileDialog::SaveFile);
    m_page->ffmpegLocation->setMode(KoFileDialog::OpenFile);
#endif

    m_page->cmbRenderType->setPlaceholderText(i18nc("Not applicable. No render types without valid ffmpeg path.", "N/A"));

    {   // Establish connections...
        connect(m_page->bnExportOptions, SIGNAL(clicked()), this, SLOT(sequenceMimeTypeOptionsClicked()));
        connect(m_page->bnRenderOptions, SIGNAL(clicked()), this, SLOT(selectRenderOptions()));

#ifdef Q_OS_ANDROID
        connect(m_exportButtonGroup,
                QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
                this,
                &KisDlgAnimationRenderer::slotExportTypeChanged);
#else
        connect(m_page->shouldExportOnlyImageSequence, SIGNAL(toggled(bool)), this, SLOT(slotExportTypeChanged()));
        connect(m_page->shouldExportOnlyVideo, SIGNAL(toggled(bool)), this, SLOT(slotExportTypeChanged()));
#endif
        connect(m_page->cmbRenderType, SIGNAL(currentIndexChanged(int)), SLOT(slotRenderTypeChanged()));

        connect(m_page->intFramesPerSecond, SIGNAL(valueChanged(int)), SLOT(slotCheckWarnings()));
        connect(m_page->intWidth, SIGNAL(valueChanged(int)), SLOT(slotCheckWarnings()));
        connect(m_page->intHeight, SIGNAL(valueChanged(int)), SLOT(slotCheckWarnings()));

#ifndef Q_OS_ANDROID
        connect(m_page->ffmpegLocation, SIGNAL(fileSelected(QString)), SLOT(setFFmpegPath(QString)));
#endif

        connect(this, SIGNAL(accepted()), SLOT(slotDialogAccepted()));
    }


    // try to lock the width and height being updated
    KisAcyclicSignalConnector *constrainsConnector = new KisAcyclicSignalConnector(this);
    constrainsConnector->createCoordinatedConnector()->connectBackwardInt(m_page->intWidth, SIGNAL(valueChanged(int)), this, SLOT(slotLockAspectRatioDimensionsWidth(int)));
    constrainsConnector->createCoordinatedConnector()->connectForwardInt(m_page->intHeight, SIGNAL(valueChanged(int)), this, SLOT(slotLockAspectRatioDimensionsHeight(int)));

    {   // Initialize settings from last configuration...
        KisPropertiesConfigurationSP animProperties = loadLastConfiguration("ANIMATION_EXPORT");
        KisAnimationRenderingOptions options;
        options.fromProperties(animProperties);

        initializeRenderSettings(*doc, options);
    }

    setMainWidget(m_page);
    slotCheckWarnings();
}

KisDlgAnimationRenderer::~KisDlgAnimationRenderer()
{
    delete m_page;
}

void KisDlgAnimationRenderer::initializeRenderSettings(const KisDocument &doc, const KisAnimationRenderingOptions &lastUsedOptions)
{
#ifndef Q_OS_ANDROID
    // Initialize FFmpeg location... (!)
    KisConfig cfg(false);
    QString cfgFFmpegPath = cfg.ffmpegLocation();
#ifdef Q_OS_MACOS
    if (cfgFFmpegPath.isEmpty()) {
        QJsonObject ffmpegInfo =  KisFFMpegWrapper::findFFMpeg(cfgFFmpegPath);
        cfgFFmpegPath = (ffmpegInfo["enabled"].toBool()) ? ffmpegInfo["path"].toString() : "";
    }
#endif

    // Check known ffmpeg locations..
    QString likelyFFmpegPath = [&]() {
        // Check last used
        if (!lastUsedOptions.ffmpegPath.isEmpty() && QFileInfo(lastUsedOptions.ffmpegPath).isExecutable()) {
            return lastUsedOptions.ffmpegPath;
        }

        // Check krita config
        if (!cfgFFmpegPath.isEmpty() && QFileInfo(cfgFFmpegPath).isExecutable()) {
            return cfgFFmpegPath;
        }

        // Check standard paths
        QString systemFFmpeg = QStandardPaths::findExecutable("ffmpeg");
        if (!systemFFmpeg.isEmpty() && QFileInfo(systemFFmpeg).isExecutable()) {
           return systemFFmpeg;
        }

        // Find ffmpeg elsewhere...
        QJsonObject ffmpegJsonObj = KisFFMpegWrapper::findFFMpeg("");
        return (ffmpegJsonObj["enabled"].toBool()) ? ffmpegJsonObj["path"].toString() : "";
    }();

    if (!likelyFFmpegPath.isEmpty() && QFileInfo(likelyFFmpegPath).isExecutable()) {
        setFFmpegPath(likelyFFmpegPath);
    }
#endif

    const QString documentPath = m_doc->localFilePath();

    // Initialize these settings based on last used configuration when possible..
    if (!lastUsedOptions.lastDocumentPath.isEmpty() &&
        lastUsedOptions.lastDocumentPath == documentPath) {

        // If the file is the same as last time, we use the last used basename.
        m_page->txtBasename->setText(lastUsedOptions.basename);

        m_page->sequenceStart->setValue(lastUsedOptions.sequenceStart);
        m_page->intWidth->setValue(lastUsedOptions.width);
        m_page->intHeight->setValue(lastUsedOptions.height);

#ifdef Q_OS_ANDROID
        m_page->dirRequester->setStartDir(documentPath);
#else
        m_page->videoFilename->setStartDir(lastUsedOptions.resolveAbsoluteDocumentFilePath(documentPath));
        m_page->videoFilename->setFileName(lastUsedOptions.videoFileName);
        m_page->dirRequester->setStartDir(lastUsedOptions.resolveAbsoluteDocumentFilePath(documentPath));
#endif
        m_page->dirRequester->setFileName(lastUsedOptions.directory);

    } else {
        m_page->sequenceStart->setValue(m_image->animationInterface()->activePlaybackRange().start());
        m_page->intWidth->setValue(m_image->width());
        m_page->intHeight->setValue(m_image->height());

#ifdef Q_OS_ANDROID
        m_page->dirRequester->setStartDir(documentPath);
#else
        m_page->videoFilename->setStartDir(lastUsedOptions.resolveAbsoluteDocumentFilePath(documentPath));
        m_page->videoFilename->setFileName(defaultVideoFileName(m_doc, lastUsedOptions.videoMimeType));
        m_page->dirRequester->setStartDir(lastUsedOptions.resolveAbsoluteDocumentFilePath(documentPath));
#endif
        m_page->dirRequester->setFileName(lastUsedOptions.directory);
    }

    // Initialize FRAME render format...
    for (int i = 0; i < m_page->cmbMimetype->count(); ++i) {
        if (m_page->cmbMimetype->itemData(i).toString() == lastUsedOptions.frameMimeType) {
            m_page->cmbMimetype->setCurrentIndex(i);
            break;
        }
    }

    for (int i = 0; i < m_page->cmbScaleFilter->count(); ++i) {
        if (m_page->cmbScaleFilter->itemData(i).toString() == lastUsedOptions.scaleFilter) {
            m_page->cmbScaleFilter->setCurrentIndex(i);
            break;
        }
    }

    // Initialize VIDEO render format...
#ifdef Q_OS_ANDROID
    const QString &lastVideoType = lastUsedOptions.videoFormatKey;
#else
    const QString &lastVideoType = lastUsedOptions.videoMimeType;
#endif
    for (int i = 0; i < m_page->cmbRenderType->count(); ++i) {
        if (m_page->cmbRenderType->itemData(i).toString() == lastVideoType) {
            m_page->cmbRenderType->setCurrentIndex(i);
            break;
        }
    }

    m_page->chkOnlyUniqueFrames->setChecked(lastUsedOptions.wantsOnlyUniqueFrameSequence);
#ifdef Q_OS_ANDROID
    if (lastUsedOptions.shouldEncodeVideo) {
        m_page->bnExportVideo->setChecked(true);
    } else {
        m_page->bnExportImages->setChecked(true);
    }
#else
    m_page->shouldExportOnlyVideo->setChecked(lastUsedOptions.shouldEncodeVideo);
    m_page->shouldExportOnlyImageSequence->setChecked(!lastUsedOptions.shouldDeleteSequence);
#endif

    slotExportTypeChanged();

#ifndef Q_OS_ANDROID
    {
        KisPropertiesConfigurationSP settings = loadLastConfiguration("VIDEO_ENCODER");

        QStringList encodersPresent;
        Q_FOREACH(const QString& key, ffmpegEncoderTypes.keys()) {
            encodersPresent << ffmpegEncoderTypes[key];
        }

        getDefaultVideoEncoderOptions(lastUsedOptions.videoMimeType, settings,
                                      encodersPresent,
                                      &m_customFFMpegOptionsString,
                                      &m_wantsRenderWithHDR);
    }

    {
        KisPropertiesConfigurationSP settings = loadLastConfiguration("img_sequence/" + lastUsedOptions.frameMimeType);
        m_wantsRenderWithHDR = settings->getPropertyLazy("saveAsHDR", m_wantsRenderWithHDR);
    }

    m_page->ffmpegLocation->setFileName(likelyFFmpegPath);
    m_page->ffmpegLocation->setStartDir(QFileInfo(m_doc->localFilePath()).path());
    m_page->ffmpegLocation->setReadOnlyText(true);
#endif

    // Initialize these settings based on the current document context..
    m_page->intStart->setValue(doc.image()->animationInterface()->activePlaybackRange().start());
    m_page->intEnd->setValue(doc.image()->animationInterface()->activePlaybackRange().end());
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

    bool hasAudioLoaded = doc.getAudioTracks().count() > 0;
    m_page->chkIncludeAudio->setChecked(hasAudioLoaded);
    slotRenderTypeChanged();
}

bool KisDlgAnimationRenderer::wantImageSequenceExport() const
{
#ifdef Q_OS_ANDROID
    return !wantVideoExport();
#else
    return m_page->shouldExportOnlyImageSequence->isChecked();
#endif
}

bool KisDlgAnimationRenderer::wantVideoExport() const
{
#ifdef Q_OS_ANDROID
    return m_page->bnExportVideo->isChecked();
#else
    return m_page->shouldExportOnlyVideo->isChecked();
#endif
}

#ifndef Q_OS_ANDROID
void KisDlgAnimationRenderer::getDefaultVideoEncoderOptions(const QString &mimeType,
                                                            KisPropertiesConfigurationSP cfg,
                                                            const QStringList &availableEncoders,
                                                            QString *customFFMpegOptionsString,
                                                            bool *renderHDR)
{
    const KisVideoExportOptionsDialog::ContainerType containerType =
            KisVideoExportOptionsDialog::mimeToContainer(mimeType);

    QScopedPointer<KisVideoExportOptionsDialog> encoderConfigWidget(
        new KisVideoExportOptionsDialog(containerType, availableEncoders, KisHDRMetadataOptions(), 0));

    // we always enable HDR, letting the user to force it
    encoderConfigWidget->setSupportsHDR(true);
    encoderConfigWidget->setConfiguration(cfg);
    *customFFMpegOptionsString = encoderConfigWidget->customUserOptionsString();
    *renderHDR = encoderConfigWidget->videoConfiguredForHDR();
}
#endif

void KisDlgAnimationRenderer::filterSequenceMimeTypes(QStringList &mimeTypes)
{
    KritaUtils::filterContainer(mimeTypes, [](QString type) {
        return (type.startsWith("image/")
                || (type.startsWith("application/") &&
                    !type.startsWith("application/x-spriter")));
    });
}

#ifndef Q_OS_ANDROID
QStringList KisDlgAnimationRenderer::makeVideoMimeTypesList()
{
    QStringList supportedMimeTypes = QStringList();
    supportedMimeTypes << "video/x-matroska";
    supportedMimeTypes << "video/mp4";
    supportedMimeTypes << "video/webm";
    supportedMimeTypes << "image/gif";
    supportedMimeTypes << "image/apng";    
    supportedMimeTypes << "image/webp";       
    supportedMimeTypes << "video/ogg";

    return supportedMimeTypes;
}

bool meetsEncoderRequirementsForContainer(KisVideoExportOptionsDialog::ContainerType encoderType, const QStringList& encodersPresent) {
    QVector<KoID> encodersExpected = KisVideoExportOptionsDialog::encoderIdentifiers(encoderType);
    Q_FOREACH(const KoID &encoder, encodersExpected ) {
        if (encodersPresent.contains(encoder.id())) {
            return true;
        }
    }
    return false;
}

// For dependencies, see here:
// https://en.wikipedia.org/wiki/Comparison_of_video_container_formats
QStringList KisDlgAnimationRenderer::filterMimeTypeListByAvailableEncoders(const QStringList& input) 
{
    QStringList retValue;

    Q_FOREACH(const QString& mime, input) {
        if ( mime == "video/x-matroska" ) {
            if ( ffmpegCodecs.contains("h264") || ffmpegCodecs.contains("vp9") ) {
                QList<QString> encodersPresent;
                encodersPresent << ffmpegEncoderTypes["h264"] << ffmpegEncoderTypes["vp9"];
                if (meetsEncoderRequirementsForContainer(KisVideoExportOptionsDialog::MKV, encodersPresent))
                    retValue << mime;
            }
        } else if (mime == "video/mp4") {
            if ( ffmpegCodecs.contains("h264") || ffmpegCodecs.contains("vp9") ) {
                QList<QString> encodersPresent;
                encodersPresent << ffmpegEncoderTypes["h264"] << ffmpegEncoderTypes["vp9"];
                if (meetsEncoderRequirementsForContainer(KisVideoExportOptionsDialog::MP4, encodersPresent))
                    retValue << mime;
            }
        } else if (mime == "video/webm") {
            if ( ffmpegCodecs.contains("vp9") ) {
                QList<QString> encodersPresent;
                encodersPresent << ffmpegEncoderTypes["vp9"];
                if (meetsEncoderRequirementsForContainer(KisVideoExportOptionsDialog::WEBM, encodersPresent))
                    retValue << mime;
            }
        } else if (mime == "image/gif") {
            if ( ffmpegCodecs.contains("gif") ) {
                retValue << mime;
            }
        } else if (mime == "image/apng") {
            if ( ffmpegCodecs.contains("apng") ) {
                retValue << mime;
            }
        } else if (mime == "image/webp") {
            if ( ffmpegCodecs.contains("webp") ) {
                retValue << mime;
            }
        } else if (mime == "video/ogg") {
            if ( ffmpegCodecs.contains("theora") ) {
                retValue << mime;
            }
        }
    }

    return retValue;
}
#endif

bool KisDlgAnimationRenderer::imageMimeSupportsHDR(QString &mime)
{
    return (mime == "image/png");
}

#ifdef Q_OS_ANDROID
QVariantMap KisDlgAnimationRenderer::loadVideoFormatPreferences()
{
    KisPropertiesConfigurationSP settings = loadLastConfiguration(QStringLiteral("VIDEO_ENCODER"));
    QString s = settings->getString(QStringLiteral("format_preferences"));
    if (!s.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(s.toUtf8());
        if (doc.isObject()) {
            return doc.toVariant().toMap();
        }
    }
    return QVariantMap();
}

void KisDlgAnimationRenderer::saveVideoFormatPreferences(const QVariantMap &value)
{
    KisPropertiesConfigurationSP settings = new KisPropertiesConfiguration();
    settings->setProperty(QStringLiteral("format_preferences"),
                          QString::fromUtf8(QJsonDocument::fromVariant(value).toJson(QJsonDocument::Compact)));
    saveLastUsedConfiguration(QStringLiteral("VIDEO_ENCODER"), settings);
}
#endif

KisPropertiesConfigurationSP KisDlgAnimationRenderer::loadLastConfiguration(QString configurationID) {
    KisConfig globalConfig(true);
    return globalConfig.exportConfiguration(configurationID);
}

void KisDlgAnimationRenderer::saveLastUsedConfiguration(QString configurationID, KisPropertiesConfigurationSP config)
{
    KisConfig globalConfig(false);
    globalConfig.setExportConfiguration(configurationID, config);
}

bool KisDlgAnimationRenderer::looksLikeGif(const QString &videoType)
{
#ifdef Q_OS_ANDROID
    return videoType.contains(QStringLiteral(":gif"));
#else
    return videoType == QStringLiteral("image/gif");
#endif
}

bool KisDlgAnimationRenderer::supportsAudio(const QString &videoType)
{
#ifdef Q_OS_ANDROID
    KisMediaEncoderFormat *format = KisMediaEncoderWrapper::getFormatByKey(videoType);
    return format && format->supportsAudio();
#else
    return !videoType.startsWith(QStringLiteral("image/"));
#endif
}

KisHDRMetadataOptions KisDlgAnimationRenderer::optionsFromImage()
{
    KisHDRMetadataOptions hdrOptions;
    if (m_image->relativeContentLightLevelInformation()) {
        hdrOptions.clli = *m_image->relativeContentLightLevelInformation();
    }
    if (m_image->colorVolumeInformation()) {
        hdrOptions.cvi = *m_image->colorVolumeInformation();
    }
    if (m_image->hdrReferenceWhiteLightLevel()) {
        hdrOptions.hdrReferenceWhite = *m_image->hdrReferenceWhiteLightLevel();
    }
    return hdrOptions;
}

#ifndef Q_OS_ANDROID

void KisDlgAnimationRenderer::setFFmpegPath(const QString& path) {
    // Let's START with the assumption that user-specified ffmpeg path is invalid
    // and clear out all of the ffmpeg-specific fields to fill post-validation...
    m_page->cmbRenderType->setDisabled(true);
    m_page->bnRenderOptions->setDisabled(true);

    QString previousMimeType = m_page->cmbRenderType->currentData().toString();

    m_page->cmbRenderType->clear();
    ffmpegEncoderTypes.clear();

    // Validate FFmpeg binary and setup FFMpeg...
    if (validateFFmpeg(path)) {
        QJsonObject ffmpegJsonObj = KisFFMpegWrapper::findFFMpeg(path);
        ffmpegVersion = ffmpegJsonObj["enabled"].toBool() ? ffmpegJsonObj["version"].toString() : i18n("No valid FFmpeg binary supplied...");
        ffmpegCodecs = KisFFMpegWrapper::getSupportedCodecs(ffmpegJsonObj);

        // Build map of encoding types to their specific encoder support (e.g. h264 => libopenh264, h264, h264_vaapi or whatever)
        Q_FOREACH(const QString& codec, ffmpegCodecs) {
            QJsonObject codecjson = ffmpegJsonObj["codecs"].toObject()[codec].toObject();
            if ( codecjson["encoding"].toBool() ) {
                QJsonArray codecEncoders = codecjson["encoders"].toArray();

                // In the case where no specific codec "library" is specified but we do support
                // encoding, we simply push the type onto the list regardless. This basically
                // means that there's no "specific" requirements that we need to meet and
                // encoding should be possible.
                if (codecEncoders.size() == 0) {
                    codecEncoders.push_back(QJsonValue(codec));
                }

                Q_FOREACH(const QJsonValue& value, codecEncoders) {
                    if (ffmpegEncoderTypes.contains(codec)) {
                        ffmpegEncoderTypes[codec].push_back(value.toString());
                    } else {
                        ffmpegEncoderTypes.insert(codec, {value.toString()} );
                    }
                }
            }
        }

        KisConfig cfg(false);

        {   // Build list of supported container types and repopulate cmbRenderType.
            KisSignalsBlocker(m_page->cmbRenderType);

            QStringList supportedMimeTypes = makeVideoMimeTypesList();
            supportedMimeTypes = filterMimeTypeListByAvailableEncoders(supportedMimeTypes);

            int previousMimeTypeIndex = -1;
            Q_FOREACH (const QString &mime, supportedMimeTypes) {
                QString description = KisMimeDatabase::descriptionForMimeType(mime);
                if (description.isEmpty()) {
                    description = mime;
                }

                m_page->cmbRenderType->addItem(description, mime);
                if (mime == previousMimeType) {
                    previousMimeTypeIndex = m_page->cmbRenderType->count() - 1;
                }
            }

            const int indexCount = m_page->cmbRenderType->count();
            if (indexCount > 0) {
                if (previousMimeTypeIndex >= 0) {
                    m_page->cmbRenderType->setCurrentIndex(previousMimeTypeIndex % indexCount);
                } else {
                    m_page->cmbRenderType->setCurrentIndex(0);
                }

                selectRenderType(m_page->cmbRenderType->currentIndex());
                m_page->cmbRenderType->setDisabled(false);
                m_page->bnRenderOptions->setDisabled(false);
                connect(m_page->cmbRenderType, SIGNAL(currentIndexChanged(int)), this, SLOT(selectRenderType(int)));
            }
        }

        m_page->lblFFMpegVersion->setText(ffmpegVersion);

        // Store configuration..
        cfg.setFFMpegLocation(ffmpegJsonObj["path"].toString());

        slotCheckWarnings();
    }
}
#endif

void KisDlgAnimationRenderer::slotCheckWarnings()
{
    setUpdatesEnabled(false);
    updateWarnings();
    m_page->adjustSize();
    setUpdatesEnabled(true);
}

void KisDlgAnimationRenderer::updateWarnings()
{
    QStringList warnings;
    bool exportMayFail = false;

    if (wantVideoExport()) {
        QString videoType = m_page->cmbRenderType->itemData(m_page->cmbRenderType->currentIndex()).toString();
        bool gif = looksLikeGif(videoType);

#ifndef Q_OS_ANDROID
        const QRegularExpression minVerFFMpegRX(R"(^n{0,1}(?:[0-3]|4\.[01])[\.\-])");
        const QRegularExpressionMatch minVerFFMpegMatch = minVerFFMpegRX.match(ffmpegVersion);

        if (gif && minVerFFMpegMatch.hasMatch()) {
            warnings << i18nc("ffmpeg warning checks",
                              "FFmpeg must be at least version 4.2+ for GIF transparency to work");
        }
#endif

        int fps = m_page->intFramesPerSecond->value();
        if (gif && fps > 50) {
            warnings << i18nc("ffmpeg warning checks",
                              "Animated GIF images cannot have a framerate higher than 50. The framerate will be "
                              "reduced to 50 frames per second");
        }

        if (fps > 30) {
            exportMayFail = true;
            warnings << i18nc("ffmpeg warning checks", "FPS beyond 30 are not widely supported.");
        }

        if (m_page->intWidth->value() > 1920 || m_page->intHeight->value() > 1920) {
            exportMayFail = true;
            warnings << i18nc("ffmpeg warnings checks", "Dimensions larger than 1920 pixels are not widely supported.");
        }
    }

    if (warnings.isEmpty()) {
        m_page->lblWarnings->hide();
    } else {
        QString text = QString("<p><b>%1</b>").arg(i18n("Warning(s):"));
        text.append("<ul>");
        Q_FOREACH (const QString &warning, warnings) {
            text.append("<li>");
            text.append(warning.toHtmlEscaped());
            text.append("</li>");
        }
        text.append("</ul></p>");
        if (exportMayFail) {
            text.append(QStringLiteral("<p>"));
            text.append(
                i18nc("ffmpeg warning checks", "The export may fail and some devices may not be able to play it.")
                    .toHtmlEscaped());
            text.append(QStringLiteral("</p>"));
        }
        m_page->lblWarnings->setText(text);
        m_page->lblWarnings->show();
    }
}

#ifndef Q_OS_ANDROID
QString KisDlgAnimationRenderer::defaultVideoFileName(KisDocument *doc, const QString &mimeType)
{
    const QString docFileName = !doc->localFilePath().isEmpty() ? doc->localFilePath() : i18n("Untitled");

    if (!mimeType.isEmpty()) {
        return QString("%1.%2").arg(QFileInfo(docFileName).completeBaseName(),
                                    KisMimeDatabase::suffixesForMimeType(mimeType).first());
    } else {
        return docFileName;
    }
}

void KisDlgAnimationRenderer::selectRenderType(int index)
{
    if (m_page->cmbRenderType->count() == 0) return;

    const QString mimeType = m_page->cmbRenderType->itemData(index).toString();

    slotCheckWarnings();

    QString videoFileName = defaultVideoFileName(m_doc, mimeType);

    if (!m_page->videoFilename->fileName().isEmpty()) {
        const QFileInfo info = QFileInfo(m_page->videoFilename->fileName());
        const QString baseName = info.completeBaseName();
        const QString path = info.path();

        videoFileName = QString("%1%2%3.%4")
                            .arg(path, "/", baseName, KisMimeDatabase::suffixesForMimeType(mimeType).first());
    }
    m_page->videoFilename->setMimeTypeFilters(QStringList() << mimeType, mimeType);
    m_page->videoFilename->setFileName(videoFileName);

    m_wantsRenderWithHDR = (mimeType == "video/mp4") ? m_wantsRenderWithHDR : false;

    {   // We've got to reload the render settings to account for the user changing render type without configuration.
        // If this is removed from the configuration, ogg vorbis can fail to render on first attempt. BUG:421658
        // This should be revisited at some point, too much configuration juggling in this class makes it error-prone...

        QStringList encodersPresent;
        Q_FOREACH(const QString& key, ffmpegEncoderTypes.keys()) {
            encodersPresent << ffmpegEncoderTypes[key];
        }

        KisPropertiesConfigurationSP settings = loadLastConfiguration("VIDEO_ENCODER");
        getDefaultVideoEncoderOptions(mimeType, settings,
                                      encodersPresent,
                                      &m_customFFMpegOptionsString,
                                      &m_wantsRenderWithHDR);
    }
}
#endif

void KisDlgAnimationRenderer::selectRenderOptions()
{
#ifdef Q_OS_ANDROID
    QString key = m_page->cmbRenderType->currentData().toString();
    KisMediaEncoderFormat *format = KisMediaEncoderWrapper::getFormatByKey(key);
    KIS_SAFE_ASSERT_RECOVER_RETURN(format);

    KisMediaEncoderPreferencesDialog dlg(format, m_videoFormatPreferences.value(key).toMap(), this);
    if (dlg.exec() == QDialog::Accepted) {
        m_videoFormatPreferences.insert(key, dlg.preferences());
        saveVideoFormatPreferences(m_videoFormatPreferences);
    }
#else
    const int index = m_page->cmbRenderType->currentIndex();
    const QString mimetype = m_page->cmbRenderType->itemData(index).toString();

    const KisVideoExportOptionsDialog::ContainerType containerType =
        KisVideoExportOptionsDialog::mimeToContainer(mimetype);

    QStringList encodersPresent;
    Q_FOREACH(const QString& key, ffmpegEncoderTypes.keys()) {
        encodersPresent << ffmpegEncoderTypes[key];
    }

    KisHDRMetadataOptions hdrOptions = optionsFromImage();

    KisVideoExportOptionsDialog *encoderConfigWidget =
        new KisVideoExportOptionsDialog(containerType, encodersPresent, hdrOptions, this);

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
#ifndef Q_OS_ANDROID
        m_customFFMpegOptionsString = encoderConfigWidget->customUserOptionsString();
        m_wantsRenderWithHDR = encoderConfigWidget->videoConfiguredForHDR();
#endif
    }

    dlg.setMainWidget(0);
    encoderConfigWidget->deleteLater();
#endif
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
#ifndef Q_OS_ANDROID
            if(imageMimeSupportsHDR(mimetype)) {
                exportConfig->setProperty("saveAsHDR", m_wantsRenderWithHDR);
                if (m_wantsRenderWithHDR) {
                    exportConfig->setProperty("forceSRGB", false);
                }
            }
#endif

            frameExportConfigWidget->setConfiguration(exportConfig);
            KoDialog dlg(this);
            dlg.setMainWidget(frameExportConfigWidget);
            dlg.setButtons(KoDialog::Ok | KoDialog::Cancel);
            if (dlg.exec() == QDialog::Accepted) {
#ifndef Q_OS_ANDROID
                m_wantsRenderWithHDR = frameExportConfigWidget->configuration()->getPropertyLazy("saveAsHDR", false);
#endif
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

    options.lastDocumentPath = m_doc->localFilePath();
    QString videoType = m_page->cmbRenderType->currentData().toString();

#ifdef Q_OS_ANDROID
    bool video = wantVideoExport();
    options.shouldEncodeVideo = video;
    options.includeAudio = video && supportsAudio(videoType) && m_page->chkIncludeAudio->isChecked();
    options.wantsOnlyUniqueFrameSequence = !video && m_page->chkOnlyUniqueFrames->isChecked();

    if (video) {
        options.frameMimeType = QStringLiteral("image/png");
        options.videoFileName = m_videoFileName;
        options.videoFormatKey = videoType;
        QVariantMap videoFormatPreferences = m_videoFormatPreferences.value(videoType).toMap();
        if (!videoFormatPreferences.isEmpty()) {
            options.videoFormatPreferencesJson =
                QString::fromUtf8(QJsonDocument::fromVariant(videoFormatPreferences).toJson(QJsonDocument::Compact));
        }
    } else {
        options.directory = m_imageDirectory;
        options.frameMimeType = m_page->cmbMimetype->currentData().toString();
    }
#else
    options.videoMimeType = videoType;
    options.videoFileName = m_page->videoFilename->fileName();
    options.ffmpegPath = m_page->ffmpegLocation->fileName();
    options.customFFMpegOptions = m_customFFMpegOptionsString;
    options.directory = m_page->dirRequester->fileName();
    options.shouldEncodeVideo = wantVideoExport();
    options.shouldDeleteSequence = !wantImageSequenceExport();
    options.includeAudio = supportsAudio(videoType) && m_page->chkIncludeAudio->isChecked();
    options.wantsOnlyUniqueFrameSequence = m_page->chkOnlyUniqueFrames->isChecked();
    options.frameMimeType = m_page->cmbMimetype->currentData().toString();
#endif
    options.scaleFilter = m_page->cmbScaleFilter->currentData().toString();

    options.basename = m_page->txtBasename->text();
    options.firstFrame = m_page->intStart->value();
    options.lastFrame = m_page->intEnd->value();
    options.sequenceStart = m_page->sequenceStart->value();

    options.frameRate = m_page->intFramesPerSecond->value();
    if (options.frameRate > 50 && looksLikeGif(videoType)) {
        options.frameRate = 50;
    }

    options.width = m_page->intWidth->value();
    options.height = m_page->intHeight->value();

    {
        KisPropertiesConfigurationSP cfg = loadLastConfiguration("img_sequence/" + options.frameMimeType);
        if (cfg) {
            KisImportExportManager::fillStaticExportConfigurationProperties(cfg, m_image);
        }

#ifndef Q_OS_ANDROID
        const bool forceNecessaryHDRSettings = m_wantsRenderWithHDR && imageMimeSupportsHDR(options.frameMimeType);
        if (forceNecessaryHDRSettings) {
            KIS_SAFE_ASSERT_RECOVER_NOOP(options.frameMimeType == "image/png");
            cfg->setProperty("forceSRGB", false);
            cfg->setProperty("saveAsHDR", true);
        }
#endif

        options.frameExportConfig = cfg;
    }

    return options;
}

#ifndef Q_OS_ANDROID
KisDlgAnimationRenderer::FFmpegValidationResult KisDlgAnimationRenderer::validateFFmpeg(const QString &ffmpegPath)
{
    if (!ffmpegPath.isEmpty()) {
        QFileInfo ffmpegBinary(ffmpegPath);
        if (ffmpegBinary.exists() && ffmpegBinary.isExecutable()) {
            QStringList commpressedFormats{"zip", "7z", "tar.bz2"};
            Q_FOREACH(const QString& compressedFormat, commpressedFormats) {
                if (ffmpegBinary.fileName().endsWith(compressedFormat)) {
                    return FFmpegValidationResult::COMPRESSED_FORMAT;
                }
            }
            return FFmpegValidationResult::VALID;
        } else if (ffmpegBinary.exists()) {
            return FFmpegValidationResult::NOT_A_BINARY;
        }
    }
    return FFmpegValidationResult::INVALID;
}
#endif

#ifndef Q_OS_ANDROID
void KisDlgAnimationRenderer::slotButtonClicked(int button)
{
    if (button == KoDialog::Ok && !m_page->shouldExportOnlyImageSequence->isChecked()) {
        QString fileName = m_page->videoFilename->fileName();

        if (fileName.isEmpty()) {
            QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("Please enter a file name to render to."));
            return;
        }
        else {
            switch (validateFFmpeg(m_page->ffmpegLocation->fileName())) {
                case FFmpegValidationResult::COMPRESSED_FORMAT:
                    QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("The FFmpeg that you've given us appears to be compressed. Please try to extract FFmpeg from the archive first."));
                    return;
                case FFmpegValidationResult::NOT_A_BINARY:
                    QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("The FFmpeg that you've given us appears to be invalid. Please select the correct location of an FFmpeg executable on your system."));
                    return;
                default:
                    break;
            }
        }
    }
    KoDialog::slotButtonClicked(button);
}
#endif

void KisDlgAnimationRenderer::slotDialogAccepted()
{
#ifdef Q_OS_ANDROID
    m_imageDirectory.clear();
    m_videoFileName.clear();

    if (wantVideoExport()) {
        KisMediaEncoderFormat *format = KisMediaEncoderWrapper::getFormatByKey(m_page->cmbRenderType->currentData().toString());
        KIS_SAFE_ASSERT_RECOVER_RETURN(format);
        KoFileDialog dialog(this, KoFileDialog::SaveFile, QStringLiteral("ExportAnimation"));
        dialog.setMimeTypeFilters(QStringList(KisMimeDatabase::mimeTypeForSuffix(format->extension())));
        dialog.setDefaultDir(m_doc->localFilePath());
        m_videoFileName = dialog.filename();
        if (m_videoFileName.isEmpty()) {
            return;
        }

    } else {
        KoFileDialog dialog(this, KoFileDialog::OpenDirectory, QStringLiteral("ExportAnimation"));
        m_imageDirectory = dialog.filename();
        if (m_imageDirectory.isEmpty()) {
            return;
        }
    }
#endif

    KisConfig cfg(false);
    KisAnimationRenderingOptions options = getEncoderOptions();
    saveLastUsedConfiguration("ANIMATION_EXPORT", options.toProperties());

    m_image->animationInterface()->setExportSequenceBaseName(options.basename);
    m_image->animationInterface()->setExportSequenceFilePath(options.directory);
    m_image->animationInterface()->setExportInitialFrameNumber(options.sequenceStart);
}

void KisDlgAnimationRenderer::slotExportTypeChanged()
{
    setUpdatesEnabled(false);

#ifdef Q_OS_ANDROID
    if (wantVideoExport()) {
        m_page->stkExport->setCurrentWidget(m_page->pgVideo);
    } else {
        m_page->lblWarnings->hide();
        m_page->stkExport->setCurrentWidget(m_page->pgImages);
    }
#else
    // if a video format needs to be outputted
    if (m_page->shouldExportOnlyVideo->isChecked()) {
         // videos always uses PNG for creating video, so disable the ability to change the format
         m_page->cmbMimetype->setEnabled(false);
         m_page->cmbMimetype->setCurrentIndex(m_page->cmbMimetype->findData("image/png"));
    }

    /**
     * A fallback fix for a case when both checkboxes are unchecked
     */
    if (!m_page->shouldExportOnlyVideo->isChecked() &&
        !m_page->shouldExportOnlyImageSequence->isChecked()) {

         KisSignalsBlocker b(m_page->shouldExportOnlyImageSequence);
         m_page->shouldExportOnlyImageSequence->setChecked(true);
    }
#endif

    updateWarnings();
    m_page->adjustSize();
    setUpdatesEnabled(true);
}

void KisDlgAnimationRenderer::slotRenderTypeChanged()
{
    m_page->chkIncludeAudio->setVisible(m_page->cmbRenderType->count() != 0
                                        && supportsAudio(m_page->cmbRenderType->currentData().toString()));
    m_page->adjustSize();
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
