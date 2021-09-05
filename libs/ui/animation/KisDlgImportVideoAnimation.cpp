/*
 *  SPDX-FileCopyrightText: 2021 Know Zero
 *  SPDX-FileCopyrightText: 2021 Eoin O'Neill <eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "KisDlgImportVideoAnimation.h"

#include <QStandardPaths>
#include <QRegExp>
#include <QtMath>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>

#include <KFormat>

#include "KoFileDialog.h"

#include <KisDocument.h>
#include <KisMainWindow.h>
#include <KisImportExportManager.h>
#include <kis_image.h>
#include <kis_image_animation_interface.h>
#include <kis_memory_statistics_server.h>
#include <kis_icon_utils.h>

#include "KisFFMpegWrapper.h"

KisDlgImportVideoAnimation::KisDlgImportVideoAnimation(KisMainWindow *mainWindow, KisView *activeView) :
    KoDialog(mainWindow),
    m_mainWindow(mainWindow),
    m_activeView(activeView)
{
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setWindowTitle(i18nc("@title:window", "Import Video Animation"));

    QWidget *page = new QWidget(this);
    m_ui.setupUi(page);
    setMainWidget(page);
    
    toggleInputControls(false);
    
    KisPropertiesConfigurationSP config = loadLastUsedConfiguration("ANIMATION_EXPORT");
    QFileInfo ffmpegFileInfo(config->getPropertyLazy("ffmpeg_path",""));
    QFileInfo ffprobeFileInfo(config->getPropertyLazy("ffprobe_path",""));
    
    dbgFile << "Config data =" << "ffmpeg:" << ffmpegFileInfo.absoluteFilePath() << "ffprobe:" << ffprobeFileInfo.absoluteFilePath();
    
    QJsonObject ffmpegInfo = KisFFMpegWrapper::findFFMpeg(ffmpegFileInfo.absoluteFilePath());

    if (ffmpegInfo["enabled"].toBool()) {
        m_ui.cmbFFMpegLocation->addItem(ffmpegInfo["path"].toString(),ffmpegInfo);
        
        if (ffprobeFileInfo.filePath().isEmpty())
            ffprobeFileInfo.setFile(ffmpegFileInfo.absoluteDir().filePath("ffprobe"));
        
    } else {
        enableButtonOk(false);
        m_ui.tabGeneral->setEnabled(false);
        QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("FFMpeg not found! Please add a path to FFMpeg in the \"Advanced\" tab"));
    }

    QJsonObject ffprobeInfo = KisFFMpegWrapper::findFFProbe(ffprobeFileInfo.absoluteFilePath());    
    
    if (ffprobeInfo["enabled"].toBool())
        m_ui.cmbFFProbeLocation->addItem(ffprobeInfo["path"].toString(),ffprobeInfo);
    
    m_ui.cmbFFProbeLocation->addItem("[Disabled]",QJsonObject({{"path",""},{"enabled",false}}));
    
    m_ui.filePickerButton->setIcon(KisIconUtils::loadIcon("folder"));
    m_ui.nextFrameButton->setIcon(KisIconUtils::loadIcon("arrow-right"));
    m_ui.prevFrameButton->setIcon(KisIconUtils::loadIcon("arrow-left"));
    
    m_ui.fpsSpinbox->setValue(24.0);
    m_ui.fpsSpinbox->setSuffix(i18nc("FPS as a unit following a value, like 60 FPS", " FPS"));

    m_ui.frameSkipSpinbox->setValue(1);
    m_ui.frameSkipSpinbox->setRange(1,20);
    
    m_ui.startExportingAtSpinbox->setValue(0.0);
    m_ui.startExportingAtSpinbox->setRange(0.0, 9999.0);
    m_ui.startExportingAtSpinbox->setSuffix(i18nc("Second as a unit following a value, like 60 s", " s"));
    
    m_ui.videoPreviewSlider->setTickInterval(1);
    m_ui.videoPreviewSlider->setValue(0);
    
    m_ui.exportDurationSpinbox->setValue(3.0);
    m_ui.exportDurationSpinbox->setSuffix(i18nc("Second as a unit following a value, like 60 s", " s"));
    
    m_ui.lblWarning->hide();

    connect(m_ui.cmbDocumentHandler, SIGNAL(currentIndexChanged(int)), SLOT(slotDocumentHandlerChanged(int)));    

    m_ui.cmbDocumentHandler->addItem(i18nc("Import video to New Document", "New Document"), "0");
    
    if (m_activeView && m_activeView->document()) {
        m_ui.cmbDocumentHandler->addItem(i18nc("Import video to Current Document", "Current Document"), "1");
        m_ui.cmbDocumentHandler->setCurrentIndex(1);
        m_ui.fpsDocumentLabel->setText(i18nc("Video importer: fps of the document you're importing into"
                                             , "<small>Document:\n %1 FPS</small>"
                                             , QString::number(m_activeView->document()->image()->animationInterface()->framerate()))
                                       );
    }

    m_ui.documentWidthSpinbox->setValue(0);
    m_ui.documentHeightSpinbox->setValue(0);
    m_ui.documentWidthSpinbox->setRange(1,100000);
    m_ui.documentHeightSpinbox->setRange(1,100000);    
    
    m_ui.videoWidthSpinbox->setValue(0);
    m_ui.videoHeightSpinbox->setValue(0);
    m_ui.videoWidthSpinbox->setRange(1,100000);
    m_ui.videoHeightSpinbox->setRange(1,100000);
    
    m_ui.cmbVideoScaleFilter->addItem(i18n("bicubic"), "bicubic");
    m_ui.cmbVideoScaleFilter->addItem(i18n("bilinear"), "bilinear");
    m_ui.cmbVideoScaleFilter->addItem(i18n("lanczos3"), "lanczos");
    m_ui.cmbVideoScaleFilter->addItem(i18n("neighbor"), "neighbor");
    m_ui.cmbVideoScaleFilter->addItem(i18n("spline"), "spline");
  
    m_ui.tabWidget->setCurrentIndex(0);

    mainWidget()->setMinimumSize( page->size() );
    mainWidget()->adjustSize();

    m_videoSliderTimer = new QTimer(this);
    m_videoSliderTimer->setSingleShot(true);

    connect(m_videoSliderTimer, SIGNAL(timeout()), SLOT(slotVideoTimerTimeout()));

    m_currentFrame = 0;
    CurrentFrameChanged(0);

    connect(m_ui.filePickerButton, SIGNAL(clicked()), SLOT(slotAddFile()));
    connect(m_ui.nextFrameButton, SIGNAL(clicked()), SLOT(slotNextFrame()));
    connect(m_ui.prevFrameButton, SIGNAL(clicked()), SLOT(slotPrevFrame()));
    connect(m_ui.currentFrameNumberInput, SIGNAL(valueChanged(int)), SLOT(slotFrameNumberChanged(int)));
    connect(m_ui.videoPreviewSlider, SIGNAL(valueChanged(int)), SLOT(slotVideoSliderChanged()));

    connect(m_ui.ffprobePickerButton, SIGNAL(clicked()), SLOT(slotFFProbeFile()));
    connect(m_ui.ffmpegPickerButton, SIGNAL(clicked()), SLOT(slotFFMpegFile()));

    connect(m_ui.exportDurationSpinbox, SIGNAL(valueChanged(qreal)), SLOT(slotImportDurationChanged(qreal)));
     
}

KisPropertiesConfigurationSP KisDlgImportVideoAnimation::loadLastUsedConfiguration(QString configurationID) {
    KisConfig globalConfig(true);
    return globalConfig.exportConfiguration(configurationID);
}

void KisDlgImportVideoAnimation::saveLastUsedConfiguration(QString configurationID, KisPropertiesConfigurationSP config)
{
    KisConfig globalConfig(false);
    globalConfig.setExportConfiguration(configurationID, config);
}


QStringList KisDlgImportVideoAnimation::renderFrames()
{
    QStringList frameList;


    if ( !m_videoWorkDir.mkpath(".") ) {
        QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("Failed to create a work directory, make sure you have write permission"));
        return frameList;
    }

    KisFFMpegWrapper *ffmpeg = nullptr;
    ffmpeg = new KisFFMpegWrapper(this);

    QStringList args;
    float exportDuration = m_ui.exportDurationSpinbox->value();
    float fps = m_ui.fpsSpinbox->value(); 

    if (exportDuration / fps > 100.0) {
        if (QMessageBox::warning(this, i18nc("Title for a messagebox", "Krita"),
                             i18n("Warning: you are trying to import more than 100 frames into Krita.\n\n"
                                  "This means you might be overloading your system.\n"
                                  "If you want to edit a clip larger than 100 frames, consider using a real video editor, like Kdenlive (https://kdenlive.org)."),
                                 QMessageBox::Ok | QMessageBox::Cancel,
                                 QMessageBox::Cancel) == QMessageBox::Cancel) {
            return frameList;
        }
    }


    args << "-ss" << QString::number(m_ui.startExportingAtSpinbox->value())
         << "-i" << m_videoInfo.file
         << "-t" << QString::number(exportDuration)
         << "-r" << QString::number(fps);


    if ( m_videoInfo.width != m_ui.videoWidthSpinbox->value() || m_videoInfo.height != m_ui.videoHeightSpinbox->value() ) {
        args << "-vf" <<  QString("scale=w=")
                                    .append(QString::number(m_ui.videoWidthSpinbox->value()))
                                    .append(":h=")
                                    .append(QString::number(m_ui.videoHeightSpinbox->value()))
                                    .append(":flags=")
                                    .append(m_ui.cmbVideoScaleFilter->currentData().toString());  
    }

    QJsonObject ffmpegInfo = m_ui.cmbFFMpegLocation->currentData().toJsonObject();
    QJsonObject ffprobeInfo = m_ui.cmbFFProbeLocation->currentData().toJsonObject();

    KisPropertiesConfigurationSP config = loadLastUsedConfiguration("ANIMATION_EXPORT");

    config->setProperty("ffmpeg_path", ffmpegInfo["path"].toString());
    config->setProperty("ffprobe_path", ffprobeInfo["path"].toString());

    saveLastUsedConfiguration("ANIMATION_EXPORT", config);

    struct KisFFMpegWrapperSettings ffmpegSettings;

    ffmpegSettings.processPath = ffmpegInfo["path"].toString();
    ffmpegSettings.args = args;
    ffmpegSettings.outputFile = m_videoWorkDir.filePath("output_%04d.png");
    ffmpegSettings.logPath = QDir::tempPath() + QDir::separator() + "krita" + QDir::separator() + "ffmpeg.log";
    ffmpegSettings.totalFrames = qCeil(exportDuration * fps);
    ffmpegSettings.progressMessage = i18nc("FFMPEG animated video import message. arg1: frame progress number. arg2: file suffix."
                                           , "Extracted %1 frames from %2 video.", "[progress]", "[suffix]");

    ffmpeg->startNonBlocking(ffmpegSettings);
    ffmpeg->waitForFinished();

    frameList = m_videoWorkDir.entryList(QStringList() << "output_*.png",QDir::Files);
    frameList.replaceInStrings("output_", m_videoWorkDir.absolutePath() + QDir::separator() + "output_");

    dbgFile << "Import frames list:" << frameList;

    if ( frameList.isEmpty() ) {
         QMessageBox::critical(this, i18nc("@title:window", "Krita"), i18n("Failed to export frames from video"));
    }

    return frameList;
}


QStringList KisDlgImportVideoAnimation::documentInfo() {
    QStringList documentInfoList;

    // We're looking for a possible profile here, otherwise it gets generated. Then we get the name.
    QString profileColorSpace = RGBAColorModelID.id();
    QString profileName = KoColorSpaceRegistry::instance()->p709SRGBProfile()->name();
    if (m_videoInfo.colorTransfer != TRC_UNSPECIFIED && m_videoInfo.colorPrimaries != PRIMARIES_UNSPECIFIED) {
        const KoColorProfile *profile = KoColorSpaceRegistry::instance()->profileFor(QVector<double>(), m_videoInfo.colorPrimaries, m_videoInfo.colorTransfer);
        profileName = profile->name();
        profileColorSpace = profile->colorModelID();
    }

    documentInfoList << QString::number(m_ui.frameSkipSpinbox->value())
                     << QString::number(m_ui.fpsSpinbox->value())
                     << QString::number(qCeil(m_ui.fpsSpinbox->value() * m_ui.exportDurationSpinbox->value()))
                     << m_videoInfo.file;

    if ( m_ui.cmbDocumentHandler->currentIndex() == 0 ) {
        documentInfoList << "0"
                         << QString::number(m_ui.documentWidthSpinbox->value())
                         << QString::number(m_ui.documentHeightSpinbox->value())
                         << QString::number(72)
                         << profileColorSpace
                         << m_videoInfo.colorDepth
                         << profileName;
    } else {
        documentInfoList << "1";
    }

    return documentInfoList;
}

void KisDlgImportVideoAnimation::slotAddFile()
{
    QStringList urls = showOpenFileDialog();

    if (!urls.isEmpty())
        loadVideoFile( urls[0] );

}

QStringList KisDlgImportVideoAnimation::makeVideoMimeTypesList()
{
    QStringList supportedMimeTypes = QStringList();
    supportedMimeTypes << "video/x-matroska";
    supportedMimeTypes << "image/gif";
    supportedMimeTypes << "image/apng"; 
    supportedMimeTypes << "image/png";
    supportedMimeTypes << "image/mov";       
    supportedMimeTypes << "video/ogg";
    supportedMimeTypes << "video/mp4";
    supportedMimeTypes << "video/mpeg";
    supportedMimeTypes << "video/webm";


    supportedMimeTypes << "*/All files";

    return supportedMimeTypes;
}

