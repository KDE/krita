/*
 *  Copyright (c) 2019 Shi Yan <billconan@gmail.net>
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

#include "recorderdocker_dock.h"
#include "recorder_config.h"
#include "recorder_writer.h"
#include "ui_recorderdocker.h"
#include "recorder_export.h"

#include <klocalizedstring.h>
#include <kis_canvas2.h>
#include <kis_icon_utils.h>
#include <kis_statusbar.h>
#include <KisDocument.h>
#include <KisViewManager.h>
#include <KoDocumentInfo.h>

#include <QFileInfo>
#include <QPointer>
#include <QFileDialog>


class RecorderDockerDock::Private
{
public:
    RecorderDockerDock *const q;
    Ui::RecorderDocker *const ui;
    QPointer<KisCanvas2> canvas;
    RecorderWriter writer;

    QString snapshotDirectory;
    QString prefix;
    QString outputDirectory;
    int captureInterval = 0;
    int quality = 0;
    int resolution = 0;
    bool recordAutomatically = false;

    QLabel* statusBarLabel;
    bool isColorSpaceSupported;

    QMap<QString, bool> enabledIds;

    Private(RecorderDockerDock *q_ptr)
        : q(q_ptr)
        , ui(new Ui::RecorderDocker())
        , statusBarLabel(new QLabel())
        , isColorSpaceSupported(false)
    {
        updateRecIndicator(false);
    }

    void loadSettings()
    {
        RecorderConfig config(true);
        snapshotDirectory = config.snapshotDirectory();
        captureInterval = config.captureInterval();
        quality = config.quality();
        resolution = config.resolution();
        recordAutomatically = config.recordAutomatically();
    }


    void updateWriterSettings()
    {
        outputDirectory = snapshotDirectory % QDir::separator() % prefix % QDir::separator();
        writer.setup({ outputDirectory, quality, resolution, captureInterval });
    }

    QString getPrefix()
    {
        return !canvas ? ""
               : canvas->imageView()->document()->documentInfo()->aboutInfo("creation-date").remove(QRegExp("[^0-9]"));
    }

    void updateComboResolution(quint32 width, quint32 height)
    {
        const QStringList titles = {
            i18nc("Use original resolution for the frames when recording the canvas", "Original"),
            i18nc("Use the resolution two times smaller than the original resolution for the frames when recording the canvas", "Half"),
            i18nc("Use the resolution four times smaller than the original resolution for the frames when recording the canvas", "Quarter")
        };

        QStringList items;
        for (int index = 0, len = titles.length(); index < len; ++index) {
            int divider = 1 << index;
            items += QString("%1 (%2x%3)").arg(titles[index]).arg(width / divider).arg(height / divider);
        }
        QSignalBlocker blocker(ui->comboResolution);
        const int currentIndex = ui->comboResolution->currentIndex();
        ui->comboResolution->clear();
        ui->comboResolution->addItems(items);
        ui->comboResolution->setCurrentIndex(currentIndex);
    }

    void validateColorSpace(const KoColorSpace *colorSpace)
    {
        isColorSpaceSupported = colorSpace->colorModelId().id() == "RGBA" &&
                                colorSpace->colorDepthId().id() == "U8";
        ui->labelUnsupportedColorSpace->setVisible(!isColorSpaceSupported);
        ui->buttonRecordToggle->setEnabled(isColorSpaceSupported);
    }

    void updateRecordStatus(bool isRecording)
    {
        QSignalBlocker blocker(ui->buttonRecordToggle);
        ui->buttonRecordToggle->setChecked(isRecording);
        ui->buttonRecordToggle->setIcon(KisIconUtils::loadIcon(isRecording ? "media-playback-stop" : "media-record"));
        ui->buttonRecordToggle->setText(isRecording ? i18nc("Stop recording the canvas", "Stop")
                                        : i18nc("Start recording the canvas", "Record"));
        ui->buttonRecordToggle->setEnabled(true);

        ui->widgetSettings->setEnabled(!isRecording);

        statusBarLabel->setVisible(isRecording);

        if (!canvas)
            return;

        KisStatusBar *statusBar = canvas->viewManager()->statusBar();
        if (isRecording) {
            statusBar->addExtraWidget(statusBarLabel);
        } else {
            statusBar->removeExtraWidget(statusBarLabel);
        }
    }

    void updateRecIndicator(bool paused)
    {
        // don't remove empty <font></font> tag else label will jump a few pixels around
        statusBarLabel->setText(QString("<font%1>●</font><font> %2</font>")
                                .arg(paused ? "" : " color='#da4453'").arg(i18nc("Recording symbol", "REC")));
        statusBarLabel->setToolTip(paused ? i18n("Recorder is paused") : i18n("Recorder is active"));
    }
};

RecorderDockerDock::RecorderDockerDock()
    : QDockWidget(i18nc("Title of the docker", "Recorder"))
    , d(new Private(this))
{
    QWidget* page = new QWidget(this);
    d->ui->setupUi(page);
    d->ui->labelUnsupportedColorSpace->setVisible(false);

    d->ui->buttonBrowse->setIcon(KisIconUtils::loadIcon("folder"));
    d->ui->buttonRecordToggle->setIcon(KisIconUtils::loadIcon("media-record"));
    d->ui->buttonExport->setIcon(KisIconUtils::loadIcon("document-export"));
    d->ui->spinQuality->setMinimum(1);
    d->ui->spinQuality->setMaximum(100);
    d->ui->spinQuality->setSuffix("%");

    d->loadSettings();

    d->ui->editDirectory->setText(d->snapshotDirectory);
    d->ui->spinCaptureInterval->setValue(d->captureInterval);
    d->ui->spinQuality->setValue(d->quality);
    d->ui->comboResolution->setCurrentIndex(d->resolution);
    d->ui->checkBoxAutoRecord->setChecked(d->recordAutomatically);

    connect(d->ui->buttonBrowse, SIGNAL(clicked()), this, SLOT(onSelectRecordFolderButtonClicked()));
    connect(d->ui->spinCaptureInterval, SIGNAL(valueChanged(int)), this, SLOT(onCaptureIntervalChanged(int)));
    connect(d->ui->spinQuality, SIGNAL(valueChanged(int)), this, SLOT(onQualityChanged(int)));
    connect(d->ui->comboResolution, SIGNAL(currentIndexChanged(int)), this, SLOT(onResolutionChanged(int)));
    connect(d->ui->checkBoxAutoRecord, SIGNAL(toggled(bool)), this, SLOT(onAutoRecordToggled(bool)));
    connect(d->ui->buttonRecordToggle, SIGNAL(toggled(bool)), this, SLOT(onRecordButtonToggled(bool)));
    connect(d->ui->buttonExport, SIGNAL(clicked()), this, SLOT(onExportButtonClicked()));

    connect(&d->writer, SIGNAL(started()), this, SLOT(onWriterStarted()));
    connect(&d->writer, SIGNAL(finished()), this, SLOT(onWriterFinished()));
    connect(&d->writer, SIGNAL(pausedChanged(bool)), this, SLOT(onWriterPausedChanged(bool)));

    setWidget(page);
}

RecorderDockerDock::~RecorderDockerDock()
{
    delete d;
}

void RecorderDockerDock::setCanvas(KoCanvasBase* canvas)
{
    setEnabled(canvas != nullptr);

    if (d->canvas == canvas)
        return;

    d->canvas = dynamic_cast<KisCanvas2*>(canvas);
    d->writer.setCanvas(d->canvas);

    if (!d->canvas)
        return;

    KisDocument *document = d->canvas->imageView()->document();
    if (d->recordAutomatically && !d->enabledIds.contains(document->uniqueID()))
        onRecordButtonToggled(true);

    d->updateComboResolution(document->image()->width(), document->image()->height());
    d->validateColorSpace(document->image()->projection()->colorSpace());

    d->prefix = d->getPrefix();
    d->updateWriterSettings();

    bool enabled = d->enabledIds.value(document->uniqueID(), false);
    d->writer.setEnabled(enabled);
    d->updateRecordStatus(enabled);
}

void RecorderDockerDock::unsetCanvas()
{
    d->updateRecordStatus(false);
    setEnabled(false);
    d->writer.stop();
    d->writer.setCanvas(nullptr);
    d->canvas = nullptr;
    d->enabledIds.clear();
}

void RecorderDockerDock::onRecordButtonToggled(bool checked)
{
    if (!d->canvas)
        return;

    const QString &id = d->canvas->imageView()->document()->uniqueID();

    bool wasEmpty = d->enabledIds.isEmpty();

    d->enabledIds[id] = checked;

    d->writer.setEnabled(checked);

    if (d->enabledIds.isEmpty() == wasEmpty) {
        d->updateRecordStatus(checked);
        return;
    }


    d->ui->buttonRecordToggle->setEnabled(false);

    if (checked) {
        d->writer.start();
    } else {
        d->writer.stop();
    }
}

void RecorderDockerDock::onExportButtonClicked()
{
    if (!d->canvas)
        return;

    KisDocument *document = d->canvas->imageView()->document();

    RecorderExport exportDialog(this);
    exportDialog.setup({
        QFileInfo(document->caption().trimmed()).completeBaseName(),
        d->outputDirectory
    });
    exportDialog.exec();
}

void RecorderDockerDock::onSelectRecordFolderButtonClicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    const QString &directory = dialog.getExistingDirectory(this,
                               i18n("Select a Folder for Recordings"),
                               d->ui->editDirectory->text(),
                               QFileDialog::ShowDirsOnly);
    if (!directory.isEmpty()) {
        d->ui->editDirectory->setText(directory);
        RecorderConfig(false).setSnapshotDirectory(directory);
    }
}

void RecorderDockerDock::onAutoRecordToggled(bool checked)
{
    d->recordAutomatically = checked;
    RecorderConfig(false).setRecordAutomatically(checked);
}

void RecorderDockerDock::onCaptureIntervalChanged(int interval)
{
    d->captureInterval = interval;
    RecorderConfig(false).setCaptureInterval(interval);
}

void RecorderDockerDock::onQualityChanged(int quality)
{
    d->quality = quality;
    RecorderConfig(false).setQuality(quality);
}

void RecorderDockerDock::onResolutionChanged(int resolution)
{
    d->resolution = resolution;
    RecorderConfig(false).setResolution(resolution);
}

void RecorderDockerDock::onWriterStarted()
{
    d->updateRecordStatus(true);
}

void RecorderDockerDock::onWriterFinished()
{
    d->updateRecordStatus(false);
}

void RecorderDockerDock::onWriterPausedChanged(bool paused)
{
    d->updateRecIndicator(paused);
}
