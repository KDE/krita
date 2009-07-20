/*
 *  Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_ZOOM_MANAGER
#define KIS_ZOOM_MANAGER

#include <QObject>

#include <kstandardaction.h>
#include <klocale.h>

#include <KoZoomMode.h>
#include <KoZoomAction.h>
#include <KoZoomHandler.h>
#include <KoZoomController.h>

class KoZoomHandler;
class KisView2;
class KAction;
class KoZoomAction;
class KoRuler;
class KoCanvasController;
class QPoint;

/**
   The zoom manager handles all user actions related to zooming
   and unzooming. The actual computation of zoom levels and things
   are the job of KoZoomHandler or its descendants
*/
class KisZoomManager : public QObject
{

    Q_OBJECT

public:

    KisZoomManager(KisView2 * view, KoZoomHandler*, KoCanvasController *);
    ~KisZoomManager();

    void setup(KActionCollection * actionCollection);
    void updateGUI();
    KoZoomController * zoomController() {
        return m_zoomController;
    }

private slots:

    void slotZoomChanged(KoZoomMode::Mode mode, qreal zoom);
    void toggleShowRulers(bool show);
    void mousePositionChanged(const QPoint &pos);
    void changeAspectMode(bool aspectMode);
    void pageOffsetChanged();


private:

    KisView2 * m_view;
    KoZoomHandler * m_zoomHandler;
    KoCanvasController *m_canvasController;
    KoZoomController *m_zoomController;
    KoRuler * m_horizontalRuler;
    KoRuler * m_verticalRuler;
    QAction *m_showRulersAction;
    KoZoomAction * m_zoomAction;
    KAction * m_zoomIn;
    KAction * m_zoomOut;
    KAction * m_actualPixels;
    KAction * m_actualSize;
    KAction * m_fitToCanvas;
};

#endif