QStringList KisDlgImportVideoAnimation::showOpenFileDialog()
{
    KoFileDialog dialog(this, KoFileDialog::ImportFiles, "OpenDocument");
    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    dialog.setMimeTypeFilters( makeVideoMimeTypesList() );
    dialog.setCaption(i18n("Select your Video File"));

    return dialog.filenames();
}

void KisDlgImportVideoAnimation::cleanupWorkDir() 
{
    dbgFile << "Cleanup Animation import work directory";
    m_videoWorkDir.removeRecursively();
}


void KisDlgImportVideoAnimation::toggleInputControls(bool toggleBool) 
{
    enableButtonOk(toggleBool);
    m_ui.videoPreviewSlider->setEnabled(toggleBool);
    m_ui.currentFrameNumberInput->setEnabled(toggleBool);
    m_ui.nextFrameButton->setEnabled(toggleBool);
    m_ui.prevFrameButton->setEnabled(toggleBool);    
}

void KisDlgImportVideoAnimation::loadVideoFile(const QString &filename)
{
    const QFileInfo resultFileInfo(filename);
    const QDir videoDir(resultFileInfo.absolutePath());

    m_videoWorkDir.setPath(videoDir.filePath("Krita_Animation_Import_Temp"));

    if (m_videoWorkDir.exists()) cleanupWorkDir();

    QFontMetrics metrics(m_ui.fileLocationLabel->font());
    const int fileLabelWidth = m_ui.fileLocationLabel->width() > 400 ? m_ui.fileLocationLabel->width():400;

    QString elidedFileString = metrics.elidedText(resultFileInfo.absolutePath(), Qt::ElideMiddle, qFloor(fileLabelWidth*0.6))
                             + "/" + metrics.elidedText(resultFileInfo.fileName(), Qt::ElideMiddle, qFloor(fileLabelWidth*0.4));

    m_ui.fileLocationLabel->setText(elidedFileString);
    m_videoInfo = loadVideoInfo(filename);

    if ( m_videoInfo.file.isEmpty() ) return;

    QStringList textInfo;

    textInfo.append(i18nc("video importer: video file statistics", "Width: %1 px", QString::number(m_videoInfo.width)));
    textInfo.append(i18nc("video importer: video file statistics", "Height: %1 px", QString::number(m_videoInfo.height)));

    if (m_videoInfo.colorPrimaries != PRIMARIES_UNSPECIFIED && m_videoInfo.colorTransfer != TRC_UNSPECIFIED) {
        textInfo.append(i18nc("video importer: video file statistics"
                              , "Color Primaries: %1"
                              , KoColorProfile::getColorPrimariesName(m_videoInfo.colorPrimaries)));
        textInfo.append(i18nc("video importer: video file statistics"
                              , "Color Transfer: %1"
                              , KoColorProfile::getTransferCharacteristicName(m_videoInfo.colorTransfer)));
    }
    textInfo.append(i18nc("video importer: video file statistics", "Duration: %1 s", QString::number(m_videoInfo.duration, 'f', 2)));
    textInfo.append(i18nc("video importer: video file statistics", "Frames: %1", QString::number(m_videoInfo.frames)));
    textInfo.append(i18nc("video importer: video file statistics", "FPS: %1", QString::number(m_videoInfo.fps)));


    if ( m_videoInfo.hasOverriddenFPS ) {
        textInfo.append(i18nc("video importer: video file statistics", "*<font size='0.5em'><em>*FPS not right in file. Modified to see full duration</em></font>"));
    }

    m_ui.fpsSpinbox->setValue( qCeil(m_videoInfo.fps) );
    m_ui.fileLoadedDetails->setText(textInfo.join("\n"));


    m_ui.videoPreviewSlider->setRange(0, m_videoInfo.frames);
    m_ui.currentFrameNumberInput->setRange(0, m_videoInfo.frames);
    m_ui.exportDurationSpinbox->setRange(0, 9999.0);

    if (m_ui.cmbDocumentHandler->currentIndex() == 0) {
        m_ui.documentWidthSpinbox->setValue(m_videoInfo.width);
        m_ui.documentHeightSpinbox->setValue(m_videoInfo.height);
    }

    m_ui.videoWidthSpinbox->setValue(m_videoInfo.width);
    m_ui.videoHeightSpinbox->setValue(m_videoInfo.height);

    m_ui.exportDurationSpinbox->setValue(m_videoInfo.duration);

    CurrentFrameChanged(0);

    if ( m_videoInfo.file.isEmpty() ) {
        toggleInputControls(false);
    } else {
        toggleInputControls(true);
        updateVideoPreview();
    }


}


