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
    
    m_page->cmbScaleFilter->addItem(i18nc("bicubic filtering", "bicubic"), "bicubic");
    m_page->cmbScaleFilter->addItem(i18nc("bilinear filtering", "bilinear"), "bilinear");
    m_page->cmbScaleFilter->addItem(i18nc("lanczos3 filtering", "lanczos3"), "lanczos");
    m_page->cmbScaleFilter->addItem(i18nc("nearest neighbor filtering", "neighbor"), "neighbor");
    m_page->cmbScaleFilter->addItem(i18nc("spline filtering", "spline"), "spline");
    
    m_page->videoFilename->setMode(KoFileDialog::SaveFile);

    m_page->ffmpegLocation->setMode(KoFileDialog::OpenFile);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    m_page->cmbRenderType->setPlaceholderText(i18nc("Not applicable. No render types without valid ffmpeg path.", "N/A"));
#endif

    {   // Establish connections...
        connect(m_page->bnExportOptions, SIGNAL(clicked()), this, SLOT(sequenceMimeTypeOptionsClicked()));
        connect(m_page->bnRenderOptions, SIGNAL(clicked()), this, SLOT(selectRenderOptions()));

        connect(m_page->shouldExportOnlyImageSequence, SIGNAL(toggled(bool)), this, SLOT(slotExportTypeChanged()));
        connect(m_page->shouldExportOnlyVideo, SIGNAL(toggled(bool)), this, SLOT(slotExportTypeChanged()));

        connect(m_page->intFramesPerSecond, SIGNAL(valueChanged(int)), SLOT(frameRateChanged(int)));

        connect(m_page->ffmpegLocation, SIGNAL(fileSelected(QString)), SLOT(setFFmpegPath(QString)));

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
}

KisDlgAnimationRenderer::~KisDlgAnimationRenderer()
{
    delete m_page;
}

