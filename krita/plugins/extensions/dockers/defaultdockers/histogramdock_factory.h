/*
 * This file is part of the KDE project
 *
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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
#ifndef HISTOGRAM_DOCK_FACTORY_H
#define HISTOGRAM_DOCK_FACTORY_H

#include "histogramdock.h"
#include <KoDockFactory.h>

class KisHistogramDockFactory : public KoDockFactory
{
public:
    KisHistogramDockFactory() {}

    virtual QString id() const {
        return QString("KisHistogramDocker");
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const {
        return Qt::RightDockWidgetArea;
    }

    virtual QDockWidget* createDockWidget() {

        KisHistogramDocker* dockWidget = new KisHistogramDocker();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const {
        return DockRight;
    }
};

#endif
