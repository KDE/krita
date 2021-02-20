/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "recorder_export.h"
#include "ui_recorder_export.h"
#include "recorder_export_config.h"
#include "recorder_ffmpeg_wrapper.h"
#include "recorder_profile_settings.h"
#include "recorder_directory_cleaner.h"

#include <klocalizedstring.h>
#include <kis_icon_utils.h>

#include <QAction>
#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QProcess>
#include <QUrl>
#include <QDebug>
#include <QCloseEvent>
#include <QMessageBox>

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
    Ui::RecorderExport *ui;
    RecorderExportSettings settings;
    QSize imageSize;

    int inputFps = 30;
    int fps = 30;
    bool resize = false;
    QSize size;
    bool lockRatio = false;
    QString ffmpegPath;
    QList<RecorderProfile> profiles;
    QList<RecorderProfile> defaultProfiles;
    int profileIndex = 0;
    QString videoDirectory;
    QString videoFileName;
    QString videoFilePath;
    int framesCount = 0;

    RecorderFFMpegWrapper *ffmpeg = nullptr;
    RecorderDirectoryCleaner *cleaner = nullptr;

    Private(RecorderExport *q_ptr)
        : q(q_ptr)
        , ui(new Ui::RecorderExport)
    {
    }

    void checkFfmpeg()
    {
        QProcess process;
        process.start(ffmpegPath, {"-version"});
        bool success = process.waitForStarted(1000) && process.waitForFinished(1000);
        if (!success)
            process.kill();
        const QByteArray &out = process.readAllStandardOutput();
        success &= out.contains("ffmpeg");

        const QIcon &icon = KisIconUtils::loadIcon(success ? "dialog-ok" : "window-close");
        const QList<QAction *> &actions = ui->editFfmpegPath->actions();
        QAction *action;
        if (!actions.isEmpty()) {
            action = actions.first();
            action->setIcon(icon);
        } else {
            action = ui->editFfmpegPath->addAction(icon, QLineEdit::TrailingPosition);
        }
        action->setToolTip(out);
        ui->editFfmpegPath->setText(ffmpegPath);
    }

    void fillComboProfiles()
    {
        QSignalBlocker blocker(ui->comboProfile);
        ui->comboProfile->clear();
        for (const RecorderProfile &profile : profiles) {
            ui->comboProfile->addItem(profile.name);
        }
        blocker.unblock();
        ui->comboProfile->setCurrentIndex(profileIndex);
    }

    void updateFramesCount()
    {
        QDir dir(settings.inputDirectory, "*.jpg", QDir::Name | QDir::Reversed, QDir::Files | QDir::NoDotAndDotDot);
        framesCount = dir.count();
        if (framesCount != 0) {
            imageSize = QImage(QDirIterator(dir).next()).size();
            imageSize.rwidth() &= ~1;
            imageSize.rheight() &= ~1;
        }
    }

    void updateVideoFilePath()
    {
        if (videoFileName.isEmpty())
            videoFileName = settings.name;
        if (videoDirectory.isEmpty())
            videoDirectory = RecorderExportConfig(true).videoDirectory();

        videoFilePath = videoDirectory % QDir::separator() % videoFileName % "." % profiles[profileIndex].extension;
        QSignalBlocker blocker(ui->editVideoFilePath);
        ui->editVideoFilePath->setText(videoFilePath);
    }

    void updateRatio(bool widthToHeight)
    {
        const float ratio = static_cast<float>(imageSize.width()) / static_cast<float>(imageSize.height());
        if (widthToHeight) {
            size.setHeight(static_cast<int>(size.width() / ratio));
        } else {
            size.setWidth(static_cast<int>(size.height() * ratio));
        }
        // make width and height even
        size.rwidth() &= ~1;
        size.rheight() &= ~1;
        QSignalBlocker blockerWidth(ui->spinScaleHeight);
        QSignalBlocker blockerHeight(ui->spinScaleWidth);
        ui->spinScaleHeight->setValue(size.height());
        ui->spinScaleWidth->setValue(size.width());
    }

    bool tryAbortExport()
    {
        if (ffmpeg == nullptr)
            return true;

        if (QMessageBox::question(q, q->windowTitle(), i18n("Abort encoding the timelapse video?"))
            == QMessageBox::Yes) {
            ffmpeg->kill();
            cleanupFFMpeg();
            return true;
        }

        return false;
    }

    void startExport()
    {
        Q_ASSERT(ffmpeg == nullptr);

        updateFramesCount();

        const QString &arguments = applyVariables(profiles[profileIndex].arguments);

        ffmpeg = new RecorderFFMpegWrapper(q);
        QObject::connect(ffmpeg, SIGNAL(started()), q, SLOT(onFFMpegStarted()));
        QObject::connect(ffmpeg, SIGNAL(finished()), q, SLOT(onFFMpegFinished()));
        QObject::connect(ffmpeg, SIGNAL(finishedWithError(QString)), q, SLOT(onFFMpegFinishedWithError(QString)));
        QObject::connect(ffmpeg, SIGNAL(progressUpdated(int)), q, SLOT(onFFMpegProgressUpdated(int)));
        ffmpeg->start({ffmpegPath, arguments, videoFilePath});
        ui->labelStatus->setText(i18nc("Status for the export of the video record", "Starting FFMpeg..."));
        ui->buttonCancelExport->setEnabled(false);
        ui->progressExport->setValue(0);
    }

    void cleanupFFMpeg()
    {
        if (ffmpeg != nullptr) {
            ffmpeg->deleteLater();
            ffmpeg = nullptr;
        }
    }

    QString applyVariables(const QString &templateArguments)
    {
        const QSize &outSize = resize ? size : imageSize;
        return QString(templateArguments)
               .replace("$IN_FPS", QString::number(inputFps))
               .replace("$OUT_FPS", QString::number(fps))
               .replace("$WIDTH", QString::number(outSize.width()))
               .replace("$HEIGHT", QString::number(outSize.height()))
               .replace("$FRAMES", QString::number(framesCount))
               .replace("$INPUT_DIR", settings.inputDirectory);
    }

    void updateVideoDuration()
    {
        long ms = framesCount * 1000L / inputFps;
        ui->labelVideoDuration->setText(formatDuration(ms));
    }

    QString formatDuration(long durationMs)
    {
        QString result;
        const long ms = (durationMs % 1000) / 100;
        if (ms != 0)
            result += "." % QString::number(ms);
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


RecorderExport::RecorderExport(QWidget *parent)
    : QDialog(parent)
    , d(new Private(this))
{
    d->ui->setupUi(this);
    d->ui->buttonBrowseDirectory->setIcon(KisIconUtils::loadIcon("view-preview"));
    d->ui->buttonBrowseFfmpeg->setIcon(KisIconUtils::loadIcon("folder"));
    d->ui->buttonEditProfile->setIcon(KisIconUtils::loadIcon("document-edit"));
    d->ui->buttonBrowseExport->setIcon(KisIconUtils::loadIcon("folder"));
    d->ui->buttonLockRatio->setIcon(KisIconUtils::loadIcon("locked"));
    d->ui->buttonWatchIt->setIcon(KisIconUtils::loadIcon("media-playback-start"));
    d->ui->buttonShowInFolder->setIcon(KisIconUtils::loadIcon("folder"));
    d->ui->buttonRemoveSnapshots->setIcon(KisIconUtils::loadIcon("trash-empty"));
    d->ui->stackedWidget->setCurrentIndex(ExportPageIndex::PageSettings);

    connect(d->ui->buttonBrowseDirectory, SIGNAL(clicked()), SLOT(onButtonBrowseDirectoryClicked()));
    connect(d->ui->spinInputFps, SIGNAL(valueChanged(int)), SLOT(onSpinInputFpsValueChanged(int)));
    connect(d->ui->spinFps, SIGNAL(valueChanged(int)), SLOT(onSpinFpsValueChanged(int)));
    connect(d->ui->checkResize, SIGNAL(toggled(bool)), SLOT(onCheckResizeToggled(bool)));
    connect(d->ui->spinScaleWidth, SIGNAL(valueChanged(int)), SLOT(onSpinScaleWidthValueChanged(int)));
    connect(d->ui->spinScaleHeight, SIGNAL(valueChanged(int)), SLOT(onSpinScaleHeightValueChanged(int)));
    connect(d->ui->buttonLockRatio, SIGNAL(toggled(bool)), SLOT(onButtonLockRatioToggled(bool)));
    connect(d->ui->buttonBrowseFfmpeg, SIGNAL(clicked()), SLOT(onButtonBrowseFfmpegClicked()));
    connect(d->ui->comboProfile, SIGNAL(currentIndexChanged(int)), SLOT(onComboProfileIndexChanged(int)));
    connect(d->ui->buttonEditProfile, SIGNAL(clicked()), SLOT(onButtonEditProfileClicked()));
    connect(d->ui->editVideoFilePath, SIGNAL(textChanged(QString)), SLOT(onEditVideoPathChanged(QString)));
    connect(d->ui->buttonBrowseExport, SIGNAL(clicked()), SLOT(onButtonBrowseExportClicked()));
    connect(d->ui->buttonExport, SIGNAL(clicked()), SLOT(onButtonExportClicked()));
    connect(d->ui->buttonCancelExport, SIGNAL(clicked()), SLOT(onButtonCancelClicked()));
    connect(d->ui->buttonWatchIt, SIGNAL(clicked()), SLOT(onButtonWatchItClicked()));
    connect(d->ui->buttonShowInFolder, SIGNAL(clicked()), SLOT(onButtonShowInFolderClicked()));
    connect(d->ui->buttonRemoveSnapshots, SIGNAL(clicked()), SLOT(onButtonRemoveSnapshotsClicked()));
    connect(d->ui->buttonRestart, SIGNAL(clicked()), SLOT(onButtonRestartClicked()));

    d->ui->editVideoFilePath->installEventFilter(this);
}

RecorderExport::~RecorderExport()
{
    delete d->ui;
}

void RecorderExport::setup(const RecorderExportSettings &settings)
{
    d->settings = settings;
    d->videoFileName = settings.name;

    d->updateFramesCount();

    if (d->framesCount == 0) {
        d->ui->labelRecordInfo->setText(i18nc("Can't export recording because nothing to export", "No frames to export"));
        d->ui->buttonExport->setEnabled(false);
    } else {
        d->ui->labelRecordInfo->setText(QString("%1: %2x%3 %4, %5 %6")
                                        .arg(i18nc("General information about recording", "Recording info"))
                                        .arg(d->imageSize.width())
                                        .arg(d->imageSize.height())
                                        .arg(i18nc("Pixel dimension suffix", "px"))
                                        .arg(d->framesCount)
                                        .arg(i18nc("The suffix after number of frames", "frame(s)"))
                                       );
    }

    RecorderExportConfig config(true);

    d->inputFps = config.inputFps();
    d->fps = config.fps();
    d->resize = config.resize();
    d->size = config.size();
    d->lockRatio = config.lockRatio();
    d->ffmpegPath = config.ffmpegPath();
    d->profiles = config.profiles();
    d->defaultProfiles = config.defaultProfiles();
    d->profileIndex = config.profileIndex();
    d->videoDirectory = config.videoDirectory();

    d->ui->spinInputFps->setValue(d->inputFps);
    d->ui->spinFps->setValue(d->fps);
    d->ui->checkResize->setChecked(d->resize);
    d->ui->spinScaleWidth->setValue(d->size.width());
    d->ui->spinScaleHeight->setValue(d->size.height());
    d->ui->buttonLockRatio->setChecked(d->lockRatio);
    d->ui->editFfmpegPath->setText(d->ffmpegPath);
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
    QDesktopServices::openUrl(QUrl::fromLocalFile(d->settings.inputDirectory));
}

void RecorderExport::onSpinInputFpsValueChanged(int value)
{
    d->inputFps = value;
    RecorderExportConfig(false).setInputFps(value);
    d->updateVideoDuration();
}

void RecorderExport::onSpinFpsValueChanged(int value)
{
    d->fps = value;
    RecorderExportConfig(false).setFps(value);
}

void RecorderExport::onCheckResizeToggled(bool checked)
{
    d->resize = checked;
    RecorderExportConfig(false).setResize(checked);
}

void RecorderExport::onSpinScaleWidthValueChanged(int value)
{
    d->size.setWidth(value);
    if (d->lockRatio)
        d->updateRatio(true);
    RecorderExportConfig(false).setSize(d->size);
}

void RecorderExport::onSpinScaleHeightValueChanged(int value)
{
    d->size.setHeight(value);
    if (d->lockRatio)
        d->updateRatio(false);
    RecorderExportConfig(false).setSize(d->size);
}

void RecorderExport::onButtonLockRatioToggled(bool checked)
{
    d->lockRatio = checked;
    RecorderExportConfig config(false);
    config.setLockRatio(checked);
    if (d->lockRatio) {
        d->updateRatio(true);
        config.setSize(d->size);
    }
}

void RecorderExport::onButtonBrowseFfmpegClicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setFilter(QDir::Executable | QDir::Files);

    const QString &file = dialog.getOpenFileName(this,
                          i18n("Select FFMpeg Executable File"),
                          d->ffmpegPath);
    if (!file.isEmpty()) {
        d->ffmpegPath = file;
        RecorderExportConfig(false).setFfmpegPath(file);
        d->checkFfmpeg();
    }
}

