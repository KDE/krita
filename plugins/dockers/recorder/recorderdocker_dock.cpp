/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include <klocalizedstring.h>
#include <kis_canvas2.h>
#include <kis_icon_utils.h>
#include <kis_statusbar.h>
#include <KisDocument.h>
#include <KisViewManager.h>

#include <QFileInfo>
#include <QPointer>
#include <QFileDialog>


class RecorderDockerDock::Private {
public:
    RecorderDockerDock *const q;
    Ui::RecorderDocker *const ui;
    QPointer<KisCanvas2> canvas;
    RecorderWriter writer;
    QString defaultPrefix;
    QLabel* statusBarLabel;

    Private(RecorderDockerDock *q_ptr)
        : q(q_ptr)
        , ui(new Ui::RecorderDocker())
        , statusBarLabel(new QLabel())
    {
        updateRecIndicator(false);
    }

    void loadSettings()
    {
        RecorderConfig config(true);
        ui->editDirectory->setText(config.snapshotDirectory());
        defaultPrefix = config.defaultPrefix();
        ui->checkBoxUseDocName->setChecked(config.useDocumentName());
        ui->spinCaptureInterval->setValue(config.captureInterval());
        ui->spinQuality->setValue(config.quality());
        ui->comboResolution->setCurrentIndex(config.resolution());
        ui->checkBoxAutoRecord->setChecked(config.recordAutomatically());
    }

    void updatePrefix()
    {
        bool useDocumentPrefix = ui->checkBoxUseDocName->isChecked();

        const QString &prefix = (useDocumentPrefix && canvas)
            ? canvas->imageView()->document()->uniqueID()
            : defaultPrefix;

        ui->editPrefix->setText(prefix);
        ui->editPrefix->setEnabled(!useDocumentPrefix);
    }

    void updateComboResolution(quint32 width, quint32 height)
    {
        const QList<QPair<QString, int>> resolutions = {
            { i18n("Original"), 1 },
            { i18n("Half"), 2 }
        };

        QStringList items;
        for (const QPair<QString, int> &item : resolutions) {
            items += item.first % QString(" (%1x%2)").arg(width / item.second).arg(height / item.second);
        }
        QSignalBlocker blocker(ui->comboResolution);
        const int currentIndex = ui->comboResolution->currentIndex();
        ui->comboResolution->clear();
        ui->comboResolution->addItems(items);
        ui->comboResolution->setCurrentIndex(currentIndex);
    }

    void updateRecordStatus(bool isRecording)
    {
        QSignalBlocker blocker(ui->buttonRecordToggle);
        ui->buttonRecordToggle->setChecked(isRecording);
        ui->buttonRecordToggle->setIcon(KisIconUtils::loadIcon(isRecording ? "media-playback-stop" : "media-record"));
        ui->buttonRecordToggle->setText(i18n(isRecording ? "Stop" : "Record"));
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
        statusBarLabel->setText(paused ? "<font>● REC</font>" : "<font color='#da4453'>●</font><font> REC</font>");
        statusBarLabel->setToolTip(i18n(paused ? "Recorder is paused" : "Recorder is active"));
    }
};

RecorderDockerDock::RecorderDockerDock()
    : QDockWidget(i18n("Recorder"))
    , d(new Private(this))
{
    QWidget* page = new QWidget(this);
    d->ui->setupUi(page);

    QValidator* validator = new QRegExpValidator(QRegExp("[0-9a-zA-z_]+"), this);
    d->ui->editPrefix->setValidator(validator);
    d->ui->buttonBrowse->setIcon(KisIconUtils::loadIcon("folder"));
    d->ui->buttonRecordToggle->setIcon(KisIconUtils::loadIcon("media-record"));
    d->ui->spinQuality->setSuffix("%");

    d->loadSettings();

    connect(d->ui->buttonBrowse, SIGNAL(clicked()), this, SLOT(onSelectRecordFolderButtonClicked()));
    connect(d->ui->checkBoxUseDocName, SIGNAL(toggled(bool)), this, SLOT(onUseDocNameToggled(bool)));
    connect(d->ui->editPrefix, SIGNAL(editingFinished()), this, SLOT(onEditPrefixChanged()));
    connect(d->ui->spinCaptureInterval, SIGNAL(valueChanged(int)), this, SLOT(onCaptureIntervalChanged(int)));
    connect(d->ui->spinQuality, SIGNAL(valueChanged(int)), this, SLOT(onQualityChanged(int)));
    connect(d->ui->comboResolution, SIGNAL(currentIndexChanged(int)), this, SLOT(onResolutionChanged(int)));
    connect(d->ui->checkBoxAutoRecord, SIGNAL(toggled(bool)), this, SLOT(onAutoRecordToggled(bool)));
    connect(d->ui->buttonRecordToggle, SIGNAL(toggled(bool)), this, SLOT(onRecordButtonToggled(bool)));

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

    if (d->canvas) {
        KisDocument *document = d->canvas->imageView()->document();
        d->updatePrefix();
        if (d->ui->checkBoxAutoRecord->isChecked()) {
            d->ui->buttonRecordToggle->setChecked(true);
        }
        d->updateComboResolution(document->image()->width(), document->image()->height());

qDebug() << "RecorderDockerDock setCanvas " << document->caption();
        //    TODO: void titleModified(const QString &caption, bool isModified);
    }
}

void RecorderDockerDock::unsetCanvas()
{
    d->updateRecordStatus(false);
    d->updatePrefix();
    setEnabled(false);
    d->writer.stop();
    d->writer.setCanvas(nullptr);
    d->canvas = nullptr;
}

void RecorderDockerDock::onRecordButtonToggled(bool checked)
{
    d->ui->buttonRecordToggle->setEnabled(false);

    if (checked) {
        d->writer.start();
    } else {
        d->writer.stop();
    }
}

void RecorderDockerDock::onSelectRecordFolderButtonClicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    const QString &directory = dialog.getExistingDirectory(this, i18n("Select Output Folder"),
                                                 d->ui->editDirectory->text(),
                                                 QFileDialog::ShowDirsOnly);
    if (!directory.isEmpty()) {
        d->ui->editDirectory->setText(directory);
        RecorderConfig(false).setSnapshotDirectory(directory);
    }
}

void RecorderDockerDock::onUseDocNameToggled(bool checked)
{
    d->updatePrefix();
    RecorderConfig(false).setUseDocumentName(checked);
}

void RecorderDockerDock::onAutoRecordToggled(bool checked)
{
    RecorderConfig(false).setRecordAutomatically(checked);
}

void RecorderDockerDock::onEditPrefixChanged()
{
    RecorderConfig(false).setDefaultPrefix(d->ui->editPrefix->text());
}

void RecorderDockerDock::onCaptureIntervalChanged(int interval)
{
    RecorderConfig(false).setCaptureInterval(interval);
}

void RecorderDockerDock::onQualityChanged(int quality)
{
    RecorderConfig(false).setQuality(quality);
}

void RecorderDockerDock::onResolutionChanged(int resolution)
{
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
