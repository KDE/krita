/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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

#include "DlgAnimationRenderer.h"

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
#include <kis_image.h>
#include <kis_image_animation_interface.h>
#include <kis_time_range.h>
#include <KisImportExportManager.h>
#include <kis_config_widget.h>
#include <KisDocument.h>
#include <QHBoxLayout>
#include <kis_config.h>
#include <kis_file_name_requester.h>
#include <KoDialog.h>
#include "kis_slider_spin_box.h"
#include "kis_acyclic_signal_connector.h"
#include "video_saver.h"
#include "KisAnimationRenderingOptions.h"
#include "video_export_options_dialog.h"


DlgAnimationRenderer::DlgAnimationRenderer(KisDocument *doc, QWidget *parent)
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

    m_page->intStart->setMinimum(doc->image()->animationInterface()->fullClipRange().start());
    m_page->intStart->setMaximum(doc->image()->animationInterface()->fullClipRange().end());
    m_page->intStart->setValue(doc->image()->animationInterface()->playbackRange().start());

    m_page->intEnd->setMinimum(doc->image()->animationInterface()->fullClipRange().start());
    // animators sometimes want to export after end frame
    //m_page->intEnd->setMaximum(doc->image()->animationInterface()->fullClipRange().end());
    m_page->intEnd->setValue(doc->image()->animationInterface()->playbackRange().end());



    m_page->intHeight->setMinimum(1);
    m_page->intHeight->setMaximum(10000);
    m_page->intHeight->setValue(doc->image()->height());

    m_page->intWidth->setMinimum(1);
    m_page->intWidth->setMaximum(10000);
    m_page->intWidth->setValue(doc->image()->width());

    // try to lock the width and height being updated
    KisAcyclicSignalConnector *constrainsConnector = new KisAcyclicSignalConnector(this);
    constrainsConnector->createCoordinatedConnector()->connectBackwardInt(m_page->intWidth, SIGNAL(valueChanged(int)), this, SLOT(slotLockAspectRatioDimensionsWidth(int)));
    constrainsConnector->createCoordinatedConnector()->connectForwardInt(m_page->intHeight, SIGNAL(valueChanged(int)), this, SLOT(slotLockAspectRatioDimensionsHeight(int)));

    m_page->intFramesPerSecond->setValue(doc->image()->animationInterface()->framerate());

    QFileInfo audioFileInfo(doc->image()->animationInterface()->audioChannelFileName());
    const bool hasAudio = audioFileInfo.exists();
    m_page->chkIncludeAudio->setEnabled(hasAudio);
    m_page->chkIncludeAudio->setChecked(hasAudio && !doc->image()->animationInterface()->isAudioMuted());

    QStringList mimes = KisImportExportManager::supportedMimeTypes(KisImportExportManager::Export);
    mimes.sort();
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

    setMainWidget(m_page);

    QVector<QString> supportedMimeType;
    supportedMimeType << "video/x-matroska";
    supportedMimeType << "image/gif";
    supportedMimeType << "video/ogg";
    supportedMimeType << "video/mp4";

    Q_FOREACH (const QString &mime, supportedMimeType) {
        QString description = KisMimeDatabase::descriptionForMimeType(mime);
        if (description.isEmpty()) {
            description = mime;
        }
        m_page->cmbRenderType->addItem(description, mime);
    }

    m_page->videoFilename->setMode(KoFileDialog::SaveFile);

    connect(m_page->bnExportOptions, SIGNAL(clicked()), this, SLOT(sequenceMimeTypeSelected()));
    connect(m_page->bnRenderOptions, SIGNAL(clicked()), this, SLOT(selectRenderOptions()));

    m_page->ffmpegLocation->setMode(KoFileDialog::OpenFile);

    m_page->cmbRenderType->setCurrentIndex(cfg.readEntry<int>("AnimationRenderer/render_type", 0));

    connect(m_page->shouldExportOnlyImageSequence, SIGNAL(toggled(bool)), this, SLOT(slotExportTypeChanged()));
    connect(m_page->shouldExportOnlyVideo, SIGNAL(toggled(bool)), this, SLOT(slotExportTypeChanged()));
    connect(m_page->shouldExportAll, SIGNAL(toggled(bool)), this, SLOT(slotExportTypeChanged()));

    // connect and cold init
    connect(m_page->cmbRenderType, SIGNAL(currentIndexChanged(int)), this, SLOT(selectRenderType(int)));
    selectRenderType(m_page->cmbRenderType->currentIndex());

    resize(m_page->sizeHint());

    connect(this, SIGNAL(accepted()), SLOT(slotDialogAccepted()));

    {
        KisPropertiesConfigurationSP settings = cfg.exportConfiguration("ANIMATION_EXPORT");

        KisAnimationRenderingOptions options;
        options.fromProperties(settings);

        loadAnimationOptions(options);

        /**
         * There is already a (modified) frames config in the options themselves,
         * but we should better read the one, generated by the config widget, because
         * it may have some changes made to the "last use type config".
         */
        m_frameExportConfig = cfg.exportConfiguration(options.frameMimeType);
    }


}

