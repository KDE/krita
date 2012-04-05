/*
 * Copyright (c) 2006 Peter Simonsson <peter.simonsson@gmail.com>
 * Copyright (c) 2007 Thomas Zander <zander@kde.org>
 * Copyright (c) 2011 C. Boemann <cbo@boemann.dk>
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

#ifndef KOMODEBOXFACTORY_H
#define KOMODEBOXFACTORY_H

#include <KoDockFactoryBase.h>
#include "komain_export.h"

#include <QString>
#include <QDockWidget>

class KoCanvasControllerWidget;

/**
 * Factory class to create a new KoModeBox that contains a QToolBox which acts
 * as a replacement for KoToolBox and KoDockerManagers options docker.
 */
class KOMAIN_EXPORT KoModeBoxFactory : public KoDockFactoryBase
{
public:
    explicit KoModeBoxFactory(KoCanvasControllerWidget *canvas, const QString &applicationName, const QString& appName);
    ~KoModeBoxFactory();

    virtual QString id() const;
    KoDockFactoryBase::DockPosition defaultDockPosition() const;
    QDockWidget* createDockWidget();
    virtual bool isCollapsable() const { return false; }

private:
    class Private;
    Private * const d;
};

#endif
