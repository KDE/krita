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

#include <QDockWidget>
#include <KoCanvasObserverBase.h>

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

private:
    Q_DISABLE_COPY(RecorderDockerDock)
    class Private;
    Private *const d;

private Q_SLOTS:
    void onUseDocNameToggled(bool checked);
    void onAutoRecordToggled(bool checked);
    void onEditPrefixChanged();
    void onCaptureIntervalChanged(int interval);
    void onQualityChanged(int quality);
    void onResolutionChanged(int resolution);
    void onRecordButtonToggled(bool checked);
    void onSelectRecordFolderButtonClicked();

    void onWriterStarted();
    void onWriterFinished();
    void onWriterPausedChanged(bool paused);
};

#endif