void RecorderExport::onComboProfileIndexChanged(int index)
{
    d->profileIndex = index;
    d->updateVideoFilePath();
    RecorderExportConfig(false).setProfileIndex(index);
}

void RecorderExport::onButtonEditProfileClicked()
{
    RecorderProfileSettings settingsDialog(this);

    connect(&settingsDialog, &RecorderProfileSettings::requestPreview, [&](const QString & arguments) {
        settingsDialog.setPreview(d->ffmpegPath % " -y " % d->applyVariables(arguments).replace("\n", " ")
                                  % " \"" % d->videoFilePath % "\"");
    });

    if (settingsDialog.editProfile(&d->profiles[d->profileIndex], d->defaultProfiles[d->profileIndex])) {
        d->fillComboProfiles();
        d->updateVideoFilePath();
        RecorderExportConfig(false).setProfiles(d->profiles);
    }
}

void RecorderExport::onEditVideoPathChanged(const QString &videoFilePath)
{
    QFileInfo fileInfo(videoFilePath);
    if (!fileInfo.isRelative())
        d->videoDirectory = fileInfo.absolutePath();
    d->videoFileName = fileInfo.completeBaseName();
}

void RecorderExport::onButtonBrowseExportClicked()
{
    QFileDialog dialog(this);

    const QString &extension = d->profiles[d->profileIndex].extension;
    const QString &videoFileName = dialog.getSaveFileName(this,
                                   i18n("Export Timelapse Video As"),
                                   d->videoDirectory, "*." % extension);
    if (!videoFileName.isEmpty()) {
        QFileInfo fileInfo(videoFileName);
        d->videoDirectory = fileInfo.absolutePath();
        d->videoFileName = fileInfo.completeBaseName();
        d->updateVideoFilePath();
        RecorderExportConfig(false).setVideoDirectory(d->videoDirectory);
    }
}

