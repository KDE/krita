/*
 *  SPDX-FileCopyrightText: 2019 Shi Yan <billconan@gmail.net>
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "recorderdocker_dock.h"
#include "recorder_config.h"
#include "recorder_writer.h"
#include "recorder_const.h"
#include "ui_recorderdocker.h"
#include "recorder_snapshots_manager.h"
#include "recorder_export.h"
#include "recorder_export_settings.h"
#include "recorder_export_config.h"

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
#include <KisKineticScroller.h>
#include "KisMainWindow.h"
#include "KoFileDialog.h"

#include <QFileInfo>
#include <QPointer>
#include <QMessageBox>
#include <QTimer>
#include <QRegularExpression>

#ifdef Q_OS_ANDROID
#include <QAtomicInt>
#include <QDir>
#include <QFile>
#include <QRunnable>
#include <QThreadPool>
#endif

namespace
{
const QString keyActionRecordToggle = "recorder_record_toggle";
const QString keyActionExport = "recorder_export";

const QString activeColorGreen(" color='#5cab25'");
const QString inactiveColorGreen(" color='#b4e196'");
const QString activeColorOrange(" color='#ca8f14'");
const QString inactiveColorOrange(" color='#ffe5af'");
const QString activeColorRed(" color='#da4453'");
const QString inactiveColorRed(" color='#f2c4c9'");
const QString inactiveColorGray(" color='#3e3e3e'");

const QColor textColorOrange(0xff, 0xe5, 0xaf);
const QColor buttonColorOrange(0xca, 0x8f, 0x14);
const QColor textColorRed(0xf2, 0xc4, 0xc9);
const QColor buttonColorRed(0xda, 0x44, 0x53);

}


class RecorderDockerDock::Private
{
public:
    RecorderDockerDock *const q;
    QScopedPointer<Ui::RecorderDocker> ui;
    QPalette threadsSliderPalette;
    QPalette threadsSpinPalette;
    QPointer<KisCanvas2> canvas;
    RecorderWriterManager writer;

    QAction *recordToggleAction = nullptr;
    QAction *exportAction = nullptr;

    QString snapshotDirectory;
    QString prefix;
    QString outputDirectory;
    double captureInterval = 0.;
    RecorderFormat format = RecorderFormat::JPEG;
    int quality = 0;
    int compression = 0;
    int resolution = 0;
    bool realTimeCaptureMode = false;
    bool recordIsolateLayerMode = false;
    bool recordAutomatically = false;
    bool paused = true;
    QTimer pausedTimer;
    QTimer warningTimer;

    QLabel* statusBarLabel;
    QLabel* statusBarWarningLabel;

    QMap<QString, bool> enabledIds;

    Private(const RecorderExportSettings &es, RecorderDockerDock *q_ptr)
        : q(q_ptr)
        , ui(new Ui::RecorderDocker())
        , writer(es)
        , statusBarLabel(new QLabel())
        , statusBarWarningLabel(new QLabel())
    {
        updateRecIndicator();
        statusBarWarningLabel->setPixmap(KisIconUtils::loadIcon("warning").pixmap(16, 16));
        statusBarWarningLabel->hide();
        warningTimer.setInterval(10000);
        warningTimer.setSingleShot(true);
        pausedTimer.setSingleShot(true);
        connect(&warningTimer, SIGNAL(timeout()), q, SLOT(onWarningTimeout()));
        connect(&pausedTimer, SIGNAL(timeout()), q, SLOT(onPausedTimeout()));
    }

    void loadSettings()
    {
        RecorderConfig config(true);
        snapshotDirectory = config.snapshotDirectory();
#ifdef Q_OS_ANDROID
        fixInternalSnapshotDirectory();
#endif
        captureInterval = config.captureInterval();
        format = config.format();
        quality = config.quality();
        compression = config.compression();
        resolution = config.resolution();
        writer.recorderThreads.set(config.threads());
        realTimeCaptureMode = config.realTimeCaptureMode();
        if (realTimeCaptureMode) {
            q->exportSettings->lockFps = true;
            q->exportSettings->realTimeCaptureModeWasSet = true;
        }
        recordIsolateLayerMode = config.recordIsolateLayerMode();
        recordAutomatically = config.recordAutomatically();

        updateUiFormat();
    }

    void loadRelevantExportSettings()
    {
        RecorderExportConfig config(true);
        q->exportSettings->fps = config.fps();
    }

    void updateUiFormat() {
        int index = 0;
        QString title;
        QString hint;
        int minValue = 0;
        int maxValue = 0;
        QString suffix;
        int factor = 0;
        switch (format) {
            case RecorderFormat::JPEG:
                index = 0;
                title = i18nc("Title for label. JPEG Quality level", "Quality:");
                hint = i18nc("@tooltip", "Greater value will produce a larger file and a better quality. Doesn't affect CPU consumption.\nValues lower than 50 are not recommended due to high artifacts.");
                minValue = 1;
                maxValue = 100;
                suffix = "%";
                factor = quality;
                break;
            case RecorderFormat::PNG:
                index = 1;
                title = i18nc("Title for label. PNG Compression level", "Compression:");
                hint = i18nc("@tooltip", "Greater value will produce a smaller file but will require more from your CPU. Doesn't affect quality.\nCompression set to 0 is not recommended due to high disk space consumption.\nValues above 3 are not recommended due to high performance impact.");
                minValue = 0;
                maxValue = 5;
                suffix = "";
                factor = compression;
                break;
        }

        ui->comboFormat->setCurrentIndex(index);
        ui->labelQuality->setText(title);
        ui->spinQuality->setToolTip(hint);
        QSignalBlocker blocker(ui->spinQuality);
        ui->spinQuality->setMinimum(minValue);
        ui->spinQuality->setMaximum(maxValue);
        ui->spinQuality->setValue(factor);
        ui->spinQuality->setSuffix(suffix);
    }

    void updateUiForRealTimeMode() {
        QString title;
        double minValue = 0;
        double maxValue = 0;
        double value = 0;
        int decimals = 0;
        QString suffix;
        QSignalBlocker blocker(ui->spinRate);

        if (realTimeCaptureMode) {
                title = i18nc("Title for label. Video frames per second", "Video FPS:");
                minValue = 1;
                maxValue = 60;
                decimals = 0;
                value = q->exportSettings->fps;
                suffix = "";
                disconnect(ui->spinRate, SIGNAL(valueChanged(double)), q, SLOT(onCaptureIntervalChanged(double)));
                connect(ui->spinRate, SIGNAL(valueChanged(double)), q, SLOT(onVideoFPSChanged(double)));
        } else {
                title = i18nc("Title for label. Capture rate", "Capture interval:");
                minValue = 0.10;
                maxValue = 100.0;
                decimals = 1;
                value = captureInterval;
                suffix = " sec.";
                disconnect(ui->spinRate, SIGNAL(valueChanged(double)), q, SLOT(onVideoFPSChanged(double)));
                connect(ui->spinRate, SIGNAL(valueChanged(double)), q, SLOT(onCaptureIntervalChanged(double)));
        }

        ui->labelRate->setText(title);
        ui->spinRate->setDecimals(decimals);
        ui->spinRate->setMinimum(minValue);
        ui->spinRate->setMaximum(maxValue);
        ui->spinRate->setSuffix(suffix);
        ui->spinRate->setValue(value);
    }

    void updateWriterSettings()
    {
        outputDirectory = snapshotDirectory % QDir::separator() % prefix % QDir::separator();
        writer.setup({
            outputDirectory,
            format,
            quality,
            compression,
            resolution,
            captureInterval,
            recordIsolateLayerMode,
            realTimeCaptureMode});
    }

    QString getPrefix()
    {
        return !canvas ? ""
               : canvas->imageView()->document()->documentInfo()->aboutInfo("creation-date").remove(QRegularExpression("[^0-9]"));
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

    void updateRecordStatus(bool isRecording)
    {
        recordToggleAction->setChecked(isRecording);
        recordToggleAction->setEnabled(true);

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
            statusBar->addExtraWidget(statusBarWarningLabel);
            statusBar->addExtraWidget(statusBarLabel);
            updateRecIndicator();
        } else {
            statusBar->removeExtraWidget(statusBarWarningLabel);
            statusBar->removeExtraWidget(statusBarLabel);
        }
    }

    void updateRecIndicator()
    {
        auto threads = writer.recorderThreads.get();
        auto threadsInUse = writer.recorderThreads.getUsed();
        QString label("<font style='letter-spacing:-4px'>");
        QString activeColor;
        QString inactiveColor;
        for (unsigned int threadNr = 1; threadNr <= ThreadSystemValue::MaxThreadCount ; threadNr++)
        {
            if (threadNr > threads) {
                activeColor = inactiveColorGray;
                inactiveColor = inactiveColorGray;
            } else if (threadNr > ThreadSystemValue::MaxRecordThreadCount) {
                activeColor = activeColorRed;
                inactiveColor = inactiveColorRed;
            } else if (threadNr > ThreadSystemValue::IdealRecordThreadCount) {
                activeColor = activeColorOrange;
                inactiveColor = inactiveColorOrange;
            } else {
                activeColor = activeColorGreen;
                inactiveColor = inactiveColorGreen;
            }
            label.append(QString("<font%1>▍</font>")
                             .arg(threadNr <= threadsInUse ? activeColor : inactiveColor));
        }
        // don't remove empty <font></font> tag else label will jump a few pixels around
        label.append(QString("</font><font> %1 </font><font%2>●</font>")
                         .arg(i18nc("Recording symbol", "REC"))
                         .arg(paused ? "" : activeColorRed));
        statusBarLabel->setText(label);
        statusBarLabel->setToolTip(paused ? i18n("Recorder is paused") : QString(i18n("Active recording with %1 of %2 available threads")).arg(threadsInUse).arg(threads));
    }

    void showWarning(const QString &hint) {
        if (statusBarWarningLabel->isHidden()) {
            statusBarWarningLabel->setToolTip(hint);
            statusBarWarningLabel->show();
            warningTimer.start();
        }
    }

    void updateThreadUi()
    {
        QString toolTipText;
        auto threads = writer.recorderThreads.get();
        if (threads > ThreadSystemValue::MaxRecordThreadCount) {
            // Number of threads exceeds ideal thread count
            // -> switch color of threads slider and spin wheel to red
            QPalette pal;
            pal.setColor(QPalette::Text, textColorRed);
            pal.setColor(QPalette::Button, buttonColorRed);
            ui->spinThreads->setPalette(pal);
            ui->sliderThreads->setPalette(pal);
            toolTipText = QString(
                i18n("Set the number of recording threads.\nThe number of threads exceeds the ideal max number of your hardware setup.\nPlease be aware, that a number greater than %1 probably won't give you any performance boost.")
                    .arg(ThreadSystemValue::MaxRecordThreadCount));
        } else if (threads > ThreadSystemValue::IdealRecordThreadCount) {
            // Number of threads exceeds ideal recorder thread count
            // -> switch color of threads slider and spin wheel to orange
            QPalette pal;
            pal.setColor(QPalette::Text, textColorOrange);
            pal.setColor(QPalette::Button, buttonColorOrange);
            ui->spinThreads->setPalette(pal);
            ui->sliderThreads->setPalette(pal);
            toolTipText = QString(
                i18n("Set the number of recording threads.\nAccording to your hardware setup you should record with no more than %1 threads.\nYou can play around with one or two more threads, but keep an eye on your overall system performance.")
                    .arg(ThreadSystemValue::IdealRecordThreadCount));
        } else {
            ui->spinThreads->setPalette(threadsSpinPalette);
            ui->sliderThreads->setPalette(threadsSliderPalette);
            toolTipText = i18n("Set the number of threads to be used for recording.");
        }
        ui->spinThreads->setToolTip(toolTipText);
        ui->sliderThreads->setToolTip(toolTipText);
    }

#ifdef Q_OS_ANDROID
    // We want at most one file mover to run at a time. This starts at 0, gets
    // compared-and-set from 0 to 1 when a move starts and then set to 0 again
    // when the move finishes. Basically a non-waitable lock.
    static inline QAtomicInt internalMoveInProgress;

    void fixInternalSnapshotDirectory()
    {
        // Older versions of Krita used an internal directory as the snapshots
        // directory by default, which is a bogus place to save stuff to because
        // the user can't access it. That means the files stored there are stuck
        // inaccessible and once the user picks a "real" directory, they can no
        // longer even delete the files. So here we're rectifying the situation.
        const QString &internalPath = RecorderConfig::defaultInternalSnapshotDirectory();

        if (snapshotDirectory == internalPath) {
            // Internal path got persisted to settings. Clear that out, replace
            // it with the default of nothing.
            snapshotDirectory = QString();

        } else if (!snapshotDirectory.isEmpty() && QFileInfo::exists(internalPath)
                   && internalMoveInProgress.testAndSetOrdered(0, 1)) {
            // The user has picked a directory to record to, but the nonsense
            // internal directory is present and may have stuff inside that
            // would become effectively inaccessible. To fix that, we move the
            // files over to the selected directory. Of course moving files on
            // Android is gobsmackingly slow, so we'll have to do it in the
            // background to not lock the UI for ages. The moving should be
            // re-entrant, so getting interrupted and continuing later is fine.
            qWarning().nospace() << "Moving recordings stuck in internal directory '" << internalPath
                                 << "' to selected directory '" << snapshotDirectory << "'";
            QThreadPool::globalInstance()->start(new InternalSnapshotsMover(internalPath, snapshotDirectory));
        }
    }

    class InternalSnapshotsMover final : public QRunnable
    {
    public:
        InternalSnapshotsMover(const QString &srcRoot, const QString &dstRoot)
            : m_srcRoot(srcRoot)
            , m_dstRoot(dstRoot)
        {
        }

        void run() override
        {
            moveFromInternalSnapshotDirectory(QDir(m_srcRoot), QDir(m_dstRoot));
            if (!QDir().rmdir(m_srcRoot)) {
                qWarning().nospace() << "Failed to remove root directory '" << m_srcRoot << "'";
            }
            internalMoveInProgress.storeRelease(0);
        }

    private:
        static constexpr QDir::Filters FILTERS = QDir::Dirs | QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot;

        static void moveFromInternalSnapshotDirectory(const QDir &src, const QDir &dst)
        {
            for (const QFileInfo &srcInfo : src.entryInfoList(FILTERS)) {
                QString srcName = srcInfo.fileName();
                QString dstPath = dst.filePath(srcName);

                if (srcInfo.isDir()) {
                    // Move the directory over recursively.
                    if (dst.mkpath(dstPath)) {
                        moveFromInternalSnapshotDirectory(QDir(srcInfo.filePath()), QDir(dstPath));
                    } else {
                        qWarning().nospace()
                            << "Failed to create directory '" << dstPath << "' in '" << dst.path() << "'";
                    }

                    // Removal will fail if the directory is non-empty, so we
                    // can just attempt it unconditionally.
                    if (!src.rmdir(srcName)) {
                        qWarning().nospace()
                            << "Failed to remove directory '" << srcName << "' in '" << src.path() << "'";
                    }

                } else {
                    QFile srcFile(srcInfo.filePath());
                    // Rename refuses to replace files in the destination, so
                    // try to remove that first. The only reason it should
                    // already exist is if a previous attempt to move the file
                    // partially copied it and then got interrupted.
                    QFile::remove(dstPath);
                    if (!srcFile.rename(dstPath)) {
                        qWarning().nospace() << "Error " << srcFile.error() << " moving '" << srcFile.fileName()
                                             << "' to '" << dstPath << "': " << srcFile.errorString();
                    }
                }
            }
        }

        const QString m_srcRoot;
        const QString m_dstRoot;
    };
#endif
};

RecorderDockerDock::RecorderDockerDock()
    : QDockWidget(i18nc("Title of the docker", "Recorder"))
    , exportSettings(new RecorderExportSettings())
    , d(new Private(*exportSettings, this))
{
    QWidget* page = new QWidget(this);
    d->ui->setupUi(page);

    d->ui->buttonManageRecordings->setIcon(KisIconUtils::loadIcon("configure-thicker"));
    d->ui->buttonBrowse->setIcon(KisIconUtils::loadIcon("folder"));
    d->ui->buttonRecordToggle->setIcon(KisIconUtils::loadIcon("media-record"));
    d->ui->buttonExport->setIcon(KisIconUtils::loadIcon("document-export-16"));
    d->ui->sliderThreads->setTickPosition(QSlider::TickPosition::TicksBelow);
    d->ui->sliderThreads->setMinimum(1);
    d->ui->sliderThreads->setMaximum(ThreadSystemValue::MaxThreadCount);
    d->ui->spinThreads->setMinimum(1);
    d->ui->spinThreads->setMaximum(ThreadSystemValue::MaxThreadCount);
    d->threadsSpinPalette = d->ui->spinThreads->palette();
    d->threadsSliderPalette = d->ui->sliderThreads->palette();

    d->loadSettings();
    d->loadRelevantExportSettings();
    d->updateThreadUi();

    d->ui->editDirectory->setText(d->snapshotDirectory);
    d->ui->spinQuality->setValue(d->quality);
    d->ui->spinThreads->setValue(d->writer.recorderThreads.get());
    d->ui->comboResolution->setCurrentIndex(d->resolution);
    d->ui->checkBoxRealTimeCaptureMode->setChecked(d->realTimeCaptureMode);
    d->ui->checkBoxRecordIsolateMode->setChecked(d->recordIsolateLayerMode);
    d->ui->checkBoxAutoRecord->setChecked(d->recordAutomatically);

    KisActionRegistry *actionRegistry = KisActionRegistry::instance();
    d->recordToggleAction = actionRegistry->makeQAction(keyActionRecordToggle, this);
    d->exportAction = actionRegistry->makeQAction(keyActionExport, this);

    connect(d->recordToggleAction, SIGNAL(toggled(bool)), d->ui->buttonRecordToggle, SLOT(setChecked(bool)));
    connect(d->exportAction, SIGNAL(triggered()), d->ui->buttonExport, SIGNAL(clicked()));
    connect(d->ui->buttonRecordToggle, SIGNAL(toggled(bool)), d->ui->buttonExport, SLOT(setDisabled(bool)));
    if (d->recordAutomatically)
        d->ui->buttonExport->setDisabled(true);

    // Need to register toolbar actions before attaching canvas else it wont appear after restart.
    // Is there any better way to do this?
    connect(KisPart::instance(), SIGNAL(sigMainWindowIsBeingCreated(KisMainWindow *)),
            this, SLOT(onMainWindowIsBeingCreated(KisMainWindow *)));

    connect(d->ui->buttonManageRecordings, SIGNAL(clicked()), this, SLOT(onManageRecordingsButtonClicked()));
    connect(d->ui->buttonBrowse, SIGNAL(clicked()), this, SLOT(slotSelectSnapshotDirectory()));
    connect(d->ui->comboFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(onFormatChanged(int)));
    connect(d->ui->spinQuality, SIGNAL(valueChanged(int)), this, SLOT(onQualityChanged(int)));
    connect(d->ui->spinThreads, SIGNAL(valueChanged(int)), this, SLOT(onThreadsChanged(int)));
    connect(d->ui->comboResolution, SIGNAL(currentIndexChanged(int)), this, SLOT(onResolutionChanged(int)));
    connect(d->ui->checkBoxRealTimeCaptureMode, SIGNAL(toggled(bool)), this, SLOT(onRealTimeCaptureModeToggled(bool)));
    connect(d->ui->checkBoxRecordIsolateMode, SIGNAL(toggled(bool)), this, SLOT(onRecordIsolateLayerModeToggled(bool)));
    connect(d->ui->checkBoxAutoRecord, SIGNAL(toggled(bool)), this, SLOT(onAutoRecordToggled(bool)));
    connect(d->ui->buttonRecordToggle, SIGNAL(toggled(bool)), this, SLOT(onRecordButtonToggled(bool)));
    connect(d->ui->buttonExport, SIGNAL(clicked()), this, SLOT(onExportButtonClicked()));

    connect(&d->writer.recorderThreads, SIGNAL(notifyInUseChange(bool)), this, SLOT(onActiveRecording(bool)));
    connect(&d->writer.recorderThreads, SIGNAL(notifyInUseChange(bool)), this, SLOT(onUpdateRecIndicator()));
    connect(&d->writer, SIGNAL(started()), this, SLOT(onWriterStarted()));
    connect(&d->writer, SIGNAL(stopped()), this, SLOT(onWriterStopped()));
    connect(&d->writer, SIGNAL(frameWriteFailed()), this, SLOT(onWriterFrameWriteFailed()));
    connect(&d->writer, SIGNAL(recorderStopWarning()), this, SLOT(onRecorderStopWarning()));
    connect(&d->writer, SIGNAL(lowPerformanceWarning()), this, SLOT(onLowPerformanceWarning()));


    QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(d->ui->scrollArea);
    if (scroller) {
        connect(scroller, SIGNAL(stateChanged(QScroller::State)),
                this, SLOT(slotScrollerStateChanged(QScroller::State)));
    }

    // The system is not efficient enough for the RealTime Recording Feature
    if (ThreadSystemValue::MaxRecordThreadCount <= 1)
    {
        d->ui->checkBoxRealTimeCaptureMode->setCheckState(Qt::Unchecked);
        d->ui->checkBoxRealTimeCaptureMode->setDisabled(true);
        d->ui->checkBoxRealTimeCaptureMode->setToolTip(
            i18n("Your system is not efficient enough for this feature"));
    }

    setWidget(page);
}

RecorderDockerDock::~RecorderDockerDock()
{
    delete d;
    delete exportSettings;
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
    d->updateComboResolution(document->image()->width(), document->image()->height());

    d->prefix = d->getPrefix();
    bool wasToggled = false;
    if (d->recordAutomatically && !d->snapshotDirectory.isEmpty()
        && !d->enabledIds.contains(document->linkedResourcesStorageId())) {
        wasToggled = onRecordButtonToggled(true);
    }
    if (!wasToggled) { // onRecordButtonToggled(true) may call these, don't call them twice.
        d->updateWriterSettings();
        d->updateUiFormat();
    }
    d->updateUiForRealTimeMode();

    bool enabled = d->enabledIds.value(document->linkedResourcesStorageId(), false);
    d->writer.setEnabled(enabled);
    d->updateRecordStatus(enabled);
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
    KisKActionCollection *actionCollection = window->viewManager()->actionCollection();
    actionCollection->addAction(keyActionRecordToggle, d->recordToggleAction);
    actionCollection->addAction(keyActionExport, d->exportAction);
}

bool RecorderDockerDock::onRecordButtonToggled(bool checked)
{
    QSignalBlocker blocker(d->ui->buttonRecordToggle);

    // Ask the user to pick a directory if we don't have one. This should only
    // happen on Android, other operating systems have a non-empty default that
    // the user is not able to clear out via the user interface.
    if (checked && d->snapshotDirectory.isEmpty()) {
        slotSelectSnapshotDirectory();
        if (d->snapshotDirectory.isEmpty()) {
            d->ui->buttonRecordToggle->setChecked(false);
            d->recordToggleAction->setChecked(false);
            return false;
        }
    }

    d->recordToggleAction->setChecked(checked);

    if (!d->canvas)
        return false;

    const QString &id = d->canvas->imageView()->document()->linkedResourcesStorageId();

    bool wasEmpty = !d->enabledIds.values().contains(true);

    d->enabledIds[id] = checked;

    bool isEmpty = !d->enabledIds.values().contains(true);

    d->writer.setEnabled(checked);

    if (isEmpty == wasEmpty) {
        d->updateRecordStatus(checked);
        return false;
    }


    d->ui->buttonRecordToggle->setEnabled(false);

    if (checked) {
        d->updateWriterSettings();
        d->updateUiFormat();
        d->writer.start();

        // Calculate Rec symbol activity timeout depending on the capture interval
        // The pausedTimer interval is set to a slightly greater value than the capture interval
        // to avoid flickering for ongoing painting. This is also the reason for the min and max
        // values 305 and 2005 (instead of 300 and 2000, respectively).
        if (d->realTimeCaptureMode) {
            d->pausedTimer.setInterval(qBound(305, static_cast<int>(1000.0/static_cast<double>(exportSettings->fps)) + 5,2005));
        } else {
            d->pausedTimer.setInterval(qBound(305, static_cast<int>(qMax(d->captureInterval, .1) * 1000.0) + 5, 2005));
        }
    } else {
        d->writer.stop();
        d->warningTimer.stop();
        d->pausedTimer.stop();
        d->statusBarWarningLabel->hide();
        d->paused = true;
    }

    return true;
}

void RecorderDockerDock::onExportButtonClicked()
{
    if (!d->canvas)
        return;

    KisDocument *document = d->canvas->imageView()->document();

    exportSettings->videoFileName = QFileInfo(document->caption().trimmed()).completeBaseName();
    exportSettings->inputDirectory = d->outputDirectory;
    exportSettings->format = d->format;
    exportSettings->realTimeCaptureMode = d->realTimeCaptureMode;

    RecorderExport exportDialog(exportSettings, this);
    exportDialog.setup();
    exportDialog.exec();

    if (d->realTimeCaptureMode)
        d->ui->spinRate->setValue(exportSettings->fps);
}

void RecorderDockerDock::onManageRecordingsButtonClicked()
{
    RecorderSnapshotsManager snapshotsManager(this);
    snapshotsManager.execFor(d->snapshotDirectory);
}

void RecorderDockerDock::slotSelectSnapshotDirectory()
{
    KoFileDialog dialog(this, KoFileDialog::OpenDirectory, "SelectRecordingsDirectory");
    dialog.setCaption(i18n("Select a Directory for Recordings"));
    dialog.setDefaultDir(d->ui->editDirectory->text());
    QString directory = dialog.filename();
    if (!directory.isEmpty()) {
        d->ui->editDirectory->setText(directory);
        RecorderConfig(false).setSnapshotDirectory(directory);
        d->loadSettings();
    }
}

void RecorderDockerDock::onRecordIsolateLayerModeToggled(bool checked)
{
    d->recordIsolateLayerMode = checked;
    RecorderConfig(false).setRecordIsolateLayerMode(checked);
    d->loadSettings();
}

void RecorderDockerDock::onAutoRecordToggled(bool checked)
{
    d->recordAutomatically = checked;
    RecorderConfig(false).setRecordAutomatically(checked);
    d->loadSettings();
}

void RecorderDockerDock::onRealTimeCaptureModeToggled(bool checked)
{
    d->realTimeCaptureMode = checked;
    RecorderConfig(false).setRealTimeCaptureMode(checked);
    d->loadSettings();
    d->updateUiForRealTimeMode();
    if (d->realTimeCaptureMode) {
        exportSettings->lockFps = true;
        exportSettings->realTimeCaptureModeWasSet = true;
    }
}

void RecorderDockerDock::onCaptureIntervalChanged(double interval)
{
    d->captureInterval = interval;
    RecorderConfig(false).setCaptureInterval(interval);
    d->loadSettings();
}
void RecorderDockerDock::onVideoFPSChanged(double fps)
{
    exportSettings->fps = fps;
    RecorderExportConfig(false).setFps(fps);
    d->loadRelevantExportSettings();
}

void RecorderDockerDock::onQualityChanged(int value)
{
    switch (d->format) {
    case RecorderFormat::JPEG:
        d->quality = value;
        RecorderConfig(false).setQuality(value);
        d->loadSettings();
        break;
    case RecorderFormat::PNG:
        d->compression = value;
        RecorderConfig(false).setCompression(value);
        d->loadSettings();
        break;
    }
}

void RecorderDockerDock::onFormatChanged(int format)
{
    d->format = static_cast<RecorderFormat>(format);
    d->updateUiFormat();

    RecorderConfig(false).setFormat(d->format);
    d->loadSettings();
}

void RecorderDockerDock::onResolutionChanged(int resolution)
{
    d->resolution = resolution;
    RecorderConfig(false).setResolution(resolution);
    d->loadSettings();
}

void RecorderDockerDock::onThreadsChanged(int threads)
{
    d->writer.recorderThreads.set(threads);
    RecorderConfig(false).setThreads(threads);
    d->loadSettings();
    d->updateThreadUi();
}

void RecorderDockerDock::onWriterStarted()
{
    d->updateRecordStatus(true);
}

void RecorderDockerDock::onWriterStopped()
{
    d->updateRecordStatus(false);
}

void RecorderDockerDock::onUpdateRecIndicator()
{
    d->updateRecIndicator();
}

void RecorderDockerDock::onActiveRecording(bool valueWasIncreased)
{
    if (!valueWasIncreased)
        return;

    d->paused = false;
    d->pausedTimer.start();
}

void RecorderDockerDock::onPausedTimeout()
{
    d->paused = true;
    d->updateRecIndicator();
}

void RecorderDockerDock::onWriterFrameWriteFailed()
{
    QMessageBox::warning(this, i18nc("@title:window", "Recorder"),
                         i18n("The recorder has been stopped due to failure while writing a frame. Please check free disk space and start the recorder again."));
}

void RecorderDockerDock::onRecorderStopWarning()
{
    QMessageBox::warning(this, i18nc("@title:window", "Recorder"),
                         i18n("Krita was unable to stop the recorder probably. Please try to restart Krita."));
}
void RecorderDockerDock::onLowPerformanceWarning()
{
    if (d->realTimeCaptureMode) {
        d->showWarning(i18n("Low performance warning. The recorder is not able to write all the frames in time during Real Time Capture mode.\nTry to reduce the frame rate for the ffmpeg export or reduce the scaling filtering in the canvas acceleration settings."));
    } else {
        d->showWarning(i18n("Low performance warning. The recorder is not able to write all the frames in time.\nTry to increase the capture interval or reduce the scaling filtering in the canvas acceleration settings."));
    }
}

void RecorderDockerDock::onWarningTimeout()
{
    d->statusBarWarningLabel->hide();
}

void RecorderDockerDock::slotScrollerStateChanged(QScroller::State state)
{
    KisKineticScroller::updateCursor(this, state);
}
