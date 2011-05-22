/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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

    /// reimplemented from KoCanvasObserverBase
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas();
private:
    KisColorSelectorNgDockerWidget *m_colorSelectorNgWidget;
};


#endif
