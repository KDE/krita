/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
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

#ifndef KIS_COLOR_SELECTOR_NG_DOCKER_H
#define KIS_COLOR_SELECTOR_NG_DOCKER_H

#include <QDockWidget>
#include <KoCanvasObserverBase.h>

class KisColorSelectorNgDockerWidget;

class KisColorSelectorNgDock : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    KisColorSelectorNgDock();
    QString observerName() override { return "KisColorSelectorNgDock"; }
    /// reimplemented from KoCanvasObserverBase
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
private:
    KisColorSelectorNgDockerWidget *m_colorSelectorNgWidget;
};


#endif