DlgAnimationRenderer::~DlgAnimationRenderer()
{
    delete m_page;
}

void DlgAnimationRenderer::getDefaultVideoEncoderOptions(const QString &mimeType,
                                                         KisPropertiesConfigurationSP cfg,
                                                         QString *customFFMpegOptionsString,
                                                         bool *forceHDRVideo)
{
    const VideoExportOptionsDialog::ContainerType containerType =
        mimeType == "video/ogg" ?
        VideoExportOptionsDialog::OGV :
        VideoExportOptionsDialog::DEFAULT;

    QScopedPointer<VideoExportOptionsDialog> encoderConfigWidget(
        new VideoExportOptionsDialog(containerType, 0));

    // we always enable HDR, letting the user to force it
    encoderConfigWidget->setSupportsHDR(true);
    encoderConfigWidget->setConfiguration(cfg);
    *customFFMpegOptionsString = encoderConfigWidget->customUserOptionsString();
    *forceHDRVideo = encoderConfigWidget->forceHDRModeForFrames();
}

void DlgAnimationRenderer::loadAnimationOptions(const KisAnimationRenderingOptions &options)
{
    const QString documentPath = m_doc->localFilePath();

    m_page->txtBasename->setText(options.basename);

    if (!options.lastDocuemntPath.isEmpty() &&
            options.lastDocuemntPath == documentPath) {

        m_page->intStart->setValue(options.firstFrame);
        m_page->intEnd->setValue(options.lastFrame);
        m_page->sequenceStart->setValue(options.sequenceStart);
        m_page->intWidth->setValue(options.width);
        m_page->intHeight->setValue(options.height);
        m_page->intFramesPerSecond->setValue(options.frameRate);

        m_page->videoFilename->setStartDir(options.resolveAbsoluteDocumentFilePath(documentPath));
        m_page->videoFilename->setFileName(options.videoFileName);

        m_page->dirRequester->setStartDir(options.resolveAbsoluteDocumentFilePath(documentPath));
        m_page->dirRequester->setFileName(options.directory);
    } else {
        m_page->intStart->setValue(m_image->animationInterface()->playbackRange().start());
        m_page->intEnd->setValue(m_image->animationInterface()->playbackRange().end());
        m_page->sequenceStart->setValue(m_image->animationInterface()->playbackRange().start());
        m_page->intWidth->setValue(m_image->width());
        m_page->intHeight->setValue(m_image->height());
        m_page->intFramesPerSecond->setValue(m_image->animationInterface()->framerate());

        m_page->videoFilename->setStartDir(options.resolveAbsoluteDocumentFilePath(documentPath));
        m_page->videoFilename->setFileName(defaultVideoFileName(m_doc, options.videoMimeType));

        m_page->dirRequester->setStartDir(options.resolveAbsoluteDocumentFilePath(documentPath));
        m_page->dirRequester->setFileName(options.directory);
    }

    for (int i = 0; i < m_page->cmbMimetype->count(); ++i) {
        if (m_page->cmbMimetype->itemData(i).toString() == options.frameMimeType) {
            m_page->cmbMimetype->setCurrentIndex(i);
            break;
        }
    }

    for (int i = 0; i < m_page->cmbRenderType->count(); ++i) {
        if (m_page->cmbRenderType->itemData(i).toString() == options.videoMimeType) {
            m_page->cmbRenderType->setCurrentIndex(i);
            break;
        }
    }

    m_page->chkIncludeAudio->setChecked(options.includeAudio);

    if (options.shouldDeleteSequence) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(options.shouldEncodeVideo);
        m_page->shouldExportOnlyVideo->setChecked(true);
    } else if (!options.shouldEncodeVideo) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(!options.shouldDeleteSequence);
        m_page->shouldExportOnlyImageSequence->setChecked(true);
    } else {
        m_page->shouldExportAll->setChecked(true); // export to both
    }


    {
        KisConfig cfg(true);
        KisPropertiesConfigurationSP settings = cfg.exportConfiguration("VIDEO_ENCODER");

        getDefaultVideoEncoderOptions(options.videoMimeType, settings,
                                      &m_customFFMpegOptionsString,
                                      &m_forceHDRVideo);
    }

    m_page->ffmpegLocation->setStartDir(QFileInfo(m_doc->localFilePath()).path());
    m_page->ffmpegLocation->setFileName(findFFMpeg(options.ffmpegPath));
}

