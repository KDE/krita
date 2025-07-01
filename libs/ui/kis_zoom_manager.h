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

    void setup(KisKActionCollection * actionCollection);

    void updateImageBoundsSnapping();
    void syncOnImageResolutionChange();

    KoZoomAction *zoomAction() const;
    QWidget *zoomActionWidget() const;

    KoRuler *horizontalRuler() const;
    KoRuler *verticalRuler() const;

public Q_SLOTS:

    void setShowRulers(bool show);
    void setRulersTrackMouse(bool value);
    void mousePositionChanged(const QPoint &viewPos);

    void zoomTo100();
    void slotZoomIn();
    void slotZoomOut();
    void slotZoomToFit();
    void slotZoomToFitWidth();
    void slotZoomToFitHeight();
    void slotToggleZoomToFit();
    void applyRulersUnit(const KoUnit &baseUnit);
    void setRulersPixelMultiple2(bool enabled);


private Q_SLOTS:
    void slotUpdateGuiAfterZoomChange();
    void slotUpdateGuiAfterDocumentRectChanged();
    void slotConfigChanged();

private:
    void updateMouseTrackingConnections();
    void updateCurrentZoomResource();

private:

    QPointer<KisView> m_view;
    KoZoomHandler * m_zoomHandler {nullptr};
    KoCanvasController *m_canvasController {nullptr};
    KoRuler * m_horizontalRuler {nullptr};
    KoRuler * m_verticalRuler {nullptr};
    KoZoomAction * m_zoomAction {nullptr};
    QPointer<QWidget> m_zoomActionWidget;
    QRect m_cachedRulersRect;
    KisSignalAutoConnectionsStore m_mouseTrackingConnections;

    KisSignalCompressor m_zoomChangedCompressor;
    KisSignalCompressor m_documentRectChangedCompressor;
    qreal m_previousZoomLevel {1.0};
    KoZoomMode::Mode m_previousZoomMode;
    QPointF m_previousZoomPoint;
};


#endif
