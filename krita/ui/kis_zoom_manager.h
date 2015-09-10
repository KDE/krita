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
#include <QPointer>

#include <klocalizedstring.h>

#include <KoZoomMode.h>
#include <KoZoomAction.h>
#include <KoZoomHandler.h>
#include <KoZoomController.h>

#include "KisView.h"

class KoZoomHandler;
class QAction;
class KoZoomAction;
class KoRuler;
class KoUnit;
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

    KisZoomManager(QPointer<KisView> view, KoZoomHandler*, KoCanvasController *);
    ~KisZoomManager();

    void setup(KActionCollection * actionCollection);
    void updateGUI();
    KoZoomController * zoomController() const {
        return m_zoomController;
    }

    QWidget *zoomActionWidget() const;

public Q_SLOTS:

    void slotZoomChanged(KoZoomMode::Mode mode, qreal zoom);
    void slotScrollAreaSizeChanged();
    void toggleShowRulers(bool show);
    void mousePositionChanged(const QPoint &viewPos);
    void changeAspectMode(bool aspectMode);
    void pageOffsetChanged();
    void zoomTo100();
    void showGuides(bool toggle);
    void applyRulersUnit(const KoUnit &baseUnit);
    void setMinMaxZoom();

public:
    bool horizontalRulerVisible() const;
    bool verticalRulerVisible() const;

private:

    QPointer<KisView> m_view;
    KoZoomHandler * m_zoomHandler;
    KoCanvasController *m_canvasController;
    KoZoomController *m_zoomController;
    KoRuler * m_horizontalRuler;
    KoRuler * m_verticalRuler;
    KoZoomAction * m_zoomAction;
    QPointer<QWidget> m_zoomActionWidget;
    QPoint m_rulersOffset;
};

#endif
