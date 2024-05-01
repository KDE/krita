/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "recorder_export.h"
#include "ui_recorder_export.h"
#include "recorder_export_config.h"
#include "recorder_export_settings.h"
#include "recorder_profile_settings.h"
#include "recorder_directory_cleaner.h"
#include "animation/KisFFMpegWrapper.h"

#include <klocalizedstring.h>
#include <kis_icon_utils.h>
#include "kis_config.h"

#include <QAction>
#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QUrl>
#include <QDebug>
#include <QCloseEvent>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QImageReader>
#include <QElapsedTimer>

#include "kis_debug.h"


namespace
{
enum ExportPageIndex
{
    PageSettings = 0,
    PageProgress = 1,
    PageDone = 2
};
}


class RecorderExport::Private
{
public:
    RecorderExport *q;
    QScopedPointer<Ui::RecorderExport> ui;
    RecorderExportSettings *settings;

    QScopedPointer<KisFFMpegWrapper> ffmpeg;
    RecorderDirectoryCleaner *cleaner = nullptr;

    QElapsedTimer elapsedTimer;

    int spinInputFPSMinValue = 0;
    int spinInputFPSMaxValue = 0;

    Private(RecorderExport *q_ptr)
        : q(q_ptr)
        , ui(new Ui::RecorderExport)
        , settings(q_ptr->settings)
    {
    }

    void checkFfmpeg()
    {
        const QJsonObject ffmpegJson = KisFFMpegWrapper::findFFMpeg(settings->ffmpegPath);
        const bool success = ffmpegJson["enabled"].toBool();
        const QIcon &icon = KisIconUtils::loadIcon(success ? "dialog-ok" : "window-close");
        const QList<QAction *> &actions = ui->editFfmpegPath->actions();
        QAction *action;

        if (!actions.isEmpty()) {
            action = actions.first();
            action->setIcon(icon);
        } else {
            action = ui->editFfmpegPath->addAction(icon, QLineEdit::TrailingPosition);
        }
        if (success) {
            const QJsonArray h264Encoders = ffmpegJson["codecs"].toObject()["h264"].toObject()["encoders"].toArray();
            settings->ffmpegPath = ffmpegJson["path"].toString();
            settings->h264Encoder = h264Encoders.contains("libopenh264") ? "libopenh264" : "libx264";
            ui->editFfmpegPath->setText(settings->ffmpegPath);
            action->setToolTip("Version: "+ffmpegJson["version"].toString()
                                +(ffmpegJson["codecs"].toObject()["h264"].toObject()["encoding"].toBool() ? "":" (MP4/MKV UNSUPPORTED)")
            );
        } else {
            ui->editFfmpegPath->setText(i18nc("This text is displayed instead of path to external tool in case of external tool is not found", "[NOT FOUND]"));
            action->setToolTip(i18n("FFmpeg executable location couldn't be detected, please install it or select its location manually"));
        }
        ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(success);
    }

    void fillComboProfiles()
    {
        QSignalBlocker blocker(ui->comboProfile);
        ui->comboProfile->clear();
        for (const RecorderProfile &profile : settings->profiles) {
            ui->comboProfile->addItem(profile.name);
        }
        blocker.unblock();
        ui->comboProfile->setCurrentIndex(settings->profileIndex);
    }

    void updateFrameInfo()
    {
        QDir dir(settings->inputDirectory, "*." % RecorderFormatInfo::fileExtension(settings->format),
                QDir::Name, QDir::Files | QDir::NoDotAndDotDot);
        const QStringList &frames = dir.entryList(); // dir.count() calls entryList().count() internally
        settings->framesCount = frames.count();
        if (settings->framesCount != 0) {
            const QString &fileName = settings->inputDirectory % QDir::separator() % frames.last();
            settings->imageSize = QImageReader(fileName).size();
            settings->imageSize.rwidth() &= ~1;
            settings->imageSize.rheight() &= ~1;
        }
    }