QString DlgAnimationRenderer::defaultVideoFileName(KisDocument *doc, const QString &mimeType)
{
    const QString docFileName = !doc->localFilePath().isEmpty() ?
        doc->localFilePath() : i18n("Untitled");

    return
        QString("%1.%2")
            .arg(QFileInfo(docFileName).completeBaseName())
            .arg(KisMimeDatabase::suffixesForMimeType(mimeType).first());
}

void DlgAnimationRenderer::selectRenderType(int index)
{
    const QString mimeType = m_page->cmbRenderType->itemData(index).toString();
    QString videoFileName = defaultVideoFileName(m_doc, mimeType);

    if (!m_page->videoFilename->fileName().isEmpty()) {
        const QFileInfo info = QFileInfo(m_page->videoFilename->fileName());
        const QString baseName = info.completeBaseName();
        const QString path = info.path();

        videoFileName =
            QString("%1%2%3.%4").arg(path).arg(QDir::separator()).arg(baseName).arg(KisMimeDatabase::suffixesForMimeType(mimeType).first());

    }
    m_page->videoFilename->setMimeTypeFilters(QStringList() << mimeType, mimeType);
    m_page->videoFilename->setFileName(videoFileName);
}

void DlgAnimationRenderer::selectRenderOptions()
{
    const int index = m_page->cmbRenderType->currentIndex();
    const QString mimetype = m_page->cmbRenderType->itemData(index).toString();

    const VideoExportOptionsDialog::ContainerType containerType =
        mimetype == "video/ogg" ?
        VideoExportOptionsDialog::OGV :
        VideoExportOptionsDialog::DEFAULT;

    VideoExportOptionsDialog *encoderConfigWidget =
        new VideoExportOptionsDialog(containerType, this);

    // we always enable HDR, letting the user to force it
    encoderConfigWidget->setSupportsHDR(true);

    {
        KisConfig cfg(true);
        KisPropertiesConfigurationSP settings = cfg.exportConfiguration("VIDEO_ENCODER");
        encoderConfigWidget->setConfiguration(settings);
    }

    KoDialog dlg(this);
    dlg.setMainWidget(encoderConfigWidget);
    dlg.setButtons(KoDialog::Ok | KoDialog::Cancel);
    if (dlg.exec() == QDialog::Accepted) {
        KisConfig cfg(false);
        cfg.setExportConfiguration("VIDEO_ENCODER", encoderConfigWidget->configuration());
        m_customFFMpegOptionsString = encoderConfigWidget->customUserOptionsString();
    }

    dlg.setMainWidget(0);
    encoderConfigWidget->deleteLater();
}