void KisDlgImportVideoAnimation::updateVideoPreview() 
{
    float currentDurration = ( m_videoInfo.stream != -1 ) ?  (m_currentFrame / m_videoInfo.fps):0;
    QStringList args;

    args << "-ss" << QString::number(currentDurration)
         << "-i" << m_videoInfo.file
         << "-v" << "quiet"
         << "-vframes" << "1"
         << "-vcodec" << "mjpeg"
         << "-f" << "image2pipe"
         << "pipe:1";
             
    struct KisFFMpegWrapperSettings ffmpegSettings;

    QJsonObject ffmpegInfo = m_ui.cmbFFMpegLocation->currentData().toJsonObject();
    QByteArray byteImage = KisFFMpegWrapper::runProcessAndReturn(ffmpegInfo["path"].toString(), args, 3000);

    if ( byteImage.isEmpty() ) {
        m_ui.thumbnailImageHolder->setText( m_videoInfo.frames == m_currentFrame ? "End of Video":"No Preview" );
    } else {
        QPixmap thumbnailPixmap;
        thumbnailPixmap.loadFromData(byteImage,"JFIF");
       
        m_ui.thumbnailImageHolder->setText("");
        m_ui.thumbnailImageHolder->setPixmap(thumbnailPixmap.scaled(m_ui.thumbnailImageHolder->width()
                                                                    , m_ui.thumbnailImageHolder->height()
                                                                    , Qt::KeepAspectRatio
                                                                    , Qt::SmoothTransformation));
    }

    m_ui.thumbnailImageHolder->show();

}