    void updateVideoFilePath()
    {
        if (settings->videoDirectory.isEmpty())
            settings->videoDirectory = RecorderExportConfig(true).videoDirectory();

        settings->videoFilePath = settings->videoDirectory
            % QDir::separator()
            % settings->videoFileName
            % "."
            % settings->profiles[settings->profileIndex].extension;
        QSignalBlocker blocker(ui->editVideoFilePath);
        ui->editVideoFilePath->setText(settings->videoFilePath);
    }

    void updateRatio(bool widthToHeight)
    {
        const float ratio = static_cast<float>(settings->imageSize.width()) / static_cast<float>(settings->imageSize.height());
        if (widthToHeight) {
            settings->size.setHeight(static_cast<int>(settings->size.width() / ratio));
        } else {
            settings->size.setWidth(static_cast<int>(settings->size.height() * ratio));
        }
        // make width and height even
        settings->size.rwidth() &= ~1;
        settings->size.rheight() &= ~1;
        QSignalBlocker blockerWidth(ui->spinScaleHeight);
        QSignalBlocker blockerHeight(ui->spinScaleWidth);
        ui->spinScaleHeight->setValue(settings->size.height());
        ui->spinScaleWidth->setValue(settings->size.width());
    }

    void updateFps(RecorderExportConfig &config, bool takeFromInputFps = false)
    {
        if (!settings->lockFps)
            return;

        if (takeFromInputFps) {
            settings->fps = settings->inputFps;
            config.setFps(settings->fps);
            ui->spinFps->setValue(settings->fps);
        } else {
            settings->inputFps = settings->fps;
            config.setInputFps(settings->inputFps);
            ui->spinInputFps->setValue(settings->inputFps);
        }
        updateVideoDuration();
    }

    bool tryAbortExport()
    {
        if (!ffmpeg)
            return true;

        if (QMessageBox::question(q, q->windowTitle(), i18n("Abort encoding the timelapse video?"))
            == QMessageBox::Yes) {
            cleanupFFMpeg();
            return true;
        }

        return false;
    }

    QStringList splitCommand(const QString &command)
    {
        QStringList args;
        QString tmp;
        int quoteCount = 0;
        bool inQuote = false;

        // handle quoting. tokens can be surrounded by double quotes
        // "hello world". three consecutive double quotes represent
        // the quote character itself.
        for (int i = 0; i < command.size(); ++i) {
            if (command.at(i) == QLatin1Char('"')) {
                ++quoteCount;
                if (quoteCount == 3) {
                    // third consecutive quote
                    quoteCount = 0;
                    tmp += command.at(i);
                }
                continue;
            }
            if (quoteCount) {
                if (quoteCount == 1)
                    inQuote = !inQuote;
                quoteCount = 0;
            }
            if (!inQuote && command.at(i).isSpace()) {
                if (!tmp.isEmpty()) {
                    args += tmp;
                    tmp.clear();
                }
            } else {
                tmp += command.at(i);
            }
        }
        if (!tmp.isEmpty())
            args += tmp;

        return args;
    }

    void startExport()
    {
        Q_ASSERT(ffmpeg == nullptr);

        updateFrameInfo();

        const QString &arguments = applyVariables(settings->profiles[settings->profileIndex].arguments);

        ffmpeg.reset(new KisFFMpegWrapper(q));
        QObject::connect(ffmpeg.data(), SIGNAL(sigStarted()), q, SLOT(onFFMpegStarted()));
        QObject::connect(ffmpeg.data(), SIGNAL(sigFinished()), q, SLOT(onFFMpegFinished()));
        QObject::connect(ffmpeg.data(), SIGNAL(sigFinishedWithError(QString)), q, SLOT(onFFMpegFinishedWithError(QString)));
        QObject::connect(ffmpeg.data(), SIGNAL(sigProgressUpdated(int)), q, SLOT(onFFMpegProgressUpdated(int)));

        KisFFMpegWrapperSettings FFmpegSettings;
        KisConfig cfg(true);
        FFmpegSettings.processPath = settings->ffmpegPath;
        FFmpegSettings.args = splitCommand(arguments);
        FFmpegSettings.outputFile = settings->videoFilePath;
        FFmpegSettings.batchMode = true; //TODO: Consider renaming to 'silent' mode, meaning no window for extra window handling...

        ffmpeg->startNonBlocking(FFmpegSettings);
        ui->labelStatus->setText(i18nc("Status for the export of the video record", "Starting FFmpeg..."));
        ui->buttonCancelExport->setEnabled(false);
        ui->progressExport->setValue(0);
        elapsedTimer.start();
    }

