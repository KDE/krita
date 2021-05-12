#include "KisDlgImportVideoAnimation.h"

#include "KisDocument.h"
#include "KisMainWindow.h"
#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include "KisImportExportManager.h"
#include "KoFileDialog.h"
#include <QStandardPaths>
#include <QRegExp>
#include <QtMath>
#include <QJsonObject>
#include <QJsonArray>
#include <kis_icon_utils.h>

#include <QMessageBox>
#include "KisFFMpegWrapper.h"

KisDlgImportVideoAnimation::KisDlgImportVideoAnimation(KisMainWindow *mainWindow, KisView *activeView) :
    KoDialog(mainWindow),
    m_mainWindow(mainWindow),
    m_activeView(activeView)
{
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

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
    m_ui.fpsSpinbox->setSuffix(" FPS");

    m_ui.frameSkipSpinbox->setValue(1);
    m_ui.frameSkipSpinbox->setRange(1,20);
    
    m_ui.startExportingAtSpinbox->setValue(0.0);
    m_ui.startExportingAtSpinbox->setRange(0.0, 9999.0);
    m_ui.startExportingAtSpinbox->setSuffix(" s");
    
    m_ui.videoPreviewSlider->setTickInterval(1);
    m_ui.videoPreviewSlider->setValue(0);
    
    m_ui.exportDurationSpinbox->setValue(3.0);
    m_ui.exportDurationSpinbox->setSuffix(" s");
    
    m_ui.lblWarning->hide();

    connect(m_ui.cmbDocumentHandler, SIGNAL(currentIndexChanged(int)), SLOT(slotDocumentHandlerChanged(int)));    

    m_ui.cmbDocumentHandler->addItem("New Document", "0");
    
    if (m_activeView && m_activeView->document()) {
        m_ui.cmbDocumentHandler->addItem("Current Document", "1");
        m_ui.cmbDocumentHandler->setCurrentIndex(1);
        m_ui.fpsDocumentLabel->setText("<small>Document:<br>" + QString::number(m_activeView->document()->image()->animationInterface()->framerate()) + " FPS</small>");
    }

    m_ui.cmbDocumentColorModel->addItem("RGBA","RGBA");
    m_ui.cmbDocumentColorModel->addItem("XYZA","XYZA");
    m_ui.cmbDocumentColorModel->addItem("LABA","LABA");
    m_ui.cmbDocumentColorModel->addItem("CMYKA","CMYKA");
    m_ui.cmbDocumentColorModel->addItem("GRAYA","GRAYA");
    
    m_ui.cmbDocumentColorDepth->addItem("U8","U8");
    m_ui.cmbDocumentColorDepth->addItem("U16","U16");
    
    m_ui.cmbDocumentColorProfile->addItem("Default","");

    m_ui.documentResolutionSpinbox->setRange(0,10000);
    m_ui.documentResolutionSpinbox->setValue(120);

    m_ui.documentWidthSpinbox->setValue(0);
    m_ui.documentHeightSpinbox->setValue(0);
    m_ui.documentWidthSpinbox->setRange(1,100000);
    m_ui.documentHeightSpinbox->setRange(1,100000);    
    
    m_ui.videoWidthSpinbox->setValue(0);
    m_ui.videoHeightSpinbox->setValue(0);
    m_ui.videoWidthSpinbox->setRange(1,100000);
    m_ui.videoHeightSpinbox->setRange(1,100000);
    
    m_ui.cmbVideoScaleFilter->addItem("bicubic", "bicubic");
    m_ui.cmbVideoScaleFilter->addItem("bilinear", "bilinear");
    m_ui.cmbVideoScaleFilter->addItem("lanczos3", "lanczos");
    m_ui.cmbVideoScaleFilter->addItem("neighbor", "neighbor");
    m_ui.cmbVideoScaleFilter->addItem("spline", "spline");
  
    m_ui.tabWidget->setCurrentIndex(0);

    mainWidget()->setMinimumSize( page->size() );
    mainWidget()->adjustSize();

    videoSliderTimer = new QTimer(this);
    videoSliderTimer->setSingleShot(true);

    connect(videoSliderTimer, SIGNAL(timeout()), SLOT(slotVideoTimerTimeout()));

    currentFrame = 0;
    CurrentFrameChanged(0);

    connect(m_ui.filePickerButton, SIGNAL(clicked()), SLOT(slotAddFile()));
    connect(m_ui.nextFrameButton, SIGNAL(clicked()), SLOT(slotNextFrame()));
    connect(m_ui.prevFrameButton, SIGNAL(clicked()), SLOT(slotPrevFrame()));
    connect(m_ui.currentFrameNumberInput, SIGNAL(valueChanged(int)), SLOT(slotFrameNumberChanged(int)));
    connect(m_ui.videoPreviewSlider, SIGNAL(valueChanged(int)), SLOT(slotVideoSliderChanged()));

    connect(m_ui.ffprobePickerButton, SIGNAL(clicked()), SLOT(slotFFProbeFile()));
    connect(m_ui.ffmpegPickerButton, SIGNAL(clicked()), SLOT(slotFFMpegFile()));
     
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


    if ( !videoWorkDir.mkpath(".") ) {
        QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("Failed to create a work directory, make sure you have write permission"));
        return frameList;
    }

    KisFFMpegWrapper *ffmpeg = nullptr;
    ffmpeg = new KisFFMpegWrapper(this);

    QStringList args;
    float exportDuration = m_ui.exportDurationSpinbox->value();
    float fps = m_ui.fpsSpinbox->value(); 

    args << "-ss" << QString::number(m_ui.startExportingAtSpinbox->value())
         << "-i" << videoInfo.file
         << "-t" << QString::number(exportDuration)
         << "-r" << QString::number(fps);


    if ( videoInfo.width != m_ui.videoWidthSpinbox->value() || videoInfo.height != m_ui.videoHeightSpinbox->value() ) {         
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
    ffmpegSettings.outputFile = videoWorkDir.filePath("output_%04d.png");
    ffmpegSettings.totalFrames = qCeil(exportDuration * fps);
    ffmpegSettings.progressMessage = "Extracted [progress] frames from video...";

    ffmpeg->startNonBlocking(ffmpegSettings);
    ffmpeg->waitForFinished();

    frameList = videoWorkDir.entryList(QStringList() << "output_*.png",QDir::Files);
    frameList.replaceInStrings("output_", videoWorkDir.absolutePath() + QDir::separator() + "output_");

    dbgFile << "Import frames list:" << frameList;

    if ( frameList.isEmpty() ) {
         QMessageBox::critical(this, i18nc("@title:window", "Krita"), i18n("Failed to export frames from video"));
    }

    return frameList;
}