void KisDlgImportVideoAnimation::slotVideoTimerTimeout() 
{
    updateVideoPreview();
}

void KisDlgImportVideoAnimation::slotImportDurationChanged(qreal time)
{

    KisMemoryStatisticsServer::Statistics stats =
            KisMemoryStatisticsServer::instance()
            ->fetchMemoryStatistics(m_activeView ? m_activeView->image() : 0);
    const KFormat format;

    int resolution = m_videoInfo.width * m_videoInfo.height;
    int pixelSize = 4; //how do we even go about getting the bitdepth???
    if (m_activeView && m_ui.cmbDocumentHandler->currentIndex() > 0) {
        pixelSize = m_activeView->image()->colorSpace()->pixelSize() * 4;
    } else if (m_videoInfo.colorDepth == "U16"){
        pixelSize = 8;
    }
    int frames = m_videoInfo.fps * time + 2;
    // Sometimes, the potential size of the file is so big (a feature length film taking easily 970 gib), that we cannot put it into a number.
    // It's more efficient therefore to calculate the maximum amount of frames possible.

    int maxFrames = stats.totalMemoryLimit / resolution / pixelSize;

    QStringList warnings;

    QString text_frames = i18nc("part of warning in video importer."
                                , "WARNING, you are trying to import %1 frames, the maximum amount you can import is %2."
                                , frames
                                , maxFrames);
    warnings.append(text_frames);
    QString text_memory;

    QString text_video_editor = i18nc("part of warning in video importer.",
                                      "Use a <a href=\"https://kdenlive.org\">video editor</a> instead!");



    if (maxFrames < frames) {
        text_memory = i18nc("part of warning in video importer."
                            , "You do not have enough memory to load this many frames, the computer will be overloaded.");
        warnings.insert(0, "<span style=\"color:#ff1500;\">");
        warnings.append(text_memory);
        warnings.append(text_video_editor);
        m_ui.lblWarning->setVisible(true);
    } else if (maxFrames < frames * 2) {
        text_memory = i18nc("part of warning in video importer."
                            , "This will take over half the available memory, editing will be difficult.");
        warnings.insert(0, "<span style=\"color:#ffee00;\">");
        warnings.append(text_memory);
        warnings.append(text_video_editor);
        m_ui.lblWarning->setVisible(true);
    } else if (m_videoInfo.colorTransfer == TRC_ITU_R_BT_2100_0_HLG
               || m_videoInfo.colorTransfer == TRC_SMPTE_ST_428_1) {

        QString text_trc =  i18nc("part of warning in video importer."
                                  , "Krita does not support the video transfer curve (%1), it will be loaded as linear"
                                  , KoColorProfile::getTransferCharacteristicName(m_videoInfo.colorTransfer));
        warnings.append(text_trc);
    } else {
        m_ui.lblWarning->setVisible(false);
    }

    warnings.append("</span>");
    m_ui.lblWarning->setText(warnings.join(" "));

}