    void cleanupFFMpeg()
    {
        if (ffmpeg) {
            ffmpeg->reset();
            ffmpeg.reset();
        }
    }

    QString applyVariables(const QString &templateArguments)
    {
        const QSize &outSize = settings->resize ? settings->size : settings->imageSize;
        const int previewLength = settings->resultPreview ? settings->firstFrameSec : 0;
        const int resultLength = settings->extendResult ? settings->lastFrameSec : 0;
        const float transitionLength = settings->resultPreview ? 0.7 : 0;
        return QString(templateArguments)
               .replace("$IN_FPS", QString::number(settings->inputFps))
               .replace("$OUT_FPS", QString::number(settings->fps))
               .replace("$WIDTH", QString::number(outSize.width()))
               .replace("$HEIGHT", QString::number(outSize.height()))
               .replace("$FRAMES", QString::number(settings->framesCount))
               .replace("$INPUT_DIR", settings->inputDirectory)
               .replace("$FIRST_FRAME_SEC", QString::number(previewLength))
               .replace("$TRANSITION_LENGTH", QString::number(transitionLength))
               .replace("$H264_ENCODER", settings->h264Encoder)
               .replace("$LAST_FRAME_SEC", QString::number(resultLength))
               .replace("$EXT", RecorderFormatInfo::fileExtension(settings->format));
    }

    void updateVideoDuration()
    {
        long ms = (settings->framesCount * 1000L / (settings->inputFps ? settings->inputFps : 30));

        if (settings->resultPreview) {
            ms += (settings->firstFrameSec * 1000L);
        }

        if (settings->extendResult) {
            ms += (settings->lastFrameSec * 1000L);
        }

        ui->labelVideoDuration->setText(formatDuration(ms));
    }

    QString formatDuration(long durationMs)
    {
        QString result;
        const long ms = (durationMs % 1000) / 10;

        result += QString(".%1").arg(ms, 2, 10, QLatin1Char('0'));

        long duration = durationMs / 1000;
        const long seconds = duration % 60;
        result = QString("%1%2").arg(seconds, 2, 10, QLatin1Char('0')).arg(result);

        duration = duration / 60;
        const long minutes = duration % 60;
        if (minutes != 0) {
            result = QString("%1:%2").arg(minutes, 2, 10, QLatin1Char('0')).arg(result);

            duration = duration / 60;
            if (duration != 0)
                result = QString("%1:%2").arg(duration, 2, 10, QLatin1Char('0')).arg(result);
        }

        return result;
    }
};