QStringList KisDlgImportVideoAnimation::documentInfo() {
    QStringList documentInfoList;

    documentInfoList << QString::number(m_ui.frameSkipSpinbox->value())
                     << QString::number(m_ui.fpsSpinbox->value())
                     << QString::number(qCeil(m_ui.fpsSpinbox->value() * m_ui.exportDurationSpinbox->value()))
                     << videoInfo.file;

    if ( m_ui.cmbDocumentHandler->currentIndex() == 0 ) {
        documentInfoList << "0"
                         << QString::number(m_ui.documentWidthSpinbox->value())
                         << QString::number(m_ui.documentHeightSpinbox->value())
                         << QString::number(m_ui.documentResolutionSpinbox->value())
                         << m_ui.cmbDocumentColorModel->currentData().toString()
                         << m_ui.cmbDocumentColorDepth->currentData().toString()
                         << m_ui.cmbDocumentColorProfile->currentData().toString();
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
    videoWorkDir.removeRecursively();
}


void KisDlgImportVideoAnimation::toggleInputControls(bool toggleBool) 
{
    enableButtonOk(toggleBool);
    m_ui.videoPreviewSlider->setEnabled(toggleBool);
    m_ui.currentFrameNumberInput->setEnabled(toggleBool);
    m_ui.nextFrameButton->setEnabled(toggleBool);
    m_ui.prevFrameButton->setEnabled(toggleBool);    
}

void KisDlgImportVideoAnimation::loadVideoFile(const QString &file) 
{
    const QFileInfo resultFileInfo(file);
    const QDir videoDir(resultFileInfo.absolutePath());

    videoWorkDir.setPath(videoDir.filePath("Krita_Animation_Import_Temp"));

    if (videoWorkDir.exists()) cleanupWorkDir();


    QFontMetrics metrics(m_ui.fileLocationLabel->font());
    const int fileLabelWidth = m_ui.fileLocationLabel->width() > 400 ? m_ui.fileLocationLabel->width():400;

    QString elidedFileString = metrics.elidedText(resultFileInfo.absolutePath(), Qt::ElideMiddle, qFloor(fileLabelWidth*0.6))
                             + "/" + metrics.elidedText(resultFileInfo.fileName(), Qt::ElideMiddle, qFloor(fileLabelWidth*0.4));

    m_ui.fileLocationLabel->setText(elidedFileString);
    videoInfo = loadVideoInfo(file);

    QString textInfo = "Width: " + QString::number(videoInfo.width) + "px" + "<br>"
                     + "Height: " + QString::number(videoInfo.height) + "px" + "<br>"
                     + "Duration: "  +  QString::number(videoInfo.duration, 'f', 2) + " s" + "<br>"
                     + "Frames: " + QString::number(videoInfo.frames) + "<br>"
                     + "FPS: " + QString::number(videoInfo.fps);

    if ( videoInfo.hasOverriddenFPS ) {
        textInfo += "*<br><font size='0.5em'><em>*FPS not right in file. Modified to see full duration</em></font>";
    }

    m_ui.lblWarning->setVisible(videoInfo.frames > 100);

    m_ui.fpsSpinbox->setValue( videoInfo.fps );
    m_ui.fileLoadedDetails->setText(textInfo);


    m_ui.videoPreviewSlider->setRange(0, videoInfo.frames); 
    m_ui.currentFrameNumberInput->setRange(0, videoInfo.frames);
    m_ui.exportDurationSpinbox->setRange(0, 9999.0);

    if (m_ui.cmbDocumentHandler->currentIndex() == 0) {
        m_ui.documentWidthSpinbox->setValue(videoInfo.width);
        m_ui.documentHeightSpinbox->setValue(videoInfo.height);        
    }

    m_ui.videoWidthSpinbox->setValue(videoInfo.width);
    m_ui.videoHeightSpinbox->setValue(videoInfo.height);

    m_ui.exportDurationSpinbox->setValue(videoInfo.duration);

    CurrentFrameChanged(0);

    if ( videoInfo.file.isEmpty() ) {
        toggleInputControls(false);
    } else {
        toggleInputControls(true);
        updateVideoPreview();
    }


}


void KisDlgImportVideoAnimation::updateVideoPreview() 
{
    int currentDurration = ( videoInfo.stream != -1 ) ?  (currentFrame / videoInfo.fps):0;
    QStringList args;

    args << "-ss" << QString::number(currentDurration)
         << "-i" << videoInfo.file
         << "-v" << "quiet"
         << "-vframes" << "1"
         << "-vcodec" << "mjpeg"
         << "-f" << "image2pipe"
         << "pipe:1";
             
    struct KisFFMpegWrapperSettings ffmpegSettings;

    QJsonObject ffmpegInfo = m_ui.cmbFFMpegLocation->currentData().toJsonObject();
    QByteArray byteImage = KisFFMpegWrapper::runProcessAndReturn(ffmpegInfo["path"].toString(), args, 3000);

    if ( byteImage.isEmpty() ) {
        m_ui.thumbnailImageHolder->setText( videoInfo.frames == currentFrame ? "End of Video":"No Preview" );
    } else {
        QPixmap thumbnailPixmap;
        thumbnailPixmap.loadFromData(byteImage,"JFIF");
       
        m_ui.thumbnailImageHolder->setText("");
        m_ui.thumbnailImageHolder->setPixmap(thumbnailPixmap);
        m_ui.thumbnailImageHolder->setScaledContents(true);
    }

    m_ui.thumbnailImageHolder->show();

}


void KisDlgImportVideoAnimation::slotVideoTimerTimeout() 
{
    updateVideoPreview();
}

void KisDlgImportVideoAnimation::slotNextFrame()
{
    CurrentFrameChanged(currentFrame+1);
}

void KisDlgImportVideoAnimation::slotPrevFrame()
{
    CurrentFrameChanged(currentFrame-1);
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
        
        if (videoInfo.stream != -1) {
            m_ui.documentWidthSpinbox->setValue(videoInfo.width);
            m_ui.documentHeightSpinbox->setValue(videoInfo.height);
        }

    } else if (m_activeView) {
        m_ui.fpsDocumentLabel->setText("<small>Document:<br>" + QString::number(m_activeView->document()->image()->animationInterface()->framerate()) + " FPS</small>");   
    }

    m_ui.optionsDocumentGroup->setEnabled(toggleDocumentOptions);

}

