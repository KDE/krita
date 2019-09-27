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
#include <QScopedPointer>
#include <QVector>
#include <QPointer>
#include <QPair>
#include <QAction>
#include <QMenu>

#include <KoResourceServerObserver.h>
#include <KoResourceServer.h>
#include <resources/KoColorSet.h>

#include <kis_canvas2.h>
#include <kis_mainwindow_observer.h>
#include <KisView.h>
#include <kis_signal_auto_connection.h>

class KisViewManager;
class KisCanvasResourceProvider;
class KisWorkspaceResource;
class KisPaletteListWidget;
class KisPaletteModel;

class KisPaletteEditor;
class Ui_WdgPaletteDock;

class PaletteDockerDock : public QDockWidget, public KisMainwindowObserver
{
    Q_OBJECT
public:
    PaletteDockerDock();
    ~PaletteDockerDock() override;

public: // QDockWidget
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

public: // KisMainWindowObserver
    void setViewManager(KisViewManager* kisview) override;

private Q_SLOTS:
    void slotContextMenu(const QModelIndex &);

    void slotAddPalette();
    void slotRemovePalette(KoColorSet *);
    void slotImportPalette();
    void slotExportPalette(KoColorSet *);

    void slotAddColor();
    void slotRemoveColor();
    void slotEditEntry();
    void slotEditPalette();

    void slotPaletteIndexSelected(const QModelIndex &index);
    void slotPaletteIndexClicked(const QModelIndex &index);
    void slotPaletteIndexDoubleClicked(const QModelIndex &index);
    void slotNameListSelection(const KoColor &color);
    void slotSetColorSet(KoColorSet* colorSet);

    void saveToWorkspace(KisWorkspaceResource* workspace);
    void loadFromWorkspace(KisWorkspaceResource* workspace);

    void slotFGColorResourceChanged(const KoColor& color);
    void slotUpdatePaletteList(const QList<KoColorSet *> &oldPaletteList, const QList<KoColorSet *> &newPaletteList);

private:
    void setEntryByForeground(const QModelIndex &index);
    void setFGColorByPalette(const KisSwatch &entry);

private /* member variables */:
    QScopedPointer<Ui_WdgPaletteDock> m_ui;
    KisPaletteModel *m_model;
    KisPaletteListWidget *m_paletteChooser;

    QPointer<KisViewManager> m_view;
    KisCanvasResourceProvider *m_resourceProvider;

    KoResourceServer<KoColorSet> * const m_rServer;

    QPointer<KisDocument> m_activeDocument;
    QPointer<KoColorSet> m_currentColorSet;
    QScopedPointer<KisPaletteEditor> m_paletteEditor;

    QScopedPointer<QAction> m_actAdd;
    QScopedPointer<QAction> m_actRemove;
    QScopedPointer<QAction> m_actModify;
    QScopedPointer<QAction> m_actEditPalette;
    QMenu m_viewContextMenu;

    bool m_colorSelfUpdate;

    KisSignalAutoConnectionsStore m_connections;
};


#endif