void RecorderExport::onButtonExportClicked()
{
    if (QFile::exists(d->videoFilePath)) {
        if (QMessageBox::question(this, windowTitle(),
                                  i18n("The video file is already exists. Do you wish to overwrite?"))
            != QMessageBox::Yes) {
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
    d->ui->stackedWidget->setCurrentIndex(ExportPageIndex::PageDone);
    d->ui->labelVideoPathDone->setText(d->videoFilePath);
    d->cleanupFFMpeg();
}

void RecorderExport::onFFMpegFinishedWithError(QString error)
{
    d->ui->stackedWidget->setCurrentIndex(ExportPageIndex::PageSettings);
    QMessageBox::critical(this, windowTitle(), i18n("Export failed. FFMpeg message:") % "\n\n" % error);
    d->cleanupFFMpeg();
}

void RecorderExport::onFFMpegProgressUpdated(int frameNo)
{
    d->ui->progressExport->setValue(frameNo * 100 / (d->framesCount * d->fps / static_cast<float>(d->inputFps)));
}

void RecorderExport::onButtonWatchItClicked()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(d->videoFilePath));
}

void RecorderExport::onButtonShowInFolderClicked()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(d->videoDirectory));
}

void RecorderExport::onButtonRemoveSnapshotsClicked()
{
    const QString confirmation(i18n("The recordings for this document will be deleted"
                                    " and you will not be able to export a timelapse for it again"
                                    " (the already exported timelapses will be preserved though)."
                                    "\nDo you wish to continue?"));
    if (QMessageBox::question(this, windowTitle(), confirmation) != QMessageBox::Yes)
        return;

    d->ui->labelStatus->setText(i18nc("Label title, Snapshot directory deleting is in progress", "Cleaning up..."));
    d->ui->stackedWidget->setCurrentIndex(ExportPageIndex::PageProgress);

    Q_ASSERT(d->cleaner == nullptr);
    d->cleaner = new RecorderDirectoryCleaner({d->settings.inputDirectory});
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
