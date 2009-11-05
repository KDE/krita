/* This file is part of the KDE project
 *
 * Copyright (c) 2005-2006 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (c) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KO_TOOL_DOCKER_FACTORY_H
#define KO_TOOL_DOCKER_FACTORY_H

#include "KoToolDocker.h"
#include <KoDockFactory.h>

#include "komain_export.h"

/**
 * The factory class for the KoToolDocker.
 */
class KOMAIN_EXPORT KoToolDockerFactory : public KoDockFactory
{
public:
    KoToolDockerFactory( QString dockWidgetId = 0 );
    ~KoToolDockerFactory();

    /// reimplemented from parent
    virtual QString id() const;

    /// reimplemented from parent
    QDockWidget* createDockWidget();
    
    
    DockPosition defaultDockPosition() const
    {
        return DockRight;
    }
private:
    class Private;
    Private * const d;
};

#endif
