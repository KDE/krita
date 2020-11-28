/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006 Peter Simonsson <peter.simonsson@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KODOCKFACTORY_H
#define KODOCKFACTORY_H

#include "kritaflake_export.h"

class QDockWidget;
class QString;

/**
 * Base class for factories used to create new dock widgets.
 * @see KoDockRegistry
 * @see KoCanvasObserverBase
 */
class KRITAFLAKE_EXPORT KoDockFactoryBase
{
public:
    enum DockPosition {
        DockTornOff, ///< Floating as its own top level window
        DockTop,    ///< Above the central widget
        DockBottom, ///< Below the central widget
        DockRight,  ///< Right of the centra widget
        DockLeft,   ///< Left of the centra widget
        DockMinimized  ///< Not docked, but reachable via the menu
    };

    KoDockFactoryBase();
    virtual ~KoDockFactoryBase();

    /// @return the id of the dock widget
    virtual QString id() const = 0;

    /// @return the dock widget area the widget should appear in by default
    virtual DockPosition defaultDockPosition() const = 0;

    /// Creates the dock widget
    /// @return the created dock widget
    virtual QDockWidget* createDockWidget() = 0;
};

#endif