RecorderExport::RecorderExport(RecorderExportSettings *s, QWidget *parent)
    : QDialog(parent)
    , settings(s)
    , d(new Private(this))
{
    d->ui->setupUi(this);
    d->spinInputFPSMaxValue = d->ui->spinInputFps->minimum();
    d->spinInputFPSMaxValue = d->ui->spinInputFps->maximum();
    d->ui->buttonBrowseDirectory->setIcon(KisIconUtils::loadIcon("view-preview"));
    d->ui->buttonBrowseFfmpeg->setIcon(KisIconUtils::loadIcon("folder"));
    d->ui->buttonEditProfile->setIcon(KisIconUtils::loadIcon("document-edit"));
    d->ui->buttonBrowseExport->setIcon(KisIconUtils::loadIcon("folder"));
    d->ui->buttonLockRatio->setIcon(settings->lockRatio ? KisIconUtils::loadIcon("locked") : KisIconUtils::loadIcon("unlocked"));
    d->ui->buttonLockFps->setIcon(settings->lockFps ? KisIconUtils::loadIcon("locked") : KisIconUtils::loadIcon("unlocked"));
    d->ui->buttonWatchIt->setIcon(KisIconUtils::loadIcon("media-playback-start"));
    d->ui->buttonShowInFolder->setIcon(KisIconUtils::loadIcon("folder"));
    d->ui->buttonRemoveSnapshots->setIcon(KisIconUtils::loadIcon("edit-delete"));
    d->ui->stackedWidget->setCurrentIndex(ExportPageIndex::PageSettings);
    d->ui->spinLastFrameSec->setEnabled(d->ui->extendResultCheckBox->isChecked());
    d->ui->spinFirstFrameSec->setEnabled(d->ui->resultPreviewCheckBox->isChecked());

    connect(d->ui->buttonBrowseDirectory, SIGNAL(clicked()), SLOT(onButtonBrowseDirectoryClicked()));
    connect(d->ui->spinInputFps, SIGNAL(valueChanged(int)), SLOT(onSpinInputFpsValueChanged(int)));
    connect(d->ui->spinFps, SIGNAL(valueChanged(int)), SLOT(onSpinFpsValueChanged(int)));
    connect(d->ui->resultPreviewCheckBox, SIGNAL(toggled(bool)), SLOT(onCheckResultPreviewToggled(bool)));
    connect(d->ui->spinFirstFrameSec, SIGNAL(valueChanged(int)), SLOT(onFirstFrameSecValueChanged(int)));
    connect(d->ui->extendResultCheckBox, SIGNAL(toggled(bool)), SLOT(onCheckExtendResultToggled(bool)));
    connect(d->ui->spinLastFrameSec, SIGNAL(valueChanged(int)), SLOT(onLastFrameSecValueChanged(int)));
    connect(d->ui->checkResize, SIGNAL(toggled(bool)), SLOT(onCheckResizeToggled(bool)));
    connect(d->ui->spinScaleWidth, SIGNAL(valueChanged(int)), SLOT(onSpinScaleWidthValueChanged(int)));
    connect(d->ui->spinScaleHeight, SIGNAL(valueChanged(int)), SLOT(onSpinScaleHeightValueChanged(int)));
    connect(d->ui->buttonLockRatio, SIGNAL(toggled(bool)), SLOT(onButtonLockRatioToggled(bool)));
    connect(d->ui->buttonLockFps, SIGNAL(toggled(bool)), SLOT(onButtonLockFpsToggled(bool)));
    connect(d->ui->buttonBrowseFfmpeg, SIGNAL(clicked()), SLOT(onButtonBrowseFfmpegClicked()));
    connect(d->ui->comboProfile, SIGNAL(currentIndexChanged(int)), SLOT(onComboProfileIndexChanged(int)));
    connect(d->ui->buttonEditProfile, SIGNAL(clicked()), SLOT(onButtonEditProfileClicked()));
    connect(d->ui->editVideoFilePath, SIGNAL(textChanged(QString)), SLOT(onEditVideoPathChanged(QString)));
    connect(d->ui->buttonBrowseExport, SIGNAL(clicked()), SLOT(onButtonBrowseExportClicked()));
    connect(d->ui->buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(onButtonExportClicked()));
    connect(d->ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(d->ui->buttonCancelExport, SIGNAL(clicked()), SLOT(onButtonCancelClicked()));
    connect(d->ui->buttonWatchIt, SIGNAL(clicked()), SLOT(onButtonWatchItClicked()));
    connect(d->ui->buttonShowInFolder, SIGNAL(clicked()), SLOT(onButtonShowInFolderClicked()));
    connect(d->ui->buttonRemoveSnapshots, SIGNAL(clicked()), SLOT(onButtonRemoveSnapshotsClicked()));
    connect(d->ui->buttonRestart, SIGNAL(clicked()), SLOT(onButtonRestartClicked()));
    connect(d->ui->resultPreviewCheckBox, SIGNAL(toggled(bool)), d->ui->spinFirstFrameSec, SLOT(setEnabled(bool)));
    connect(d->ui->extendResultCheckBox, SIGNAL(toggled(bool)), d->ui->spinLastFrameSec, SLOT(setEnabled(bool)));

    if (settings->realTimeCaptureMode)
        d->ui->buttonBox->button(QDialogButtonBox::Close)->setText("OK");
    d->ui->buttonBox->button(QDialogButtonBox::Save)->setText(i18n("Export"));
    d->ui->editVideoFilePath->installEventFilter(this);
}

RecorderExport::~RecorderExport()
{
}

void RecorderExport::setup()
{
    RecorderExportConfig config(true);
    d->updateFps(config);
    d->updateFrameInfo();

    if (settings->framesCount == 0) {
        d->ui->labelRecordInfo->setText(i18nc("Can't export recording because nothing to export", "No frames to export"));
        d->ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
    } else {
        d->ui->labelRecordInfo->setText(QString("%1: %2x%3 %4, %5 %6")
                                        .arg(i18nc("General information about recording", "Recording info"))
                                        .arg(settings->imageSize.width())
                                        .arg(settings->imageSize.height())
                                        .arg(i18nc("Pixel dimension suffix", "px"))
                                        .arg(settings->framesCount)
                                        .arg(i18nc("The suffix after number of frames", "frame(s)"))
                                       );
    }


    // Don't load lockFps flag from config, if liveCaptureMode was just set by the user
    config.loadConfiguration(settings, !settings->realTimeCaptureModeWasSet);
    settings->realTimeCaptureModeWasSet = false;

    d->ui->spinInputFps->setValue(settings->inputFps);
    d->ui->spinFps->setValue(settings->fps);
    d->ui->resultPreviewCheckBox->setChecked(settings->resultPreview);
    d->ui->spinFirstFrameSec->setValue(settings->firstFrameSec);
    d->ui->extendResultCheckBox->setChecked(settings->extendResult);
    d->ui->spinLastFrameSec->setValue(settings->lastFrameSec);
    d->ui->checkResize->setChecked(settings->resize);
    d->ui->spinScaleWidth->setValue(settings->size.width());
    d->ui->spinScaleHeight->setValue(settings->size.height());
    d->ui->buttonLockRatio->setChecked(settings->lockRatio);
    d->ui->buttonLockRatio->setIcon(settings->lockRatio ? KisIconUtils::loadIcon("locked") : KisIconUtils::loadIcon("unlocked"));
    d->ui->labelRealTimeCaptureNotion->setVisible(settings->realTimeCaptureMode);
    d->ui->buttonLockFps->setChecked(settings->lockFps);
    d->ui->buttonLockFps->setIcon(settings->lockFps ? KisIconUtils::loadIcon("locked") : KisIconUtils::loadIcon("unlocked"));
    d->fillComboProfiles();
    d->checkFfmpeg();
    d->updateVideoFilePath();
    d->updateVideoDuration();
}

void RecorderExport::closeEvent(QCloseEvent *event)
{
    if (!d->tryAbortExport())
        event->ignore();
}

void RecorderExport::reject()
{
    if (d->tryAbortExport())
        QDialog::reject();
}

void RecorderExport::onButtonBrowseDirectoryClicked()
{
    if (settings->framesCount != 0) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(settings->inputDirectory));
    } else {
        QMessageBox::warning(this, windowTitle(), i18nc("Can't browse frames of recording because no frames have been recorded", "No frames to browse."));
        return;
    }
}

