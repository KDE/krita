/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
