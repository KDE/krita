/*
 *  SPDX-FileCopyrightText: 2019 Shi Yan <billconan@gmail.net>
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "recorderdocker_dock.h"
#include "recorder_config.h"
#include "recorder_writer.h"
#include "ui_recorderdocker.h"
#include "recorder_snapshots_manager.h"
#include "recorder_export.h"

#include <klocalizedstring.h>
#include <kis_action_registry.h>
#include <kis_canvas2.h>
#include <kis_icon_utils.h>
#include <kis_statusbar.h>
#include <KisDocument.h>
#include <KisViewManager.h>
#include <KoDocumentInfo.h>
#include <kactioncollection.h>
#include <KisPart.h>

#include <QFileInfo>
#include <QPointer>
#include <QFileDialog>

namespace
{
const QString keyActionRecordToggle = "recorder_record_toggle";
const QString keyActionExport = "recorder_export";
}


class RecorderDockerDock::Private
{
public:
    RecorderDockerDock *const q;
    Ui::RecorderDocker *const ui;
    QPointer<KisCanvas2> canvas;
    RecorderWriter writer;

    QAction *recordToggleAction = nullptr;
    QAction *exportAction = nullptr;

    QString snapshotDirectory;
    QString prefix;
    QString outputDirectory;
    int captureInterval = 0;
    int quality = 0;
    int resolution = 0;
    bool recordIsolateLayerMode = false;
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
        recordIsolateLayerMode = config.recordIsolateLayerMode();
        recordAutomatically = config.recordAutomatically();
    }


    void updateWriterSettings()
    {
        outputDirectory = snapshotDirectory % QDir::separator() % prefix % QDir::separator();
        writer.setup({ outputDirectory, quality, resolution, captureInterval, recordIsolateLayerMode });
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
            items += QString("%1 (%2x%3)").arg(titles[index])
                    .arg((width / divider) & ~1)
                    .arg((height / divider) & ~1);
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
        recordToggleAction->setChecked(isRecording);
        recordToggleAction->setEnabled(isColorSpaceSupported);

        QSignalBlocker blocker(ui->buttonRecordToggle);
        ui->buttonRecordToggle->setChecked(isRecording);
        ui->buttonRecordToggle->setIcon(KisIconUtils::loadIcon(isRecording ? "media-playback-stop" : "media-record"));
        ui->buttonRecordToggle->setText(isRecording ? i18nc("Stop recording the canvas", "Stop")
                                        : i18nc("Start recording the canvas", "Record"));
        ui->buttonRecordToggle->setEnabled(isColorSpaceSupported);

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

    d->ui->buttonManageRecordings->setIcon(KisIconUtils::loadIcon("configure-thicker"));
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
    d->ui->checkBoxRecordIsolateMode->setChecked(d->recordIsolateLayerMode);
    d->ui->checkBoxAutoRecord->setChecked(d->recordAutomatically);

    KisActionRegistry *actionRegistry = KisActionRegistry::instance();
    d->recordToggleAction = actionRegistry->makeQAction(keyActionRecordToggle, this);
    d->exportAction = actionRegistry->makeQAction(keyActionExport, this);

    connect(d->recordToggleAction, SIGNAL(toggled(bool)), d->ui->buttonRecordToggle, SLOT(setChecked(bool)));
    connect(d->exportAction, SIGNAL(triggered()), d->ui->buttonExport, SIGNAL(clicked()));

    // Need to register toolbar actions before attaching canvas else it wont appear after restart.
    // Is there any better way to do this?
    connect(KisPart::instance(), SIGNAL(sigMainWindowIsBeingCreated(KisMainWindow *)),
            this, SLOT(onMainWindowIsBeingCreated(KisMainWindow *)));

    connect(d->ui->buttonManageRecordings, SIGNAL(clicked()), this, SLOT(onManageRecordingsButtonClicked()));
    connect(d->ui->buttonBrowse, SIGNAL(clicked()), this, SLOT(onSelectRecordFolderButtonClicked()));
    connect(d->ui->spinCaptureInterval, SIGNAL(valueChanged(int)), this, SLOT(onCaptureIntervalChanged(int)));
    connect(d->ui->spinQuality, SIGNAL(valueChanged(int)), this, SLOT(onQualityChanged(int)));
    connect(d->ui->comboResolution, SIGNAL(currentIndexChanged(int)), this, SLOT(onResolutionChanged(int)));
    connect(d->ui->checkBoxRecordIsolateMode, SIGNAL(toggled(bool)), this, SLOT(onRecordIsolateLayerModeToggled(bool)));
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
    d->writer.setEnabled(enabled && d->isColorSpaceSupported);
    d->updateRecordStatus(enabled && d->isColorSpaceSupported);
}

void RecorderDockerDock::unsetCanvas()
{
    d->updateRecordStatus(false);
    d->recordToggleAction->setChecked(false);
    setEnabled(false);
    d->writer.stop();
    d->writer.setCanvas(nullptr);
    d->canvas = nullptr;
    d->enabledIds.clear();
}

void RecorderDockerDock::onMainWindowIsBeingCreated(KisMainWindow *window)
{
    KActionCollection *actionCollection = window->viewManager()->actionCollection();
    actionCollection->addAction(keyActionRecordToggle, d->recordToggleAction);
    actionCollection->addAction(keyActionExport, d->exportAction);
}

void RecorderDockerDock::onRecordButtonToggled(bool checked)
{
    d->recordToggleAction->setChecked(checked);

    if (!d->canvas)
        return;

    const QString &id = d->canvas->imageView()->document()->uniqueID();

    bool wasEmpty = !d->enabledIds.values().contains(true);

    d->enabledIds[id] = checked;

    bool isEmpty = !d->enabledIds.values().contains(true);

    d->writer.setEnabled(checked);

    if (isEmpty == wasEmpty) {
        d->updateRecordStatus(checked);
        return;
    }


    d->ui->buttonRecordToggle->setEnabled(false);

    if (checked) {
        d->updateWriterSettings();
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

void RecorderDockerDock::onManageRecordingsButtonClicked()
{
    RecorderSnapshotsManager snapshotsManager(this);
    snapshotsManager.execFor(d->snapshotDirectory);
}


void RecorderDockerDock::onSelectRecordFolderButtonClicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    const QString &directory = dialog.getExistingDirectory(this,
                               i18n("Select a Directory for Recordings"),
                               d->ui->editDirectory->text(),
                               QFileDialog::ShowDirsOnly);
    if (!directory.isEmpty()) {
        d->ui->editDirectory->setText(directory);
        RecorderConfig(false).setSnapshotDirectory(directory);
    }
}

void RecorderDockerDock::onRecordIsolateLayerModeToggled(bool checked)
{
    d->recordIsolateLayerMode = checked;
    RecorderConfig(false).setRecordIsolateLayerMode(checked);
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
