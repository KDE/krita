/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KO_TOOL_REGISTRY_H_
#define KO_TOOL_REGISTRY_H_

#include "KoGenericRegistry.h"
#include "kritaflake_export.h"

class KoToolFactoryBase;

/**
 * This singleton class keeps a register of all available flake tools,
 * or rather, of the factories that the KoToolBox (and KoToolManager) will use
 * to create flake tools.
 */
class KRITAFLAKE_EXPORT KoToolRegistry : public KoGenericRegistry<KoToolFactoryBase*>
{
public:
    KoToolRegistry();
    ~KoToolRegistry() override;

    /**
     * Return an instance of the KoToolRegistry
     * Create a new instance on first call and return the singleton.
     */
    static KoToolRegistry *instance();

private:
    KoToolRegistry(const KoToolRegistry&);
    KoToolRegistry operator=(const KoToolRegistry&);
    void init();

    class Private;
    Private * const d;
};

#endif
