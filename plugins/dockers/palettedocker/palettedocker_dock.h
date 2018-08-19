/*
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#ifndef PALETTEDOCKER_DOCK_H
#define PALETTEDOCKER_DOCK_H

#include <QDockWidget>
#include <QModelIndex>
#include <QPointer>

#include <KoCanvasObserverBase.h>
#include <KoResourceServerAdapter.h>
#include <KoResourceServerObserver.h>
#include <resources/KoColorSet.h>

#include <kis_canvas2.h>
#include <kis_mainwindow_observer.h>

class KisViewManager;
class KisCanvasResourceProvider;
class KisWorkspaceResource;
class KisColorsetChooser;
class KisPaletteModel;
class Ui_WdgPaletteDock;


class PaletteDockerDock : public QDockWidget, public KisMainwindowObserver, public KoResourceServerObserver<KoColorSet> {
    Q_OBJECT
public:
    PaletteDockerDock();
    ~PaletteDockerDock() override;
    QString observerName() override { return "PaletteDockerDock"; }
    void setViewManager(KisViewManager* kisview) override;
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

public: // KoResourceServerObserver

    void unsetResourceServer() override;
    void resourceAdded(KoColorSet *) override {}
    void removingResource(KoColorSet *resource) override;
    void resourceChanged(KoColorSet *resource) override;
    void syncTaggedResourceView() override {}
    void syncTagAddition(const QString&) override {}
    void syncTagRemoval(const QString&) override {}

private Q_SLOTS:
    void addColorForeground();
    void removeColor();
    void entrySelected(KoColorSetEntry entry);
    void entrySelectedBack(KoColorSetEntry entry);
    void setColorSet(KoColorSet* colorSet);

    void setColorFromNameList(int index);

    void saveToWorkspace(KisWorkspaceResource* workspace);
    void loadFromWorkspace(KisWorkspaceResource* workspace);

private:    
    Ui_WdgPaletteDock* m_wdgPaletteDock;
    KisPaletteModel *m_model;
    QSharedPointer<KoAbstractResourceServerAdapter> m_serverAdapter;
    KoColorSet *m_currentColorSet;
    KisColorsetChooser *m_paletteChooser;
    KisCanvasResourceProvider *m_resourceProvider;
    QPointer<KisCanvas2> m_canvas;
};


#endif