void KisDlgImportVideoAnimation::slotVideoSliderChanged()
{
    CurrentFrameChanged(m_ui.videoPreviewSlider->value());

    if (!videoSliderTimer->isActive()) videoSliderTimer->start(300);

}



void KisDlgImportVideoAnimation::CurrentFrameChanged(int frame)
{
    float currentSeconds = 0;

    // update frame and seconds model data if they have changed
    if (currentFrame != frame ) {
        dbgFile << "Frame change to:" << frame;
        currentFrame = frame;
        currentSeconds = currentFrame / videoInfo.fps; 
    }

    // update UI components if they are out of sync
    if (currentFrame != m_ui.currentFrameNumberInput->value())
        m_ui.currentFrameNumberInput->setValue(currentFrame);
        
    if (currentFrame != m_ui.videoPreviewSlider->value())
        m_ui.videoPreviewSlider->setValue(currentFrame);
            
    m_ui.videoPreviewSliderValueLabel->setText( QString::number(currentSeconds, 'f', 2).append(" s") );
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

            videoInfoData.frames = ffmpegProgressJsonObj["frame"].toString().toInt();
            if (videoInfoData.fps && videoInfoData.frames) {
                videoInfoData.duration = videoInfoData.frames / videoInfoData.fps;
            } else {
                videoInfoData.duration = ffmpegProgressJsonObj["out_time_ms"].toString().toInt() / 1000000;
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


