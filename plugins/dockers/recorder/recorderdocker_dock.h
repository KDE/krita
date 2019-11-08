/*
 *  Copyright (c) 2019 Shi Yan <billconan@gmail.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
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

#ifndef _RECORDER_DOCK_H_
#define _RECORDER_DOCK_H_

#include "encoder.h"
#include "kis_idle_watcher.h"
#include <KoCanvasObserverBase.h>
#include <kis_canvas2.h>
#include <QDockWidget>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QSpacerItem>

class QVBoxLayout;
class RecorderWidget;

class RecorderDockerDock : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    RecorderDockerDock();
    QString observerName() override
    {
        return "RecorderDockerDock";
    }
    void setCanvas(KoCanvasBase* canvas) override;
    void unsetCanvas() override;

private:
    QGridLayout* m_layout;
    QPointer<KisCanvas2> m_recordingCanvas;
    QString m_recordPath;

    QPointer<KisCanvas2> m_canvas;
    QLabel* m_recordDirectoryLabel;
    QLineEdit* m_recordDirectoryLineEdit;
    QPushButton* m_recordDirectoryPushButton;
    QLabel* m_imageNameLabel;
    QLineEdit* m_imageNameLineEdit;
    QPushButton* m_recordToggleButton;
    QSpacerItem* m_spacer;
    QLabel* m_logLabel;
    QLineEdit* m_logLineEdit;
    KisIdleWatcher m_imageIdleWatcher;
    QMutex m_saveMutex;
    QMutex m_eventMutex;
    Encoder* m_encoder;

    bool m_recordEnabled;
    int m_recordCounter;
    void enableRecord(bool& enabled, const QString& path);

private Q_SLOTS:
    void onRecordButtonToggled(bool enabled);
    void onSelectRecordFolderButtonClicked();
    void startUpdateCanvasProjection();
    void generateThumbnail();
};

#endif