void KisDlgImportVideoAnimation::slotNextFrame()
{
    CurrentFrameChanged(m_currentFrame+1);
}

void KisDlgImportVideoAnimation::slotPrevFrame()
{
    CurrentFrameChanged(m_currentFrame-1);
}

void KisDlgImportVideoAnimation::slotFrameNumberChanged(int frame)
{
    CurrentFrameChanged(frame);
}


void KisDlgImportVideoAnimation::slotFFProbeFile()
{
    KoFileDialog dialog(this, KoFileDialog::OpenFile, i18n("Open FFProbe"));
    dialog.setDefaultDir(QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation).last());
    dialog.setCaption(i18n("Open FFProbe"));

    QStringList filenames = dialog.filenames();

    if (!filenames.isEmpty()) {
        QJsonObject ffprobeInfo = KisFFMpegWrapper::findFFProbe(filenames[0]);

        if (ffprobeInfo["enabled"].toBool() && ffprobeInfo["custom"].toBool()) {
            m_ui.cmbFFProbeLocation->addItem(filenames[0],ffprobeInfo);
            m_ui.cmbFFProbeLocation->setCurrentText(filenames[0]);
            return;
        }
        QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("FFProbe is invalid!"));
    }

}

void KisDlgImportVideoAnimation::slotFFMpegFile()
{
    KoFileDialog dialog(this, KoFileDialog::OpenFile, i18n("Open FFMpeg"));
    dialog.setDefaultDir(QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation).last());
    dialog.setCaption(i18n("Open FFMpeg"));

    QStringList filenames = dialog.filenames();

    if (!filenames.isEmpty()) {
        QJsonObject ffmpegInfo = KisFFMpegWrapper::findFFMpeg(filenames[0]);

        if (ffmpegInfo["enabled"].toBool()) {
            if (ffmpegInfo["custom"].toBool()) {
                m_ui.cmbFFMpegLocation->addItem(filenames[0],ffmpegInfo);
                m_ui.cmbFFMpegLocation->setCurrentText(filenames[0]);
            } else {
                QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("FFMpeg is invalid!"));
            }
            m_ui.tabGeneral->setEnabled(true);
            return;
        }

        m_ui.tabGeneral->setEnabled(false);
        QMessageBox::critical(this, i18nc("@title:window", "Krita"), i18n("No FFMpeg found!"));
    }

}