void RecorderExport::onSpinInputFpsValueChanged(int value)
{
    settings->inputFps = value;
    RecorderExportConfig config(false);
    config.setInputFps(value);
    d->updateFps(config, true);
    d->updateVideoDuration();
}

void RecorderExport::onSpinFpsValueChanged(int value)
{
    settings->fps = value;
    RecorderExportConfig config(false);
    config.setFps(value);
    d->updateFps(config, false);
    d->updateVideoDuration();
}

void RecorderExport::onCheckResultPreviewToggled(bool checked)
{
    settings->resultPreview = checked;
    RecorderExportConfig(false).setResultPreview(checked);
    d->updateVideoDuration();
}

void RecorderExport::onFirstFrameSecValueChanged(int value)
{
    settings->firstFrameSec = value;
    RecorderExportConfig(false).setFirstFrameSec(value);
    d->updateVideoDuration();
}

void RecorderExport::onCheckExtendResultToggled(bool checked)
{
    settings->extendResult = checked;
    RecorderExportConfig(false).setExtendResult(checked);
    d->updateVideoDuration();
}

void RecorderExport::onLastFrameSecValueChanged(int value)
{
    settings->lastFrameSec = value;
    RecorderExportConfig(false).setLastFrameSec(value);
    d->updateVideoDuration();
}

