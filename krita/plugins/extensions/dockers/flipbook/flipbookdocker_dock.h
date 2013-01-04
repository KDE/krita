/*
 *  Copyright (c) 2012 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef FLIPBOOKDOCKER_DOCK_H
#define FLIPBOOKDOCKER_DOCK_H

#include <QDockWidget>
#include <KoCanvasObserverBase.h>

#include "FlipbookView.h"

#include "ui_wdgflipbookdocker.h"

class KoResourceLoaderThread;
class KisFlipbook;
class QListView;
class QThread;
class KisCanvas2;
class FlipbookView;
class SequenceViewer;
class KoMainWindow;

class FlipbookDockerDock : public QDockWidget, public KoCanvasObserverBase, public Ui_WdgFlipbookDocker {
    Q_OBJECT
public:
    FlipbookDockerDock();
    ~FlipbookDockerDock();

    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas();
    

private slots:

    void updateLayout(Qt::DockWidgetArea area);

    void saveFlipbook();
    void newFlipbook();
    void openFlipbook();

    void addImage();
    void removeImage();

    void goFirst();
    void goPrevious();
    void goNext();
    void goLast();

    void selectImage(const QModelIndex &index);

    void toggleAnimation();

private:

    void showCurrentItem();

    KisCanvas2 *m_canvas;
    KisFlipbook *m_flipbook;

    bool m_animating;
    SequenceViewer *m_animationWidget;
    QWidget *m_canvasWidget;
};


#endif

