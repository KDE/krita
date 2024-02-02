/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _BRUSHHUD_DOCK_H_
#define _BRUSHHUD_DOCK_H_

#include "kis_brush_hud.h"
#include <QPointer>
#include <QDockWidget>
#include <KoCanvasObserverBase.h>
#include <kis_canvas2.h>


class BrushHudDock : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    BrushHudDock( );
    QString observerName() override { return "BrushHudDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override { m_canvas = 0; setEnabled(false);}
private:
    QPointer<KisCanvas2> m_canvas;
    KisBrushHud* m_brushHud;
};


#endif