void KisDlgAnimationRenderer::initializeRenderSettings(const KisDocument &doc, const KisAnimationRenderingOptions &lastUsedOptions)
{
    const QString documentPath = m_doc->localFilePath();

    // Initialize these settings based on last used configuration when possible..
    if (!lastUsedOptions.lastDocumentPath.isEmpty() &&
        lastUsedOptions.lastDocumentPath == documentPath) {

        // If the file is the same as last time, we use the last used basename.
        m_page->txtBasename->setText(lastUsedOptions.basename);

        m_page->sequenceStart->setValue(lastUsedOptions.sequenceStart);
        m_page->intWidth->setValue(lastUsedOptions.width);
        m_page->intHeight->setValue(lastUsedOptions.height);

        m_page->videoFilename->setStartDir(lastUsedOptions.resolveAbsoluteDocumentFilePath(documentPath));
        m_page->videoFilename->setFileName(lastUsedOptions.videoFileName);

        m_page->dirRequester->setStartDir(lastUsedOptions.resolveAbsoluteDocumentFilePath(documentPath));
        m_page->dirRequester->setFileName(lastUsedOptions.directory);
    } else {
        m_page->sequenceStart->setValue(m_image->animationInterface()->activePlaybackRange().start());
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

    // Skipping m_page->cmbRenderType for now, as it depends on the ffmpeg provided.

    for (int i = 0; i < m_page->cmbScaleFilter->count(); ++i) {
        if (m_page->cmbScaleFilter->itemData(i).toString() == lastUsedOptions.scaleFilter) {
            m_page->cmbScaleFilter->setCurrentIndex(i);
            break;
        }
    }

    m_page->chkOnlyUniqueFrames->setChecked(lastUsedOptions.wantsOnlyUniqueFrameSequence);

    m_page->shouldExportOnlyVideo->setChecked(lastUsedOptions.shouldEncodeVideo);
    m_page->shouldExportOnlyImageSequence->setChecked(!lastUsedOptions.shouldDeleteSequence);
    slotExportTypeChanged();

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

    // Initialize FFmpeg location... (!)
    KisConfig cfg(false);
    QString cfgFFmpegPath = cfg.ffmpegLocation();
#ifdef Q_OS_MACOS
    if (cfgFFmpegPath.isEmpty()) {
        QJsonObject ffmpegInfo =  KisFFMpegWrapper::findFFMpeg(cfgFFmpegPath);
        cfgFFmpegPath = (ffmpegInfo["enabled"].toBool()) ? ffmpegInfo["path"].toString() : "";
    }
#endif
    QString likelyFFmpegPath = [&]() {
        if (!cfgFFmpegPath.isEmpty()) {
            return cfgFFmpegPath;
        }

        if (!lastUsedOptions.ffmpegPath.isEmpty()) {
            return lastUsedOptions.ffmpegPath;
        }

        return QStandardPaths::findExecutable("ffmpeg");
    }();

    if (likelyFFmpegPath.isEmpty() || !QFileInfo(likelyFFmpegPath).isExecutable()) {
        QJsonObject ffmpegJsonObj = KisFFMpegWrapper::findFFMpeg("");

        likelyFFmpegPath = (ffmpegJsonObj["enabled"].toBool()) ? ffmpegJsonObj["path"].toString() : "";
    }

    m_page->ffmpegLocation->setFileName(likelyFFmpegPath);
    m_page->ffmpegLocation->setStartDir(QFileInfo(m_doc->localFilePath()).path());
    m_page->ffmpegLocation->setReadOnlyText(true);

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
}

void KisDlgAnimationRenderer::getDefaultVideoEncoderOptions(const QString &mimeType,
                                                            KisPropertiesConfigurationSP cfg,
                                                            const QStringList &availableEncoders,
                                                            QString *customFFMpegOptionsString,
                                                            bool *renderHDR)
{
    const KisVideoExportOptionsDialog::ContainerType containerType =
            KisVideoExportOptionsDialog::mimeToContainer(mimeType);

    QScopedPointer<KisVideoExportOptionsDialog> encoderConfigWidget(
        new KisVideoExportOptionsDialog(containerType, availableEncoders,   0));

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

void KisDlgAnimationRenderer::setFFmpegPath(const QString& path) {
    // Let's START with the assumption that user-specified ffmpeg path is invalid
    // and clear out all of the ffmpeg-specific fields to fill post-validation...
    m_page->cmbRenderType->setDisabled(true);
    m_page->bnRenderOptions->setDisabled(true);
    m_page->cmbRenderType->clear();
    ffmpegEncoderTypes.clear();

    // Validate FFmpeg binary and setup FFMpeg...
    if (validateFFmpeg()) {
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

            Q_FOREACH (const QString &mime, supportedMimeTypes) {
                QString description = KisMimeDatabase::descriptionForMimeType(mime);
                if (description.isEmpty()) {
                    description = mime;
                }

                m_page->cmbRenderType->addItem(description, mime);
            }

            const int indexCount = m_page->cmbRenderType->count();
            if (indexCount > 0) {
                const int desiredIndex = cfg.readEntry<int>("AnimationRenderer/render_type", 0);
                m_page->cmbRenderType->setCurrentIndex(desiredIndex % indexCount); // ;P
                selectRenderType(m_page->cmbRenderType->currentIndex());
                m_page->cmbRenderType->setDisabled(false);
                m_page->bnRenderOptions->setDisabled(false);
                connect(m_page->cmbRenderType, SIGNAL(currentIndexChanged(int)), this, SLOT(selectRenderType(int)));
            }
        }

        m_page->lblFFMpegVersion->setText(ffmpegVersion);

        // Store configuration..
        cfg.setFFMpegLocation(ffmpegJsonObj["path"].toString());

        ffmpegWarningCheck();
    }
}

void KisDlgAnimationRenderer::ffmpegWarningCheck() {
    const QString mimeType = m_page->cmbRenderType->itemData(m_page->cmbRenderType->currentIndex()).toString();

    const QRegularExpression minVerFFMpegRX(R"(^n{0,1}(?:[0-3]|4\.[01])[\.\-])");
    const QRegularExpressionMatch minVerFFMpegMatch = minVerFFMpegRX.match(ffmpegVersion);

    QStringList warnings;

    if (mimeType == "image/gif" && minVerFFMpegMatch.hasMatch()) {
        warnings << i18nc("ffmpeg warning checks", "FFmpeg must be at least version 4.2+ for GIF transparency to work");
    }

    // m_page->bnRenderOptions->setEnabled(mimeType != "image/gif" && mimeType != "image/webp" && mimeType !=
    // "image/png" );
    if (mimeType == "image/gif" && m_page->intFramesPerSecond->value() > 50) {
        warnings << i18nc("ffmpeg warning checks",
                          "Animated GIF images cannot have a framerate higher than 50. The framerate will be reduced "
                          "to 50 frames per second");
    }

    m_page->lblWarnings->setVisible(!warnings.isEmpty());

    if (!warnings.isEmpty()) {
        QString text = QString("<p><b>%1</b>").arg(i18n("Warning(s):"));
        text.append("<ul>");
        Q_FOREACH (const QString &warning, warnings) {
            text.append("<li>");
            text.append(warning.toHtmlEscaped());
            text.append("</li>");
        }
        text.append("</ul></p>");
        m_page->lblWarnings->setText(text);

        m_page->lblWarnings->setPixmap(
            m_page->lblWarnings->style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(QSize(32, 32)));
    }

    m_page->adjustSize();
}

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

    /*
    m_page->bnRenderOptions->setEnabled(mimeType != "image/gif" && mimeType != "image/webp" && mimeType != "image/png");
    */

    ffmpegWarningCheck();

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

void KisDlgAnimationRenderer::selectRenderOptions()
{
    const int index = m_page->cmbRenderType->currentIndex();
    const QString mimetype = m_page->cmbRenderType->itemData(index).toString();

    const KisVideoExportOptionsDialog::ContainerType containerType =
        KisVideoExportOptionsDialog::mimeToContainer(mimetype);

    QStringList encodersPresent;
    Q_FOREACH(const QString& key, ffmpegEncoderTypes.keys()) {
        encodersPresent << ffmpegEncoderTypes[key];
    }

    KisVideoExportOptionsDialog *encoderConfigWidget =
        new KisVideoExportOptionsDialog(containerType, encodersPresent, this);

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

    options.lastDocumentPath = m_doc->localFilePath();
    options.videoMimeType = m_page->cmbRenderType->currentData().toString();
    options.frameMimeType = m_page->cmbMimetype->currentData().toString();
    options.scaleFilter = m_page->cmbScaleFilter->currentData().toString();

    options.basename = m_page->txtBasename->text();
    options.directory = m_page->dirRequester->fileName();
    options.firstFrame = m_page->intStart->value();
    options.lastFrame = m_page->intEnd->value();
    options.sequenceStart = m_page->sequenceStart->value();

    options.shouldEncodeVideo = m_page->shouldExportOnlyVideo->isChecked();
    options.shouldDeleteSequence = !m_page->shouldExportOnlyImageSequence->isChecked();
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

bool KisDlgAnimationRenderer::validateFFmpeg(bool warn)
{
    QString ffmpeg = m_page->ffmpegLocation->fileName();

    if (!ffmpeg.isEmpty()) {
        QFileInfo file(ffmpeg);
        if (file.exists()) {
            QStringList commpressedFormats{"zip", "7z", "tar.bz2"};
            Q_FOREACH(const QString& compressedFormat, commpressedFormats) {
                if (file.fileName().endsWith(compressedFormat)) {
                    if (warn) {
                        QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("The FFmpeg that you've given us appears to be compressed. Please try to extract FFmpeg from the archive first."));
                    }

                    return false;
                }
            }

            if (file.isExecutable()) {
                return true;
            }
        }
    }

    if (warn) {
        QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("The FFmpeg that you've given us appears to be invalid. Please select the correct location of an FFmpeg executable on your system."));
    }

    return false;
}

void KisDlgAnimationRenderer::slotButtonClicked(int button)
{
    if (button == KoDialog::Ok && !m_page->shouldExportOnlyImageSequence->isChecked()) {
        QString fileName = m_page->videoFilename->fileName();

        if (fileName.isEmpty()) {
            QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("Please enter a file name to render to."));
            return;
        }
        else {
            if (!validateFFmpeg(true)) return;
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

    const bool willEncodeVideo = m_page->shouldExportOnlyVideo->isChecked();

    // if a video format needs to be outputted
    if (willEncodeVideo) {
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
}

void KisDlgAnimationRenderer::frameRateChanged(int framerate)
{
    Q_UNUSED(framerate);
    ffmpegWarningCheck();
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
