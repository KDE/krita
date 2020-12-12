/*
 *  Copyright (c) 2020 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef TOOLPRESET_DOCKER_H
#define TOOLPRESET_DOCKER_H

#include <QDockWidget>
#include <kis_mainwindow_observer.h>
#include <QWidget>
#include <KoDialog.h>

#include <ui_WdgToolPresets.h>
#include <ui_WdgNewToolPreset.h>

class QLabel;
class QListWidgetItem;
class KoCanvasBase;
class KisCanvas2;
class KoCanvasController;
class KisCanvasResourceProvider;
class ToolPresetModel;
class ToolPresets;

class DlgNewPreset : public KoDialog
{
    Q_OBJECT
public:
    DlgNewPreset();
    QString name();
    bool executeOnSelection();
    bool saveResourcesWithPreset();

    void accept() override;

private:

    Ui_WdgNewToolPreset m_ui;
};



class ToolPresetDocker : public QDockWidget, public KisMainwindowObserver , public Ui_WdgToolPresets
{
    Q_OBJECT
public:
    ToolPresetDocker();
    ~ToolPresetDocker() override;
    QString observerName() override { return "ToolPresetsDocker"; }

    void setViewManager(KisViewManager* kisview) override;

    void setCanvas(KoCanvasBase *) override;
    void unsetCanvas() override;

private Q_SLOTS:

    void optionWidgetsChanged(KoCanvasController *canvasController, QList<QPointer<QWidget> > optionWidgets);
    void toolChanged(KoCanvasController *canvasController, int uniqueToolId);

    void bnAddPressed();
    void bnDeletePressed();

    void presetSelected(QModelIndex);

private:
    KoCanvasController *m_canvasController {0};
    KisCanvasResourceProvider *m_resourceProvider {0};
    KoCanvasBase *m_canvas {0};
    QList<QPointer<QWidget> > m_currentOptionWidgets;
    QString m_currentToolId;
    ToolPresetModel *m_toolPresetModel {0};
};


#endif

