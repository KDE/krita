/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include "kis_signal_compressor.h"

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
    void updateGuiAfterDocumentSize();
    KoZoomController * zoomController() const {
        return m_zoomController;
    }

    void updateImageBoundsSnapping();
    void updateCurrentZoomResource();

    QWidget *zoomActionWidget() const;

    KoRuler *horizontalRuler() const;
    KoRuler *verticalRuler() const;

    qreal zoom() const;

    qreal resolutionX() const;
    qreal resolutionY() const;

    void updateZoomMarginSize();

public Q_SLOTS:

    void slotZoomChanged(KoZoomMode::Mode mode, qreal zoom);
    void slotZoomLevelsChanged();
    void slotScrollAreaSizeChanged();
    void setShowRulers(bool show);
    void setRulersTrackMouse(bool value);
    void mousePositionChanged(const QPoint &viewPos);
    void changeCanvasMappingMode(bool canvasMappingMode);
    void pageOffsetChanged();
    void zoomTo100();
    void slotToggleZoomToFit();
    void applyRulersUnit(const KoUnit &baseUnit);
    void setMinMaxZoom();
    void setRulersPixelMultiple2(bool enabled);


private Q_SLOTS:
    void slotUpdateGuiAfterZoomChange();

private:
    void updateMouseTrackingConnections();

private:

    QPointer<KisView> m_view;
    KoZoomHandler * m_zoomHandler {nullptr};
    KoCanvasController *m_canvasController {nullptr};
    KoZoomController *m_zoomController {nullptr};
    KoRuler * m_horizontalRuler {nullptr};
    KoRuler * m_verticalRuler {nullptr};
    KoZoomAction * m_zoomAction {nullptr};
    QPointer<QWidget> m_zoomActionWidget;
    QPoint m_rulersOffset;
    KisSignalAutoConnectionsStore m_mouseTrackingConnections;
    qreal m_physicalDpiX {72.0};
    qreal m_physicalDpiY {72.0};
    qreal m_devicePixelRatio {1.0};
    bool m_canvasMappingMode {false};
    KisSignalCompressor m_guiUpdateCompressor;
    qreal m_previousZoomLevel {1.0};
    KoZoomMode::Mode m_previousZoomMode;
    QPointF m_previousZoomPoint;
};


#endif