void DlgAnimationRenderer::sequenceMimeTypeSelected()
{
    int index = m_page->cmbMimetype->currentIndex();

    KisConfigWidget *frameExportConfigWidget = 0;

    QString mimetype = m_page->cmbMimetype->itemData(index).toString();
    QSharedPointer<KisImportExportFilter> filter(KisImportExportManager::filterForMimeType(mimetype, KisImportExportManager::Export));
    if (filter) {
        frameExportConfigWidget = filter->createConfigurationWidget(0, KisDocument::nativeFormatMimeType(), mimetype.toLatin1());

        if (frameExportConfigWidget) {
            KisPropertiesConfigurationSP config = filter->lastSavedConfiguration("", mimetype.toLatin1());
            if (config) {
                KisImportExportManager::fillStaticExportConfigurationProperties(config, m_image);
            }

            frameExportConfigWidget->setConfiguration(config);
            KoDialog dlg(this);
            dlg.setMainWidget(frameExportConfigWidget);
            dlg.setButtons(KoDialog::Ok | KoDialog::Cancel);
            if (dlg.exec() == QDialog::Accepted) {
                m_frameExportConfig = frameExportConfigWidget->configuration();

                KisConfig cfg(false);
                cfg.setExportConfiguration(mimetype, frameExportConfigWidget->configuration());
            }

            frameExportConfigWidget->hide();
            dlg.setMainWidget(0);
            frameExportConfigWidget->setParent(0);
            frameExportConfigWidget->deleteLater();

        }
    }
}

inline int roundByTwo(int value) {
    return value + (value & 0x1);
}

KisAnimationRenderingOptions DlgAnimationRenderer::getEncoderOptions() const
{
    KisAnimationRenderingOptions options;

    options.lastDocuemntPath = m_doc->localFilePath();
    options.videoMimeType = m_page->cmbRenderType->currentData().toString();

    options.basename = m_page->txtBasename->text();
    options.directory = m_page->dirRequester->fileName();
    options.firstFrame = m_page->intStart->value();
    options.lastFrame = m_page->intEnd->value();
    options.sequenceStart = m_page->sequenceStart->value();

    options.shouldEncodeVideo = !m_page->shouldExportOnlyImageSequence->isChecked();
    options.shouldDeleteSequence = m_page->shouldExportOnlyVideo->isChecked();
    options.includeAudio = m_page->chkIncludeAudio->isChecked();

    options.ffmpegPath = m_page->ffmpegLocation->fileName();
    options.frameRate = m_page->intFramesPerSecond->value();
    options.width = roundByTwo(m_page->intWidth->value());
    options.height = roundByTwo(m_page->intHeight->value());
    options.videoFileName = m_page->videoFilename->fileName();

    options.customFFMpegOptions = m_customFFMpegOptionsString;

    // we should create **a copy** of the properties
    if (m_frameExportConfig) {
        KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration(*m_frameExportConfig);
        const bool forceHDR = m_forceHDRVideo && !m_page->shouldExportOnlyImageSequence->isChecked();
        if (forceHDR) {
            KIS_SAFE_ASSERT_RECOVER_NOOP(m_page->cmbMimetype->currentData().toString() == "image/png");
            cfg->setProperty("forceSRGB", false);
            cfg->setProperty("saveAsHDR", true);
        }
        options.frameExportConfig = cfg;
    }

    return options;
}

void DlgAnimationRenderer::slotButtonClicked(int button)
{
    if (button == KoDialog::Ok && !m_page->shouldExportOnlyImageSequence->isChecked()) {
        QString ffmpeg = m_page->ffmpegLocation->fileName();
        if (m_page->videoFilename->fileName().isEmpty()) {
            QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("Please enter a file name to render to."));
            return;
        }
        else if (ffmpeg.isEmpty()) {
            QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("The location of FFmpeg is unknown. Please install FFmpeg first: Krita cannot render animations without FFmpeg. (<a href=\"https://www.ffmpeg.org\">www.ffmpeg.org</a>)"));
            return;
        }
        else {
            QFileInfo fi(ffmpeg);
            if (!fi.exists()) {
                QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("The location of FFmpeg is invalid. Please select the correct location of the FFmpeg executable on your system."));
                return;
            }
        }
    }
    KoDialog::slotButtonClicked(button);
}

