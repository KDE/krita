/*
 *  Copyright (c) 2020 Saurabh Kumar <saurabhk660@gmail.com>
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

#ifndef _STORYBOARDDOCKER_DOCK_H_
#define _STORYBOARDDOCKER_DOCK_H_

#include <QDockWidget>
#include <QButtonGroup>
#include <QItemSelection>

#include <kis_mainwindow_observer.h>
#include <kis_canvas2.h>
#include <kis_action.h>

class Ui_WdgStoryboardDock;
class CommentMenu;
class ArrangeMenu;
class CommentModel;
class StoryboardModel;

class StoryboardDockerDock : public QDockWidget, public KisMainwindowObserver{
    Q_OBJECT
public:
    StoryboardDockerDock();
    ~StoryboardDockerDock() override;
    QString observerName() override { return "StoryboardDockerDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
    void setViewManager(KisViewManager* kisview) override;

private Q_SLOTS:
    void slotExportAsPdf();
    void slotExportAsSvg();
    void slotExport(QString);

    void slotLockClicked(bool);
    void slotModeChanged(QAbstractButton*);
    void slotViewChanged(QAbstractButton*);

    void slotFrameChanged(int frameId);
    void slotChangeFrameGlobal(QItemSelection selected, QItemSelection deselected);

private:
    KisCanvas2* m_canvas;
    QScopedPointer<Ui_WdgStoryboardDock> m_ui;

    QMenu *m_exportMenu;
    KisAction *m_exportAsPdfAction;
    KisAction *m_exportAsSvgAction;

    QPointer<CommentModel> m_commentModel;
    CommentMenu *m_commentMenu;

    KisAction *m_lockAction;

    ArrangeMenu *m_arrangeMenu;

    QButtonGroup *m_modeGroup;
    QButtonGroup *m_viewGroup;

    QPointer<StoryboardModel> m_storyboardModel;

};


#endif
