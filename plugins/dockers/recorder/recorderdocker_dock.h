/*
 *  SPDX-FileCopyrightText: 2019 Shi Yan <billconan@gmail.net>
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _RECORDER_DOCK_H_
#define _RECORDER_DOCK_H_

#include <QScroller>
#include <QDockWidget>
#include <KoCanvasObserverBase.h>
#ifdef Q_OS_ANDROID
#include <QDir>
#include <QRunnable>
#endif


class KisMainWindow;
struct RecorderExportSettings;

class RecorderDockerDock : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    RecorderDockerDock();
    ~RecorderDockerDock();
    QString observerName() override
    {
        return "RecorderDockerDock";
    }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

private Q_SLOTS:
    void onMainWindowIsBeingCreated(KisMainWindow *window);

    void onRecordIsolateLayerModeToggled(bool checked);
    void onAutoRecordToggled(bool checked);
    void onRealTimeCaptureModeToggled(bool checked);
    void onCaptureIntervalChanged(double interval);
    void onVideoFPSChanged(double interval);
    void onQualityChanged(int value);
    void onFormatChanged(int format);
    void onResolutionChanged(int resolution);
    void onThreadsChanged(int threads);
    void onManageRecordingsButtonClicked();
    void slotSelectSnapshotDirectory();
    bool onRecordButtonToggled(bool checked);
    void onExportButtonClicked();

    void onWriterStarted();
    void onWriterStopped();
    void onUpdateRecIndicator();
    void onActiveRecording(bool valueWasIncreased);
    void onPausedTimeout();
    void onWriterFrameWriteFailed();
    void onRecorderStopWarning();
    void onLowPerformanceWarning();
    void onWarningTimeout();

    void slotScrollerStateChanged(QScroller::State state);

#ifdef Q_OS_ANDROID
    void moveFilesFromInternalSnapshotDirectory();
    void slotInternalSnapshotMoveFinished(const QString &srcRoot);
#endif

private:
    Q_DISABLE_COPY(RecorderDockerDock)
    class Private;
    RecorderExportSettings *const exportSettings;
    Private *const d;
};

#ifdef Q_OS_ANDROID
class RecorderDockerInternalSnapshotsMover final : public QObject, public QRunnable
{
    Q_OBJECT
public:
    RecorderDockerInternalSnapshotsMover(const QString &srcRoot, const QString &dstRoot);

    void run() override;

Q_SIGNALS:
    void sigMoveFinished(const QString &srcRoot);

private:
    static constexpr QDir::Filters FILTERS = QDir::Dirs | QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot;

    static void moveFromInternalSnapshotDirectory(const QDir &src, const QDir &dst);

    const QString m_srcRoot;
    const QString m_dstRoot;
};
#endif

#endif