void DlgAnimationRenderer::slotDialogAccepted()
{
    KisConfig cfg(false);
    KisAnimationRenderingOptions options = getEncoderOptions();
    cfg.setExportConfiguration("ANIMATION_EXPORT", options.toProperties());
}

QString DlgAnimationRenderer::findFFMpeg(const QString &customLocation)
{
    QString result;

    QStringList proposedPaths;

    if (!customLocation.isEmpty()) {
        proposedPaths << customLocation;
        proposedPaths << customLocation + QDir::separator() + "ffmpeg";
    }

#ifndef Q_OS_WIN
    proposedPaths << QDir::homePath() + "/bin/ffmpeg";
    proposedPaths << "/usr/bin/ffmpeg";
    proposedPaths << "/usr/local/bin/ffmpeg";
#endif
    proposedPaths << KoResourcePaths::getApplicationRoot() +
        QDir::separator() + "bin" + QDir::separator() + "ffmpeg";

    Q_FOREACH (QString path, proposedPaths) {
        if (path.isEmpty()) continue;

#ifdef Q_OS_WIN
        path = QDir::toNativeSeparators(QDir::cleanPath(path));
        if (path.endsWith(QDir::separator())) {
            continue;
        }
        if (!path.endsWith(".exe")) {
            if (!QFile::exists(path)) {
                path += ".exe";
                if (!QFile::exists(path)) {
                    continue;
                }
            }
        }
#endif
        QProcess testProcess;
        testProcess.start(path, QStringList() << "-version");
        if (testProcess.waitForStarted(1000)) {
            testProcess.waitForFinished(1000);
        }

        const bool successfulStart =
            testProcess.state() == QProcess::NotRunning &&
            testProcess.error() == QProcess::UnknownError;

        if (successfulStart) {
            result = path;
            break;
        }
    }

    return result;
}

void DlgAnimationRenderer::slotExportTypeChanged()
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
    m_page->lblWidth->setVisible(willEncodeVideo);
    m_page->lblHeight->setVisible(willEncodeVideo);

    // if only exporting video
    if (m_page->shouldExportOnlyVideo->isChecked()) {
        m_page->cmbMimetype->setEnabled(false); // allow to change image format
        m_page->imageSequenceOptionsGroup->setVisible(false);
        m_page->videoOptionsGroup->setVisible(false); //shrinks the horizontal space temporarily to help resize() work
        m_page->videoOptionsGroup->setVisible(true);

        cfg.writeEntry<QString>("AnimationRenderer/export_type", "Video");
    }


    // if only an image sequence needs to be output
    if (m_page->shouldExportOnlyImageSequence->isChecked()) {
        m_page->cmbMimetype->setEnabled(true); // allow to change image format
        m_page->videoOptionsGroup->setVisible(false);
        m_page->imageSequenceOptionsGroup->setVisible(false);
        m_page->imageSequenceOptionsGroup->setVisible(true);

        cfg.writeEntry<QString>("AnimationRenderer/export_type", "ImageSequence");
    }

    // show all options
     if (m_page->shouldExportAll->isChecked() ) {
         m_page->imageSequenceOptionsGroup->setVisible(true);
         m_page->videoOptionsGroup->setVisible(true);

         cfg.writeEntry<QString>("AnimationRenderer/export_type", "VideoAndImageSequence");
     }


     // for the resize to work as expected, try to hide elements first before displaying other ones.
     // if the widget gets bigger at any point, the resize will use that, even if elements are hidden later to make it smaller
     resize(m_page->sizeHint());
}

void DlgAnimationRenderer::slotLockAspectRatioDimensionsWidth(int width)
{
    Q_UNUSED(width);

    float aspectRatio = (float)m_image->width() / (float)m_image->height();

    // update height here
    float newHeight = m_page->intWidth->value() / aspectRatio  ;

    m_page->intHeight->setValue(newHeight);

}

void DlgAnimationRenderer::slotLockAspectRatioDimensionsHeight(int height)
{
    Q_UNUSED(height);

    float aspectRatio = (float)m_image->width() / (float)m_image->height();

    // update width here
     float newWidth = aspectRatio *  m_page->intHeight->value();

     m_page->intWidth->setValue(newWidth);
}
