/*
 *  SPDX-FileCopyrightText: 2019 Shi Yan <billconan@gmail.net>
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _RECORDER_DOCK_H_
#define _RECORDER_DOCK_H_

#include <kddockwidgets/DockWidget.h>
#include <KoCanvasObserverBase.h>

class KisMainWindow;

class RecorderDockerDock : public KDDockWidgets::DockWidget, public KoCanvasObserverBase
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
    void onCaptureIntervalChanged(int interval);
    void onQualityChanged(int quality);
    void onResolutionChanged(int resolution);
    void onManageRecordingsButtonClicked();
    void onSelectRecordFolderButtonClicked();
    void onRecordButtonToggled(bool checked);
    void onExportButtonClicked();

    void onWriterStarted();
    void onWriterFinished();
    void onWriterPausedChanged(bool paused);

private:
    Q_DISABLE_COPY(RecorderDockerDock)
    class Private;
    Private *const d;
};

#endif
