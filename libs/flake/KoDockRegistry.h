/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KODOCKREGISTRY_
#define KODOCKREGISTRY_

#include <QFont>

#include "KoGenericRegistry.h"
#include <KoDockFactoryBase.h>

#include "kritaflake_export.h"

/**
 * This singleton class keeps a register of all available dockers,
 * or rather, of the factories that can create the QDockWidget instances
 * for the mainwindows.
 * Note that adding your KoDockFactoryBase to this registry will mean it will automatically be
 * added to an application, no extra code is required for that.
 *
 * @see KoCanvasObserverBase
 */
class KRITAFLAKE_EXPORT KoDockRegistry : public KoGenericRegistry<KoDockFactoryBase*>
{
public:
    KoDockRegistry();
    ~KoDockRegistry() override;

    /**
     * Return an instance of the KoDockRegistry
     * Create a new instance on first call and return the singleton.
     */
    static KoDockRegistry *instance();


    /**
     * @brief dockFontSize calculates a smallish font size for dock widgets to use
     * @return the point size in floating point.
     */
    static QFont dockFont();

private:

    KoDockRegistry(const KoDockRegistry&);
    KoDockRegistry operator=(const KoDockRegistry&);
    void init();

    class Private;
    Private * const d;
};

#endif