void KisDlgImportVideoAnimation::slotDocumentHandlerChanged(int selectedIndex)
{
    bool toggleDocumentOptions = selectedIndex == 0;

    if (toggleDocumentOptions) {
        m_ui.fpsDocumentLabel->setText(" ");
        
        if (m_videoInfo.stream != -1) {
            m_ui.documentWidthSpinbox->setValue(m_videoInfo.width);
            m_ui.documentHeightSpinbox->setValue(m_videoInfo.height);
        }

    } else if (m_activeView) {
        m_ui.fpsDocumentLabel->setText(i18nc("Video importer: fps of the document you're importing into"
                                             , "<small>Document:\n %1 FPS</small>"
                                             , QString::number(m_activeView->document()->image()->animationInterface()->framerate()))
                                       );
    }

    m_ui.optionsDocumentGroup->setEnabled(toggleDocumentOptions);

}

void KisDlgImportVideoAnimation::slotVideoSliderChanged()
{
    CurrentFrameChanged(m_ui.videoPreviewSlider->value());

    if (!m_videoSliderTimer->isActive()) m_videoSliderTimer->start(300);

}



void KisDlgImportVideoAnimation::CurrentFrameChanged(int frame)
{
    float currentSeconds = 0;

    // update frame and seconds model data if they have changed
    if (m_currentFrame != frame ) {
        dbgFile << "Frame change to:" << frame;
        m_currentFrame = frame;
        currentSeconds = m_currentFrame / m_videoInfo.fps;
    }

    // update UI components if they are out of sync
    if (m_currentFrame != m_ui.currentFrameNumberInput->value())
        m_ui.currentFrameNumberInput->setValue(m_currentFrame);
        
    if (m_currentFrame != m_ui.videoPreviewSlider->value())
        m_ui.videoPreviewSlider->setValue(m_currentFrame);
            
    m_ui.videoPreviewSliderValueLabel->setText( QString::number(currentSeconds, 'f', 2).append(i18nc("Second as a unit following a value, like 60 s", " s")) );
}

