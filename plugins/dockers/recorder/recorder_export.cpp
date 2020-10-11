/*
 *  Copyright (c) 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "recorder_export.h"
#include "recorder_export_config.h"
#include "recorder_profile_settings.h"
#include "ui_recorder_export.h"

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

class RecorderExport::Private
{
public:
    RecorderExport *q;
    Ui::RecorderExport *ui;
    RecorderExportSettings settings;
    QSize imageSize;

    int fps = 30;
    bool resize = false;
    QSize size;
    bool lockRatio = false;
    QString ffmpegPath;
    QList<RecorderProfile> profiles;
    QList<RecorderProfile> defaultProfiles;
    int profileIndex = 0;
    QString videoDirectory;

    QProcess *ffmpegProcess = nullptr;

    Private(RecorderExport *q_ptr)
        : q(q_ptr)
        , ui(new Ui::RecorderExport)
    {
    }

    void checkFfmpeg()
    {
        QProcess process;
        process.start(ffmpegPath, {"-version"});
        bool success = process.waitForFinished();
        const QByteArray &out = process.readAllStandardOutput();

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

    void updateVideoFilePath()
    {
        ui->editVideoFilePath->setText(videoDirectory % QDir::separator()
                                       % settings.name % "." % profiles[profileIndex].extension);
    }

    void updateRatio(bool widthToHeight)
    {
        const float ratio = static_cast<float>(imageSize.width()) / static_cast<float>(imageSize.height());
        if (widthToHeight) {
            QSignalBlocker blocker(ui->spinScaleHeight);
            size.setHeight(static_cast<int>(size.width() / ratio));
            ui->spinScaleHeight->setValue(size.height());
        } else {
            QSignalBlocker blocker(ui->spinScaleWidth);
            size.setWidth(static_cast<int>(size.height() * ratio));
            ui->spinScaleWidth->setValue(size.width());
        }
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

    connect(d->ui->buttonBrowseDirectory, SIGNAL(clicked()), SLOT(onButtonBrowseDirectoryClicked()));
    connect(d->ui->spinFps, SIGNAL(valueChanged(int)), SLOT(onSpinFpsValueChanged(int)));
    connect(d->ui->checkResize, SIGNAL(toggled(bool)), SLOT(onCheckResizeToggled(bool)));
    connect(d->ui->spinScaleWidth, SIGNAL(valueChanged(int)), SLOT(onSpinScaleWidthValueChanged(int)));
    connect(d->ui->spinScaleHeight, SIGNAL(valueChanged(int)), SLOT(onSpinScaleHeightValueChanged(int)));
    connect(d->ui->buttonLockRatio, SIGNAL(toggled(bool)), SLOT(onButtonLockRatioToggled(bool)));
    connect(d->ui->buttonBrowseFfmpeg, SIGNAL(clicked()), SLOT(onButtonBrowseFfmpegClicked()));
    connect(d->ui->comboProfile, SIGNAL(currentIndexChanged(int)), SLOT(onComboProfileIndexChanged(int)));
    connect(d->ui->buttonEditProfile, SIGNAL(clicked()), SLOT(onButtonEditProfileClicked()));
    connect(d->ui->buttonBrowseExport, SIGNAL(clicked()), SLOT(onButtonBrowseExportClicked()));
    connect(d->ui->buttonExport, SIGNAL(clicked()), SLOT(onButtonExportClicked()));
}

RecorderExport::~RecorderExport()
{
    delete d->ui;
}

void RecorderExport::setup(const RecorderExportSettings &settings)
{
    d->settings = settings;

    QDir dir(settings.inputDirectory, "*.jpg", QDir::Name | QDir::Reversed, QDir::Files | QDir::NoDotAndDotDot);

    if (dir.isEmpty()) {
        d->ui->labelRecordInfo->setText(i18n("No files to export"));
        d->ui->buttonExport->setEnabled(false);
        return;
    }

    d->imageSize = QImage(QDirIterator(dir).next()).size();

    d->ui->labelRecordInfo->setText(QString("%1: %2x%3 %4, %5 %6")
                                    .arg(i18nc("General information about recording", "Recording info"))
                                    .arg(d->imageSize.width())
                                    .arg(d->imageSize.height())
                                    .arg(i18nc("Pixel dimension suffix", "px"))
                                    .arg(dir.count())
                                    .arg(i18nc("The suffix after number of frames", "frame(s)"))
                                   );

    RecorderExportConfig config(true);

    d->fps = config.fps();
    d->resize = config.resize();
    d->size = config.size();
    d->lockRatio = config.lockRatio();
    d->ffmpegPath = config.ffmpegPath();
    d->profiles = config.profiles();
    d->defaultProfiles = config.defaultProfiles();
    d->profileIndex = config.profileIndex();
    d->videoDirectory = config.videoDirectory();

    d->ui->spinFps->setValue(d->fps);
    d->ui->checkResize->setChecked(d->resize);
    d->ui->spinScaleWidth->setValue(d->size.width());
    d->ui->spinScaleHeight->setValue(d->size.height());
    d->ui->buttonLockRatio->setChecked(d->lockRatio);
    d->ui->editFfmpegPath->setText(d->ffmpegPath);
    d->fillComboProfiles();
    d->checkFfmpeg();
    d->updateVideoFilePath();
}

void RecorderExport::onButtonBrowseDirectoryClicked()
{
    QDesktopServices::openUrl(QUrl(d->settings.inputDirectory));
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
    if (settingsDialog.editProfile(&d->profiles[d->profileIndex], d->defaultProfiles[d->profileIndex])) {
        d->fillComboProfiles();
        d->updateVideoFilePath();
        RecorderExportConfig(false).setProfiles(d->profiles);
    }
}

void RecorderExport::onButtonBrowseExportClicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    const QString &directory = dialog.getExistingDirectory(this,
                               i18n("Select a Folder for Video"),
                               d->videoDirectory,
                               QFileDialog::ShowDirsOnly);
    if (!directory.isEmpty()) {
        d->videoDirectory = directory;
        d->updateVideoFilePath();
        RecorderExportConfig(false).setVideoDirectory(directory);
    }
}

void RecorderExport::onButtonExportClicked()
{
    // TODO
}
