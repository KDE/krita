/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _SMALLCOLORSELECTOR_DOCK_H_
#define _SMALLCOLORSELECTOR_DOCK_H_

#include <QPointer>
#include <QDockWidget>

#include <KoCanvasBase.h>
#include <KoCanvasObserverBase.h>

class KoColor;
class KisSmallColorWidget;

class SmallColorSelectorDock : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    SmallColorSelectorDock();
    QString observerName() override { return "SmallColorSelectorDock"; }
    /// reimplemented from KoCanvasObserverBase
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override { m_canvas = 0; setEnabled(false); }
public Q_SLOTS:
    void colorChangedProxy(const KoColor &);
    void canvasResourceChanged(int, const QVariant&);
private:
    KisSmallColorWidget* m_smallColorWidget;
    QPointer<KoCanvasBase> m_canvas;
};


#endif