KisBasicVideoInfo KisDlgImportVideoAnimation::loadVideoInfo(const QString &inputFile)
{
    QJsonObject ffmpegInfo = m_ui.cmbFFMpegLocation->currentData().toJsonObject();
    QJsonObject ffprobeInfo = m_ui.cmbFFProbeLocation->currentData().toJsonObject();
    struct KisBasicVideoInfo videoInfoData;

    KisFFMpegWrapper *ffprobe = new KisFFMpegWrapper(this);

    bool supportedCodec = true;
    QJsonObject ffprobeJsonObj;

    if (ffprobeInfo["enabled"].toBool())
        ffprobeJsonObj=ffprobe->ffprobe(inputFile, ffprobeInfo["path"].toString());

    if ( !ffprobeInfo["enabled"].toBool() || ffprobeJsonObj["error"].toInt() > 1 ) {
        KisFFMpegWrapper *ffmpeg = new KisFFMpegWrapper(this);

        ffprobeJsonObj = ffmpeg->ffmpegProbe(inputFile, ffmpegInfo["path"].toString(), false);

        dbgFile << "ffmpeg probe1" << ffprobeJsonObj;

        QJsonObject ffprobeProgress = ffprobeJsonObj["progress"].toObject();

        videoInfoData.frames = ffprobeProgress["frame"].toString().toInt();
    }

    if ( ffprobeJsonObj["error"].toInt() == 0 ) {

        QJsonObject ffprobeFormat = ffprobeJsonObj["format"].toObject();
        QJsonArray ffprobeStreams = ffprobeJsonObj["streams"].toArray();

        videoInfoData.file = inputFile;
        for (const QJsonValueRef &streamItemRef : ffprobeStreams) {
            QJsonObject streamItemObj = streamItemRef.toObject();

            if ( streamItemObj["codec_type"].toString() == "video" ) {
                videoInfoData.stream = streamItemObj["index"].toInt();
                break;
            }
        }

        if ( videoInfoData.stream == -1 ) {
            QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("No video stream could be found!"));
            return {};       
        } 

        QJsonObject ffprobeSelectedStream = ffprobeStreams[videoInfoData.stream].toObject();

        if (!ffmpegInfo.value("decoder").toObject()[ffprobeSelectedStream["codec_name"].toString()].toBool())
            supportedCodec = false;


        videoInfoData.width = ffprobeSelectedStream["width"].toInt();
        videoInfoData.height = ffprobeSelectedStream["height"].toInt();
        videoInfoData.encoding = ffprobeSelectedStream["codec_name"].toString();
        videoInfoData.colorPrimaries = KisFFMpegWrapper::colorPrimariesFromName(ffprobeSelectedStream["color_primaries"].toString());
        videoInfoData.colorTransfer = KisFFMpegWrapper::transferCharacteristicsFromName(ffprobeSelectedStream["color_transfer"].toString());

        // bits_per_raw_sample was introduced in 2014.
        if (ffprobeSelectedStream.value("bits_per_raw_sample").toInt() > 8) {
            videoInfoData.colorDepth = Integer16BitsColorDepthID.id();
        } else {
            videoInfoData.colorDepth = Integer8BitsColorDepthID.id();
        }

        // frame rate comes back in odd format...so we need to do a bit of work so it is more usable. 
        // data will come back like "50/3"
        QStringList rawFrameRate = ffprobeSelectedStream["r_frame_rate"].toString().split('/');
        
        if (!rawFrameRate.isEmpty())
            videoInfoData.fps = qCeil(rawFrameRate[0].toFloat() / rawFrameRate[1].toFloat());
            
        if ( !ffprobeSelectedStream["nb_frames"].isNull() ) {
            videoInfoData.frames = ffprobeSelectedStream["nb_frames"].toString().toInt();
        }
            
        // Get duration from stream, if it doesn't exist such as on VP8 and VP9, try to get it out of format
        if ( !ffprobeSelectedStream["duration"].isNull() ) {
            videoInfoData.duration = ffprobeSelectedStream["duration"].toString().toFloat();
        } else if ( !ffprobeFormat["duration"].isNull() ) {
            videoInfoData.duration = ffprobeFormat["duration"].toString().toFloat();  
        } else if ( videoInfoData.frames ) {
            videoInfoData.duration = videoInfoData.frames / videoInfoData.fps;
        }

        dbgFile << "Initial video info from probe: "
                << "stream:" << videoInfoData.stream
                << "frames:" << videoInfoData.frames 
                << "duration:" << videoInfoData.duration 
                << "fps:" << videoInfoData.fps
                << "encoding:" << videoInfoData.encoding;

        if ( !videoInfoData.frames && !videoInfoData.duration ) {
            KisFFMpegWrapper *ffmpeg = new KisFFMpegWrapper(this);

            QJsonObject ffmpegJsonObj = ffmpeg->ffmpegProbe(inputFile, ffmpegInfo["path"].toString(), false);

            dbgFile << "ffmpeg probe2" << ffmpegJsonObj;

            if ( ffprobeJsonObj["error"].toInt() != 0 ) {
                QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("Failed to load video information"));
                return {};
            }

            QJsonObject ffmpegProgressJsonObj = ffmpegJsonObj["progress"].toObject();
            float ffmpegFPS = ffmpegProgressJsonObj["ffmpeg_fps"].toString().toFloat();

            videoInfoData.frames = ffmpegProgressJsonObj["frame"].toString().toInt();

            if (ffmpegFPS > 0 && videoInfoData.frames) {
                videoInfoData.duration = videoInfoData.frames / ffmpegFPS;
                videoInfoData.fps = ffmpegFPS;
            } else {
                videoInfoData.duration = ffmpegProgressJsonObj["out_time_ms"].toString().toFloat() / 1000000;
                if (videoInfoData.frames) videoInfoData.fps =  videoInfoData.frames / videoInfoData.duration;
            }

        }

    } else if ( ffprobeJsonObj["error"].toInt() == 1) {
        supportedCodec = false;
    }


    if (!supportedCodec) {
        QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("Your FFMpeg version does not support this format"));
        return {};       
    }


    // if there is no data on frames but duration and fps exists like OGV, try to estimate it
    if ( videoInfoData.fps && videoInfoData.duration && !videoInfoData.frames ) {
        videoInfoData.frames = qCeil( videoInfoData.fps * videoInfoData.duration );
    }

    dbgFile << "Final video info from probe: "
            << "stream:" << videoInfoData.stream
            << "frames:" << videoInfoData.frames 
            << "duration:" << videoInfoData.duration 
            << "fps:" << videoInfoData.fps
            << "encoding:" << videoInfoData.encoding;

    const float calculatedFrameRateByDuration = videoInfoData.frames / videoInfoData.duration;
    const int frameRateDifference = qAbs(videoInfoData.fps - calculatedFrameRateByDuration);

    if (frameRateDifference > 1) { 
        // something is not right with the frame rate with this file, so let's use our calculated value
        // to make  sure we get the whole duration
        videoInfoData.hasOverriddenFPS = true;
        videoInfoData.fps = calculatedFrameRateByDuration;
    } else {
        videoInfoData.hasOverriddenFPS = false;
    }

    return videoInfoData;
}