void RecorderExport::onCheckResizeToggled(bool checked)
{
    settings->resize = checked;
    RecorderExportConfig(false).setResize(checked);
}

void RecorderExport::onSpinScaleWidthValueChanged(int value)
{
    settings->size.setWidth(value);
    if (settings->lockRatio)
        d->updateRatio(true);
    RecorderExportConfig(false).setSize(settings->size);
}

void RecorderExport::onSpinScaleHeightValueChanged(int value)
{
    settings->size.setHeight(value);
    if (settings->lockRatio)
        d->updateRatio(false);
    RecorderExportConfig(false).setSize(settings->size);
}

void RecorderExport::onButtonLockRatioToggled(bool checked)
{
    settings->lockRatio = checked;
    RecorderExportConfig config(false);
    config.setLockRatio(checked);
    if (settings->lockRatio) {
        d->updateRatio(true);
        config.setSize(settings->size);
    }
    d->ui->buttonLockRatio->setIcon(settings->lockRatio ? KisIconUtils::loadIcon("locked") : KisIconUtils::loadIcon("unlocked"));
}

void RecorderExport::onButtonLockFpsToggled(bool checked)
{
    settings->lockFps = checked;
    RecorderExportConfig config(false);
    config.setLockFps(checked);
    d->updateFps(config);
    if (settings->lockFps) {
        d->ui->buttonLockFps->setIcon(KisIconUtils::loadIcon("locked"));
        d->ui->spinInputFps->setMinimum(d->ui->spinFps->minimum());
        d->ui->spinInputFps->setMaximum(d->ui->spinFps->maximum());
    } else {
        d->ui->buttonLockFps->setIcon(KisIconUtils::loadIcon("unlocked"));
        d->ui->spinInputFps->setMinimum(d->spinInputFPSMinValue);
        d->ui->spinInputFps->setMaximum(d->spinInputFPSMaxValue);
    }

}

void RecorderExport::onButtonBrowseFfmpegClicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setFilter(QDir::Executable | QDir::Files);

    const QString &file = dialog.getOpenFileName(this,
                          i18n("Select FFmpeg Executable File"),
                          settings->ffmpegPath);
    if (!file.isEmpty()) {
        settings->ffmpegPath = file;
        RecorderExportConfig(false).setFfmpegPath(file);
        d->checkFfmpeg();
    }
}

void RecorderExport::onComboProfileIndexChanged(int index)
{
    settings->profileIndex = index;
    d->updateVideoFilePath();
    RecorderExportConfig(false).setProfileIndex(index);
}

void RecorderExport::onButtonEditProfileClicked()
{
    RecorderProfileSettings settingsDialog(this);

    connect(&settingsDialog, &RecorderProfileSettings::requestPreview, [&](const QString & arguments) {
        settingsDialog.setPreview(settings->ffmpegPath % " -y " % d->applyVariables(arguments).replace("\n", " ")
                                  % " \"" % settings->videoFilePath % "\"");
    });

    if (settingsDialog.editProfile(
            &settings->profiles[settings->profileIndex], settings->defaultProfiles[settings->profileIndex])) {
        d->fillComboProfiles();
        d->updateVideoFilePath();
        RecorderExportConfig(false).setProfiles(settings->profiles);
    }
}

void RecorderExport::onEditVideoPathChanged(const QString &videoFilePath)
{
    QFileInfo fileInfo(videoFilePath);
    if (!fileInfo.isRelative())
        settings->videoDirectory = fileInfo.absolutePath();
    settings->videoFileName = fileInfo.completeBaseName();
}

void RecorderExport::onButtonBrowseExportClicked()
{
    QFileDialog dialog(this);

    const QString &extension = settings->profiles[settings->profileIndex].extension;
    const QString &videoFileName = dialog.getSaveFileName(this,
                                   i18n("Export Timelapse Video As"),
                                   settings->videoDirectory, "*." % extension);
    if (!videoFileName.isEmpty()) {
        QFileInfo fileInfo(videoFileName);
        settings->videoDirectory = fileInfo.absolutePath();
        settings->videoFileName = fileInfo.completeBaseName();
        d->updateVideoFilePath();
        RecorderExportConfig(false).setVideoDirectory(settings->videoDirectory);
    }
}

