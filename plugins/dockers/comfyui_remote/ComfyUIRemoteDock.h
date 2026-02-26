/*
 * SPDX-FileCopyrightText: 2025 Krita Project
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COMFYUI_REMOTE_DOCK_H_
#define COMFYUI_REMOTE_DOCK_H_

#include <QDockWidget>
#include <QScopedPointer>
#include <kis_mainwindow_observer.h>

class ComfyUIRemoteDock : public QDockWidget, public KisMainwindowObserver
{
    Q_OBJECT
public:
    ComfyUIRemoteDock();
    ~ComfyUIRemoteDock() override;

    QString observerName() override { return "ComfyUIRemoteDock"; }
    void setViewManager(KisViewManager *viewManager) override;
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

private:
    void startPolling();
    void updateQueueStatus();
    void refreshHistoryList();

private Q_SLOTS:
    void slotTestConnection();
    void slotRefreshCheckpoints();
    void slotLoadWorkflowFromFile();
    void slotPresetChanged(int index);
    void slotSaveAsPreset();
    void slotDeletePreset();
    void slotGenerate();
    void slotCancelQueue();
    void slotHistoryReRun();
    void slotHistoryItemSelected();
    void slotInpaint();
    void slotAddRegion();
    void slotRemoveRegion();
    void slotMoveRegionUp();
    void slotMoveRegionDown();
    void slotEditRegion();
    void slotGenerateRegions();
    void runNextRegionInpainting();
    void pollRegionHistory();

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif
