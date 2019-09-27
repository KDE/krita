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
#include "kis_signal_auto_connection.h"

#include "KisView.h"

class KoZoomHandler;
class KoZoomAction;
class KoRuler;
class KoUnit;
class KoCanvasController;
class QPoint;

#include "kritaui_export.h"

/**
 *  The zoom manager handles all user actions related to zooming
 *  and unzooming. The actual computation of zoom levels and things
 *  are the job of KoZoomHandler or its descendants
 */
class KRITAUI_EXPORT KisZoomManager : public QObject
{

    Q_OBJECT

public:

    KisZoomManager(QPointer<KisView> view, KoZoomHandler*, KoCanvasController *);
    ~KisZoomManager() override;

    void updateScreenResolution(QWidget *parentWidget);

    void setup(KActionCollection * actionCollection);
    void updateGUI();
    KoZoomController * zoomController() const {
        return m_zoomController;
    }

    void updateImageBoundsSnapping();

    QWidget *zoomActionWidget() const;

    KoRuler *horizontalRuler() const;
    KoRuler *verticalRuler() const;

    qreal zoom() const;

public Q_SLOTS:

    void slotZoomChanged(KoZoomMode::Mode mode, qreal zoom);
    void slotScrollAreaSizeChanged();
    void setShowRulers(bool show);
    void setRulersTrackMouse(bool value);
    void mousePositionChanged(const QPoint &viewPos);
    void changeAspectMode(bool aspectMode);
    void pageOffsetChanged();
    void zoomTo100();
    void applyRulersUnit(const KoUnit &baseUnit);
    void setMinMaxZoom();
    void setRulersPixelMultiple2(bool enabled);


private:
    void updateMouseTrackingConnections();

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
    KisSignalAutoConnectionsStore m_mouseTrackingConnections;
    qreal m_physicalDpiX;
    qreal m_physicalDpiY;
    qreal m_devicePixelRatio;
    bool m_aspectMode {false};
};


#endif