void RecorderExport::onButtonExportClicked()
{
    if (QFile::exists(settings->videoFilePath)) {
        if (settings->framesCount != 0) {
            if (QMessageBox::question(this, windowTitle(),
                                      i18n("The video file already exists. Do you wish to overwrite it?"))
                != QMessageBox::Yes) {
                return;
            }
        } else {
            QMessageBox::warning(this, windowTitle(), i18n("No frames to export."));
            return;
        }
    }


    d->ui->stackedWidget->setCurrentIndex(ExportPageIndex::PageProgress);
    d->startExport();
}

void RecorderExport::onButtonCancelClicked()
{
    if (d->cleaner) {
        d->cleaner->stop();
        d->cleaner->deleteLater();
        d->cleaner = nullptr;
        return;
    }

    if (d->tryAbortExport())
        d->ui->stackedWidget->setCurrentIndex(ExportPageIndex::PageSettings);
}


void RecorderExport::onFFMpegStarted()
{
    d->ui->buttonCancelExport->setEnabled(true);
    d->ui->labelStatus->setText(i18n("The timelapse video is being encoded..."));
}

void RecorderExport::onFFMpegFinished()
{
    quint64 elapsed = d->elapsedTimer.elapsed();
    d->ui->labelRenderTime->setText(d->formatDuration(elapsed));
    d->ui->stackedWidget->setCurrentIndex(ExportPageIndex::PageDone);
    d->ui->labelVideoPathDone->setText(settings->videoFilePath);
    d->cleanupFFMpeg();
}

void RecorderExport::onFFMpegFinishedWithError(QString error)
{
    d->ui->stackedWidget->setCurrentIndex(ExportPageIndex::PageSettings);
    QMessageBox::critical(this, windowTitle(), i18n("Export failed. FFmpeg message:") % "\n\n" % error);
    d->cleanupFFMpeg();
}

void RecorderExport::onFFMpegProgressUpdated(int frameNo)
{
    d->ui->progressExport->setValue(frameNo * 100 / (settings->framesCount * settings->fps / static_cast<float>(settings->inputFps)));
}

void RecorderExport::onButtonWatchItClicked()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(settings->videoFilePath));
}

void RecorderExport::onButtonShowInFolderClicked()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(settings->videoDirectory));
}

void RecorderExport::onButtonRemoveSnapshotsClicked()
{
    const QString confirmation(i18n("The recordings for this document will be deleted"
                                    " and you will not be able to export a timelapse for it again"
                                    ". Note that already exported timelapses will still be preserved."
                                    "\n\nDo you wish to continue?"));
    if (QMessageBox::question(this, windowTitle(), confirmation) != QMessageBox::Yes)
        return;

    d->ui->labelStatus->setText(i18nc("Label title, Snapshot directory deleting is in progress", "Cleaning up..."));
    d->ui->stackedWidget->setCurrentIndex(ExportPageIndex::PageProgress);

    Q_ASSERT(d->cleaner == nullptr);
    d->cleaner = new RecorderDirectoryCleaner({d->settings->inputDirectory});
    connect(d->cleaner, SIGNAL(finished()), this, SLOT(onCleanUpFinished()));
    d->cleaner->start();
}

void RecorderExport::onButtonRestartClicked()
{
    d->ui->stackedWidget->setCurrentIndex(ExportPageIndex::PageSettings);
}

void RecorderExport::onCleanUpFinished()
{
    d->cleaner->deleteLater();
    d->cleaner = nullptr;

    d->ui->stackedWidget->setCurrentIndex(ExportPageIndex::PageDone);
    d->ui->buttonRestart->hide();
    d->ui->buttonRemoveSnapshots->hide();
}

bool RecorderExport::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == d->ui->editVideoFilePath && event->type() == QEvent::FocusOut)
        d->updateVideoFilePath();

    return QDialog::eventFilter(obj, event);
}
